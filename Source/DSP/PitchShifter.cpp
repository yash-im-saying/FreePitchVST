#include "PitchShifter.h"
#include <cmath>
#include <algorithm>

PitchShifter::PitchShifter() {
    m_inputBuffer.assign(kBufferSize, 0.0f);
    m_voicedBuffer.assign(kBufferSize, static_cast<uint8_t>(0));
    updateLatency();
    reset();
}

void PitchShifter::prepare(double sampleRate, int /*maxBlockSize*/) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;

    m_inputBuffer.assign(kBufferSize, 0.0f);
    m_voicedBuffer.assign(kBufferSize, static_cast<uint8_t>(0));

    m_formantEngine.prepare(m_sampleRate, 2048);

    updateLatency();
    reset();
}

void PitchShifter::reset() {
    std::fill(m_inputBuffer.begin(), m_inputBuffer.end(), 0.0f);
    std::fill(m_voicedBuffer.begin(), m_voicedBuffer.end(), static_cast<uint8_t>(0));

    m_writeSampleCount = 0;
    m_tapDelay1 = static_cast<double>(m_latencySamples);
    m_tapDelay2 = static_cast<double>(m_latencySamples);
    m_crossfading = false;
    m_fadePos = 0;
    m_fadeLen = 256;

    m_smoothedAlpha = 1.0f;
    m_currentPitchPeriod = static_cast<float>(m_sampleRate / 200.0);
    m_targetPitchPeriod = m_currentPitchPeriod;
    m_voicedGain = 0.0f;

    m_formantEngine.reset();
}

void PitchShifter::setLookaheadMode(bool lookaheadEnabled) {
    if (m_lookaheadEnabled != lookaheadEnabled) {
        m_lookaheadEnabled = lookaheadEnabled;
        updateLatency();
    }
}

void PitchShifter::setVocalMode(VocalMode mode) {
    m_vocalMode = mode;
    updateLatency();
}

void PitchShifter::updateLatency() {
    int idx = static_cast<int>(m_vocalMode);
    if (idx < 0 || idx >= static_cast<int>(VocalMode::NumModes)) idx = 3; // Tenor default
    float minFreq = g_vocalModeBounds[idx].minHz;
    int maxPeriodSamples = static_cast<int>(std::ceil(m_sampleRate / minFreq));

    // Base latency must allow at least 2.5x the maximum period of the active vocal mode
    // to allow continuous pitch drift and crossfading without running out of samples
    int neededLatency = static_cast<int>(std::ceil(maxPeriodSamples * 2.5f));
    int minLatency = static_cast<int>(m_sampleRate * 0.008); // 8 ms minimum

    if (m_lookaheadEnabled) {
        int lookaheadSamples = static_cast<int>(m_sampleRate * 0.030); // 30 ms
        m_latencySamples = std::max(neededLatency, lookaheadSamples);
    } else {
        m_latencySamples = std::max(neededLatency, minLatency);
    }
}

void PitchShifter::process(const float* input, float* output, int numSamples,
                           float inputPitchHz, float targetPitchHz, bool isVoiced,
                           bool formantsEnabled, float formantShiftSt) {
    if (input == nullptr || output == nullptr || numSamples <= 0)
        return;

    // Calculate periods in samples
    float inPeriod = (inputPitchHz > 30.0f) ? static_cast<float>(m_sampleRate / inputPitchHz) : m_currentPitchPeriod;
    float outPeriod = (targetPitchHz > 30.0f) ? static_cast<float>(m_sampleRate / targetPitchHz) : inPeriod;

    // Constrain periods strictly to valid audio bounds (30 Hz to 1200 Hz)
    float minPeriod = static_cast<float>(m_sampleRate / 1200.0f);
    float maxPeriod = static_cast<float>(m_sampleRate / 30.0f);
    inPeriod = std::clamp(inPeriod, minPeriod, maxPeriod);
    outPeriod = std::clamp(outPeriod, minPeriod, maxPeriod);

    // Smooth period transitions
    m_currentPitchPeriod += 0.10f * (inPeriod - m_currentPitchPeriod);
    m_targetPitchPeriod += 0.10f * (outPeriod - m_targetPitchPeriod);

    // Target pitch shift ratio alpha: target pitch / detected pitch
    float targetAlpha = (inputPitchHz > 30.0f && targetPitchHz > 30.0f)
                      ? (targetPitchHz / inputPitchHz)
                      : 1.0f;
    targetAlpha = std::clamp(targetAlpha, 0.5f, 2.0f); // Limit to +-1 octave

    double baseLatency = static_cast<double>(m_latencySamples);

    // Process sample by sample
    for (int n = 0; n < numSamples; ++n) {
        int64_t currentSample = m_writeSampleCount++;
        m_inputBuffer[currentSample & kBufferMask] = input[n];
        m_voicedBuffer[currentSample & kBufferMask] = isVoiced ? 1 : 0;

        // Smooth pitch ratio per sample over ~4 ms to eliminate block-boundary slope kinks
        m_smoothedAlpha += 0.005f * (targetAlpha - m_smoothedAlpha);
        double rateStep = 1.0 - static_cast<double>(m_smoothedAlpha);

        float shiftedSample = 0.0f;

        if (m_crossfading) {
            // Both taps advance continuously at the identical Doppler rate during crossfade
            m_tapDelay1 += rateStep;
            m_tapDelay2 += rateStep;
            m_fadePos++;

            float phase = static_cast<float>(m_fadePos) / static_cast<float>(m_fadeLen);
            phase = std::clamp(phase, 0.0f, 1.0f);
            float w = 0.5f - 0.5f * std::cos(3.14159265358979323846f * phase);

            shiftedSample = (1.0f - w) * readHermite(currentSample, m_tapDelay1)
                          + w * readHermite(currentSample, m_tapDelay2);

            if (m_fadePos >= m_fadeLen) {
                m_tapDelay1 = m_tapDelay2;
                m_crossfading = false;
            }
        } else {
            if (isVoiced) {
                m_tapDelay1 += rateStep;

                double drift = m_tapDelay1 - baseLatency;
                if (std::abs(drift) > static_cast<double>(m_currentPitchPeriod)) {
                    int k = static_cast<int>(std::round(drift / static_cast<double>(m_currentPitchPeriod)));
                    if (k != 0) {
                        // Spawn Tap 2 at EXACT integer pitch-period offset: 100% phase-aligned!
                        double nominalD2 = m_tapDelay1 - static_cast<double>(k) * static_cast<double>(m_currentPitchPeriod);
                        nominalD2 = std::clamp(nominalD2, 32.0, static_cast<double>(kBufferSize - 2048));

                        m_tapDelay2 = nominalD2;
                        m_crossfading = true;
                        m_fadePos = 0;
                        m_fadeLen = std::max(64, static_cast<int>(std::round(m_currentPitchPeriod)));
                    }
                }
            }
            shiftedSample = readHermite(currentSample, m_tapDelay1);
        }

        // Output read position aligned with PDC latency
        float bypassSample = readHermite(currentSample, baseLatency);

        // Voiced gain crossfade based on latency-delayed voiced state
        int64_t delayedSample = currentSample - m_latencySamples;
        bool delayedVoiced = (delayedSample >= 0) ? (m_voicedBuffer[delayedSample & kBufferMask] != 0) : false;
        float targetGain = delayedVoiced ? 1.0f : 0.0f;
        m_voicedGain += 0.008f * (targetGain - m_voicedGain);

        // When audio is completely unvoiced and silent in the mix, gently re-center delay taps
        if (m_voicedGain < 0.001f && !isVoiced) {
            m_tapDelay1 = baseLatency;
            m_tapDelay2 = baseLatency;
            m_crossfading = false;
            m_fadePos = 0;
        }

        float outSample = (1.0f - m_voicedGain) * bypassSample + m_voicedGain * shiftedSample;

        if (!std::isfinite(outSample)) {
            outSample = 0.0f;
        }

        output[n] = outSample;
    }

    // Real-time spectral envelope formant preservation and shifting
    float pitchRatio = (inputPitchHz > 20.0f && targetPitchHz > 20.0f) ? (targetPitchHz / inputPitchHz) : 1.0f;
    m_formantEngine.process(output, numSamples, inputPitchHz, pitchRatio, formantShiftSt, formantsEnabled);
}
