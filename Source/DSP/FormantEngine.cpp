#include "FormantEngine.h"
#include <cmath>
#include <algorithm>

FormantEngine::FormantEngine() {
    prepare(48000.0, 2048);
}

void FormantEngine::prepare(double sampleRate, int /*maxBlockSize*/) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;

    // Use order 10 (1024) for 44.1/48k, order 11 (2048) for 88.2/96k, order 12 (4096) for 192k
    if (m_sampleRate > 120000.0) {
        m_fftOrder = 12; // 4096
    } else if (m_sampleRate > 64000.0) {
        m_fftOrder = 11; // 2048
    } else {
        m_fftOrder = 10; // 1024
    }

    m_fftSize = 1 << m_fftOrder;
    m_hopSize = m_fftSize / 4; // 75% overlap
    m_latencySamples = m_fftSize; // 1024 samples at 1024 (ensures complete 4-grain COLA summation)

    m_fft = std::make_unique<juce::dsp::FFT>(m_fftOrder);

    m_analysisWindow.resize(m_fftSize);
    m_synthesisWindow.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i) {
        float h = 0.5f - 0.5f * std::cos(2.0f * 3.14159265358979323846f * static_cast<float>(i) / static_cast<float>(m_fftSize));
        m_analysisWindow[i] = h;
        // For 75% overlap of Hann^2, sum is 1.5. Dividing by 1.5 gives exact unity gain.
        m_synthesisWindow[i] = h / 1.5f;
    }

    m_inputFifo.assign(kFifoSize, 0.0f);
    m_outputFifo.assign(kFifoSize, 0.0f);

    m_fftBuffer.assign(m_fftSize * 2, 0.0f);
    int numBins = m_fftSize / 2 + 1;
    m_magSpectrum.assign(numBins, 0.0f);
    m_smoothedEnvelope.assign(numBins, 0.0f);
    m_targetEnvelope.assign(numBins, 0.0f);
    m_peakEnv.assign(numBins, 0.0f);

    reset();
}

void FormantEngine::reset() {
    std::fill(m_inputFifo.begin(), m_inputFifo.end(), 0.0f);
    std::fill(m_outputFifo.begin(), m_outputFifo.end(), 0.0f);
    std::fill(m_fftBuffer.begin(), m_fftBuffer.end(), 0.0f);
    std::fill(m_magSpectrum.begin(), m_magSpectrum.end(), 0.0f);
    std::fill(m_smoothedEnvelope.begin(), m_smoothedEnvelope.end(), 0.0f);
    std::fill(m_targetEnvelope.begin(), m_targetEnvelope.end(), 0.0f);
    std::fill(m_peakEnv.begin(), m_peakEnv.end(), 0.0f);

    m_writePos = 0;
    m_hopCounter = 0;
    m_smoothedWarpFactor = 1.0f;
}

void FormantEngine::computeSpectralEnvelope(const float* mag, float* envOut, int numBins, float pitchHz) {
    float binHz = static_cast<float>(m_sampleRate) / static_cast<float>(m_fftSize);
    int windowBins = 12;
    if (pitchHz > 60.0f) {
        windowBins = std::clamp(static_cast<int>(std::round(1.5f * (pitchHz / binHz))), 6, 28);
    }

    // Pass 1: Local peak follower over harmonic spacing
    for (int k = 0; k < numBins; ++k) {
        int start = std::max(0, k - windowBins);
        int end = std::min(numBins - 1, k + windowBins);
        float pMax = 0.0f;
        for (int j = start; j <= end; ++j) {
            if (mag[j] > pMax) pMax = mag[j];
        }
        m_peakEnv[k] = pMax;
    }

    // Pass 2: Moving average smoothing for vocal tract resonance contour
    int smoothHalf = windowBins;
    float sum = 0.0f;
    int count = 0;

    for (int k = 0; k < std::min(smoothHalf, numBins); ++k) {
        sum += m_peakEnv[k];
        count++;
    }

    for (int k = 0; k < numBins; ++k) {
        int addIdx = k + smoothHalf;
        if (addIdx < numBins) {
            sum += m_peakEnv[addIdx];
            count++;
        }
        int subIdx = k - smoothHalf - 1;
        if (subIdx >= 0) {
            sum -= m_peakEnv[subIdx];
            count--;
        }
        envOut[k] = (count > 0) ? (sum / static_cast<float>(count)) : m_peakEnv[k];
    }
}

void FormantEngine::process(float* buffer, int numSamples, float inputPitchHz,
                            float pitchRatio, float formantShiftSemitones, bool formantsEnabled) {
    if (buffer == nullptr || numSamples <= 0) return;

    // Formant shift scaling factor: beta = 2^(shift / 12)
    float formantScale = std::exp2(formantShiftSemitones / 12.0f);

    // If formants are enabled (preserve natural vocal tract resonances):
    // Pitch shift scaled formants by pitchRatio. To preserve them, envelope warp factor = formantScale / pitchRatio.
    float targetWarpFactor = formantsEnabled ? (formantScale / std::max(0.01f, pitchRatio)) : formantScale;
    targetWarpFactor = std::clamp(targetWarpFactor, 0.5f, 2.0f);

    // Smooth warp factor per block
    m_smoothedWarpFactor += 0.15f * (targetWarpFactor - m_smoothedWarpFactor);

    int numBins = m_fftSize / 2 + 1;

    for (int i = 0; i < numSamples; ++i) {
        // 1. Write sample to input FIFO
        int64_t curWritePos = m_writePos;
        m_inputFifo[curWritePos & kFifoMask] = buffer[i];
        m_writePos++;
        m_hopCounter++;

        // 2. Perform STFT Hop
        if (m_hopCounter >= m_hopSize) {
            m_hopCounter = 0;

            int64_t startIdx = m_writePos - m_fftSize;
            for (int k = 0; k < m_fftSize; ++k) {
                m_fftBuffer[k] = m_inputFifo[(startIdx + k) & kFifoMask] * m_analysisWindow[k];
            }
            std::fill(m_fftBuffer.begin() + m_fftSize, m_fftBuffer.end(), 0.0f);

            m_fft->performRealOnlyForwardTransform(m_fftBuffer.data());

            // If warp factor is substantially different from 1.0, apply spectral envelope reshaping
            if (std::abs(m_smoothedWarpFactor - 1.0f) >= 0.015f) {
                // Compute magnitude spectrum
                for (int k = 0; k < numBins; ++k) {
                    float re = m_fftBuffer[2 * k];
                    float im = m_fftBuffer[2 * k + 1];
                    m_magSpectrum[k] = std::sqrt(re * re + im * im);
                }

                // Extract smooth spectral envelope
                computeSpectralEnvelope(m_magSpectrum.data(), m_smoothedEnvelope.data(), numBins, inputPitchHz);

                // Frequency-domain envelope warping
                for (int k = 0; k < numBins; ++k) {
                    float unwarpedBin = static_cast<float>(k) / m_smoothedWarpFactor;
                    if (unwarpedBin <= 0.0f) {
                        m_targetEnvelope[k] = m_smoothedEnvelope[0];
                    } else if (unwarpedBin >= numBins - 1) {
                        m_targetEnvelope[k] = m_smoothedEnvelope[numBins - 1];
                    } else {
                        int bin0 = static_cast<int>(unwarpedBin);
                        int bin1 = bin0 + 1;
                        float frac = unwarpedBin - static_cast<float>(bin0);
                        m_targetEnvelope[k] = (1.0f - frac) * m_smoothedEnvelope[bin0] + frac * m_smoothedEnvelope[bin1];
                    }
                }

                // Apply spectral envelope quotient filter
                for (int k = 0; k < numBins; ++k) {
                    float denom = m_smoothedEnvelope[k] + 1e-4f;
                    float gain = m_targetEnvelope[k] / denom;
                    gain = std::clamp(gain, 0.125f, 6.0f); // -18 dB to +15.5 dB
                    m_fftBuffer[2 * k] *= gain;
                    m_fftBuffer[2 * k + 1] *= gain;
                }
            }

            // Inverse FFT back to time domain
            m_fft->performRealOnlyInverseTransform(m_fftBuffer.data());

            // Overlap-add to output FIFO
            for (int k = 0; k < m_fftSize; ++k) {
                m_outputFifo[(startIdx + k) & kFifoMask] += m_fftBuffer[k] * m_synthesisWindow[k];
            }
        }

        // 3. Read reconstructed sample from output FIFO
        int64_t outReadIdx = curWritePos - m_latencySamples;
        float outSample = m_outputFifo[outReadIdx & kFifoMask];
        m_outputFifo[outReadIdx & kFifoMask] = 0.0f; // Clear slot for future overlap-adds

        if (!std::isfinite(outSample)) outSample = 0.0f;
        buffer[i] = outSample;
    }
}
