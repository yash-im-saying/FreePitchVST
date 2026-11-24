#include "PitchDetector.h"
#include <cmath>
#include <algorithm>

PitchDetector::PitchDetector() {
    updateLagBounds();
}

void PitchDetector::prepare(double sampleRate, int /*maxBlockSize*/) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;

    // Minimum frequency supported is ~30 Hz (E1/C1). At 48kHz, 1 period = 1600 samples.
    // 2048 sample window is sufficient up to 48kHz. For 96kHz or 192kHz, adjust window size accordingly.
    if (m_sampleRate >= 88200.0) {
        m_windowSize = 4096;
        m_fftOrder = 13; // 8192
    } else {
        m_windowSize = 2048;
        m_fftOrder = 12; // 4096
    }
    m_fftSize = 1 << m_fftOrder;

    m_fft = std::make_unique<juce::dsp::FFT>(m_fftOrder);

    m_inputRingBuffer.assign(m_windowSize * 2, 0.0f);
    m_ringWritePos = 0;

    m_analysisWindow.assign(m_windowSize, 0.0f);
    m_fftBuffer.assign(m_fftSize * 2, 0.0f); // 2x for complex FFT
    m_nsdf.assign(m_windowSize, 0.0f);
    m_squareSum.assign(m_windowSize + 1, 0.0f);

    updateLagBounds();
}

void PitchDetector::reset() {
    std::fill(m_inputRingBuffer.begin(), m_inputRingBuffer.end(), 0.0f);
    m_ringWritePos = 0;
    std::fill(m_nsdf.begin(), m_nsdf.end(), 0.0f);
    m_samplesSinceLastAnalysis = 9999;
    m_lastDetectedHz = 0.0f;
    m_lastConfidence = 0.0f;
    m_lastRms = 0.0f;
    m_lastVoiced = false;
}

void PitchDetector::setVocalMode(VocalMode mode) {
    if (mode < VocalMode::Soprano || mode >= VocalMode::NumModes)
        mode = VocalMode::Tenor;

    m_vocalMode = mode;
    updateLagBounds();
}

void PitchDetector::updateLagBounds() {
    int idx = static_cast<int>(m_vocalMode);
    if (idx < 0 || idx >= static_cast<int>(VocalMode::NumModes))
        idx = 3; // Tenor default

    const auto& b = g_vocalModeBounds[idx];
    
    // lag = sampleRate / freq
    // For max frequency, lag is minLag
    m_minLag = std::max(2, static_cast<int>(std::floor(m_sampleRate / b.maxHz)));
    // For min frequency, lag is maxLag
    m_maxLag = std::min(m_windowSize - 2, static_cast<int>(std::ceil(m_sampleRate / b.minHz)));

    if (m_minLag >= m_maxLag) {
        m_minLag = 10;
        m_maxLag = m_windowSize / 2;
    }
}

void PitchDetector::pushSamples(const float* input, int numSamples) {
    if (input == nullptr || numSamples <= 0 || m_inputRingBuffer.empty())
        return;

    int bufSize = static_cast<int>(m_inputRingBuffer.size());
    for (int i = 0; i < numSamples; ++i) {
        m_inputRingBuffer[m_ringWritePos] = input[i];
        m_ringWritePos = (m_ringWritePos + 1) % bufSize;
    }
    m_samplesSinceLastAnalysis += numSamples;
}

void PitchDetector::computeNSDF() {
    // 1. Extract the most recent m_windowSize samples from the ring buffer
    int bufSize = static_cast<int>(m_inputRingBuffer.size());
    int readStart = (m_ringWritePos - m_windowSize + bufSize) % bufSize;

    for (int i = 0; i < m_windowSize; ++i) {
        m_analysisWindow[i] = m_inputRingBuffer[(readStart + i) % bufSize];
    }

    // 2. Compute cumulative square sums for fast denominator calculation
    // squareSum[i] = sum_{j=0}^{i-1} (x_j)^2
    m_squareSum[0] = 0.0f;
    for (int i = 0; i < m_windowSize; ++i) {
        m_squareSum[i + 1] = m_squareSum[i] + m_analysisWindow[i] * m_analysisWindow[i];
    }

    // 3. Compute Autocorrelation via FFT
    // Clear fftBuffer
    std::fill(m_fftBuffer.begin(), m_fftBuffer.end(), 0.0f);

    // Copy analysis window to first half of fftBuffer, remaining zero-padded
    for (int i = 0; i < m_windowSize; ++i) {
        m_fftBuffer[i] = m_analysisWindow[i];
    }

    // Forward real FFT
    // JUCE performRealOnlyForwardTransform expects size 2 * fftSize floats (interleaved complex format)
    m_fft->performRealOnlyForwardTransform(m_fftBuffer.data());

    // Power spectrum in-place: |X(k)|^2
    for (int k = 0; k <= m_fftSize / 2; ++k) {
        float re = m_fftBuffer[2 * k];
        float im = m_fftBuffer[2 * k + 1];
        float magSq = re * re + im * im;
        m_fftBuffer[2 * k] = magSq;
        m_fftBuffer[2 * k + 1] = 0.0f;
    }
    // Conjugate symmetry for k > m_fftSize / 2
    for (int k = m_fftSize / 2 + 1; k < m_fftSize; ++k) {
        m_fftBuffer[2 * k] = m_fftBuffer[2 * (m_fftSize - k)];
        m_fftBuffer[2 * k + 1] = 0.0f;
    }

    // Inverse real FFT to get autocorrelation r_xx(tau)
    m_fft->performRealOnlyInverseTransform(m_fftBuffer.data());

    // 4. Compute NSDF
    // r(tau) = 2 * r_xx(tau) / ( sum_{j=0}^{W-1-tau} x_j^2 + sum_{j=0}^{W-1-tau} x_{j+tau}^2 )
    // sum_{j=0}^{W-1-tau} x_j^2 = squareSum[W - tau] - squareSum[0]
    // sum_{j=0}^{W-1-tau} x_{j+tau}^2 = sum_{k=tau}^{W-1} x_k^2 = squareSum[W] - squareSum[tau]
    // denom = squareSum[W - tau] + squareSum[W] - squareSum[tau]
    for (int tau = 0; tau < m_windowSize; ++tau) {
        float r_xx = m_fftBuffer[tau];
        float energyTerm1 = m_squareSum[m_windowSize - tau];
        float energyTerm2 = m_squareSum[m_windowSize] - m_squareSum[tau];
        float denom = energyTerm1 + energyTerm2;

        if (denom > 1e-9f) {
            float val = (2.0f * r_xx) / denom;
            m_nsdf[tau] = std::clamp(val, -1.0f, 1.0f);
        } else {
            m_nsdf[tau] = 0.0f;
        }
    }
}

bool PitchDetector::processPitch(float& outPitchHz, float& outConfidence, float& outRms) {
    if (m_samplesSinceLastAnalysis < m_hopSize) {
        outPitchHz = m_lastDetectedHz;
        outConfidence = m_lastConfidence;
        outRms = m_lastRms;
        return m_lastVoiced;
    }
    m_samplesSinceLastAnalysis = 0;

    outPitchHz = 0.0f;
    outConfidence = 0.0f;
    outRms = 0.0f;

    // First compute RMS of the window
    computeNSDF();

    float totalEnergy = m_squareSum[m_windowSize];
    float meanSquare = totalEnergy / static_cast<float>(m_windowSize);
    outRms = std::sqrt(std::max(0.0f, meanSquare));

    // Silence gate (-60 dB threshold ~ 0.001 RMS; -50 dB ~ 0.003 RMS in Choosy mode)
    float silenceThreshold = m_choosyTracking ? 0.003f : 0.001f;
    if (outRms < silenceThreshold) {
        m_lastDetectedHz = 0.0f;
        m_lastConfidence = 0.0f;
        m_lastRms = outRms;
        m_lastVoiced = false;
        return false;
    }

    // Peak Picking in NSDF within [m_minLag, m_maxLag]
    // Find all positive local maxima that occur after zero crossings
    struct Peak {
        int lag;
        float value;
    };
    std::vector<Peak> peaks;
    peaks.reserve(32);

    float maxPeakValue = 0.0f;

    // Search for peaks
    for (int tau = m_minLag; tau <= m_maxLag; ++tau) {
        // Local maximum condition
        if (m_nsdf[tau] > 0.0f &&
            m_nsdf[tau] > m_nsdf[tau - 1] &&
            m_nsdf[tau] >= m_nsdf[tau + 1]) {
            
            peaks.push_back({ tau, m_nsdf[tau] });
            if (m_nsdf[tau] > maxPeakValue) {
                maxPeakValue = m_nsdf[tau];
            }
        }
    }

    float minConfidence = m_choosyTracking ? 0.65f : 0.40f;
    if (peaks.empty() || maxPeakValue < minConfidence) {
        outConfidence = maxPeakValue;
        m_lastDetectedHz = 0.0f;
        m_lastConfidence = outConfidence;
        m_lastRms = outRms;
        m_lastVoiced = false;
        return false;
    }

    // McLeod MPM heuristic: choose the FIRST peak with value >= k * maxPeakValue (k ~ 0.85 - 0.90)
    // This avoids subharmonic (octave-too-low) errors
    float kFactor = m_choosyTracking ? 0.90f : 0.85f;
    float threshold = kFactor * maxPeakValue;
    const Peak* chosenPeak = nullptr;

    for (const auto& p : peaks) {
        if (p.value >= threshold) {
            chosenPeak = &p;
            break;
        }
    }

    if (!chosenPeak) {
        chosenPeak = &peaks[0];
    }

    int tau = chosenPeak->lag;
    float alpha = m_nsdf[tau - 1];
    float beta = m_nsdf[tau];
    float gamma = m_nsdf[tau + 1];

    // Parabolic interpolation:
    // delta = (alpha - gamma) / (2 * (alpha - 2*beta + gamma))
    float denom = 2.0f * (alpha - 2.0f * beta + gamma);
    float delta = 0.0f;
    if (std::abs(denom) > 1e-7f) {
        delta = (alpha - gamma) / denom;
        delta = std::clamp(delta, -1.0f, 1.0f);
    }

    float refinedLag = static_cast<float>(tau) + delta;
    float refinedPeakValue = beta - 0.25f * (alpha - gamma) * delta;

    outConfidence = std::clamp(refinedPeakValue, 0.0f, 1.0f);

    float requiredConfidence = m_choosyTracking ? 0.70f : 0.50f;
    if (refinedLag > 0.0f && outConfidence >= requiredConfidence) {
        outPitchHz = static_cast<float>(m_sampleRate) / refinedLag;
        m_lastDetectedHz = outPitchHz;
        m_lastConfidence = outConfidence;
        m_lastRms = outRms;
        m_lastVoiced = true;
        return true;
    }

    m_lastDetectedHz = 0.0f;
    m_lastConfidence = outConfidence;
    m_lastRms = outRms;
    m_lastVoiced = false;
    return false;
}
