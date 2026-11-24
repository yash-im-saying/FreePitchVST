#include "EFXRack.h"
#include <cmath>
#include <algorithm>

EFXRack::EFXRack() {
    updateEqCoefficients();
    updateDistToneFilter();
}

void EFXRack::prepare(double sampleRate, int /*maxBlockSize*/) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;

    // Pre-allocate delay buffers: Flanger max 10 ms (~1000 samples), Chorus max 50 ms (~5000 samples)
    int maxDelayFlanger = static_cast<int>(m_sampleRate * 0.020);
    m_flangerDelayBufferL.assign(maxDelayFlanger, 0.0f);
    m_flangerDelayBufferR.assign(maxDelayFlanger, 0.0f);
    m_flangerWritePos = 0;

    int maxDelayChorus = static_cast<int>(m_sampleRate * 0.050);
    m_chorusDelayBufferL.assign(maxDelayChorus, 0.0f);
    m_chorusDelayBufferR.assign(maxDelayChorus, 0.0f);
    m_chorusWritePos = 0;

    // Pre-allocate delay line: max 2.5 seconds
    int maxDelaySamples = static_cast<int>(m_sampleRate * 2.5);
    m_delayBufferL.assign(maxDelaySamples, 0.0f);
    m_delayBufferR.assign(maxDelaySamples, 0.0f);
    m_delayWritePos = 0;

    // Supermassive allpass diffusers (scaled by sample rate relative to 48kHz)
    float srScale = static_cast<float>(m_sampleRate / 48000.0);
    int diffLengthsL[4] = { std::max(8, static_cast<int>(149 * srScale)), std::max(8, static_cast<int>(211 * srScale)),
                            std::max(8, static_cast<int>(293 * srScale)), std::max(8, static_cast<int>(379 * srScale)) };
    int diffLengthsR[4] = { std::max(8, static_cast<int>(157 * srScale)), std::max(8, static_cast<int>(223 * srScale)),
                            std::max(8, static_cast<int>(307 * srScale)), std::max(8, static_cast<int>(389 * srScale)) };
    for (int s = 0; s < 4; ++s) {
        m_diffusersL[s].init(diffLengthsL[s]);
        m_diffusersR[s].init(diffLengthsR[s]);
    }

    updateEqCoefficients();
    updateDistToneFilter();
    reset();
}

void EFXRack::reset() {
    m_eqHpfL.reset(); m_eqHpfR.reset();
    m_eqLowMidL.reset(); m_eqLowMidR.reset();
    m_eqHighMidL.reset(); m_eqHighMidR.reset();
    m_eqAirL.reset(); m_eqAirR.reset();

    m_compEnvL = 0.0f;
    m_compEnvR = 0.0f;
    m_compGRDb = 0.0f;

    std::fill(m_flangerDelayBufferL.begin(), m_flangerDelayBufferL.end(), 0.0f);
    std::fill(m_flangerDelayBufferR.begin(), m_flangerDelayBufferR.end(), 0.0f);
    m_flangerWritePos = 0;
    m_flangerPhase = 0.0f;

    std::fill(m_chorusDelayBufferL.begin(), m_chorusDelayBufferL.end(), 0.0f);
    std::fill(m_chorusDelayBufferR.begin(), m_chorusDelayBufferR.end(), 0.0f);
    m_chorusWritePos = 0;
    m_chorusPhase = 0.0f;

    m_distToneL.reset(); m_distToneR.reset();

    std::fill(m_delayBufferL.begin(), m_delayBufferL.end(), 0.0f);
    std::fill(m_delayBufferR.begin(), m_delayBufferR.end(), 0.0f);
    m_delayWritePos = 0;
    m_hilbertL.reset();
    m_hilbertR.reset();
    m_shiftPhaseL = 0.0f;
    m_shiftPhaseR = 0.0f;
    for (int s = 0; s < 4; ++s) {
        m_diffusersL[s].reset();
        m_diffusersR[s].reset();
    }
    m_delayHpfStateL = m_delayHpfStateR = 0.0f;
    m_delayLpfStateL = m_delayLpfStateR = 0.0f;
}

void EFXRack::setEqParams(bool enabled, float hpfFreq, float lowMidFreq, float lowMidGainDb,
                          float highMidFreq, float highMidGainDb, float airFreq, float airGainDb,
                          float outGainDb) {
    m_eqEnabled = enabled;
    m_hpfFreq = std::clamp(hpfFreq, 20.0f, 300.0f);
    m_lowMidFreq = std::clamp(lowMidFreq, 200.0f, 1500.0f);
    m_lowMidGainDb = std::clamp(lowMidGainDb, -12.0f, 12.0f);
    m_highMidFreq = std::clamp(highMidFreq, 1500.0f, 6000.0f);
    m_highMidGainDb = std::clamp(highMidGainDb, -12.0f, 12.0f);
    m_airFreq = std::clamp(airFreq, 8000.0f, 16000.0f);
    m_airGainDb = std::clamp(airGainDb, -12.0f, 12.0f);
    m_eqOutGainLinear = juce::Decibels::decibelsToGain(outGainDb);

    if (std::abs(m_hpfFreq - m_prevHpfFreq) > 0.05f ||
        std::abs(m_lowMidFreq - m_prevLowMidFreq) > 0.05f ||
        std::abs(m_lowMidGainDb - m_prevLowMidGainDb) > 0.05f ||
        std::abs(m_highMidFreq - m_prevHighMidFreq) > 0.05f ||
        std::abs(m_highMidGainDb - m_prevHighMidGainDb) > 0.05f ||
        std::abs(m_airFreq - m_prevAirFreq) > 0.05f ||
        std::abs(m_airGainDb - m_prevAirGainDb) > 0.05f ||
        m_sampleRate != m_prevEqSampleRate) {
        updateEqCoefficients();
    }
}

void EFXRack::updateEqCoefficients() {
    m_prevHpfFreq = m_hpfFreq;
    m_prevLowMidFreq = m_lowMidFreq;
    m_prevLowMidGainDb = m_lowMidGainDb;
    m_prevHighMidFreq = m_highMidFreq;
    m_prevHighMidGainDb = m_highMidGainDb;
    m_prevAirFreq = m_airFreq;
    m_prevAirGainDb = m_airGainDb;
    m_prevEqSampleRate = m_sampleRate;

    float nyquist = static_cast<float>(m_sampleRate * 0.499);
    float hpf = std::clamp(m_hpfFreq, 20.0f, nyquist);
    float lowMid = std::clamp(m_lowMidFreq, 20.0f, nyquist);
    float highMid = std::clamp(m_highMidFreq, 20.0f, nyquist);
    float air = std::clamp(m_airFreq, 20.0f, nyquist);

    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(m_sampleRate, hpf, 0.707f);
    auto lowMidCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(m_sampleRate, lowMid, 1.2f, juce::Decibels::decibelsToGain(m_lowMidGainDb));
    auto highMidCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(m_sampleRate, highMid, 1.4f, juce::Decibels::decibelsToGain(m_highMidGainDb));
    auto airCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(m_sampleRate, air, 0.707f, juce::Decibels::decibelsToGain(m_airGainDb));

    m_eqHpfL.coefficients = hpfCoeffs;
    m_eqHpfR.coefficients = hpfCoeffs;
    m_eqLowMidL.coefficients = lowMidCoeffs;
    m_eqLowMidR.coefficients = lowMidCoeffs;
    m_eqHighMidL.coefficients = highMidCoeffs;
    m_eqHighMidR.coefficients = highMidCoeffs;
    m_eqAirL.coefficients = airCoeffs;
    m_eqAirR.coefficients = airCoeffs;
}

float EFXRack::getEqResponseAt(float freqHz) const {
    if (!m_eqEnabled) return 0.0f;
    // Approximate magnitude response in dB
    float hpf = m_eqHpfL.coefficients ? static_cast<float>(m_eqHpfL.coefficients->getMagnitudeForFrequency(freqHz, m_sampleRate)) : 1.0f;
    float lm = m_eqLowMidL.coefficients ? static_cast<float>(m_eqLowMidL.coefficients->getMagnitudeForFrequency(freqHz, m_sampleRate)) : 1.0f;
    float hm = m_eqHighMidL.coefficients ? static_cast<float>(m_eqHighMidL.coefficients->getMagnitudeForFrequency(freqHz, m_sampleRate)) : 1.0f;
    float air = m_eqAirL.coefficients ? static_cast<float>(m_eqAirL.coefficients->getMagnitudeForFrequency(freqHz, m_sampleRate)) : 1.0f;

    float totalMag = hpf * lm * hm * air * m_eqOutGainLinear;
    return juce::Decibels::gainToDecibels(std::max(1e-4f, totalMag));
}

void EFXRack::setCompParams(bool enabled, float thresholdDb, float ratio, float attackMs,
                            float releaseMs, float makeupDb) {
    m_compEnabled = enabled;
    m_compThresholdDb = thresholdDb;
    m_compRatio = std::max(1.0f, ratio);
    m_compMakeupLinear = juce::Decibels::decibelsToGain(makeupDb);

    float attSec = std::max(0.0001f, attackMs * 0.001f);
    float relSec = std::max(0.001f, releaseMs * 0.001f);
    m_compAttackCoeff = std::exp(-1.0f / (attSec * static_cast<float>(m_sampleRate)));
    m_compReleaseCoeff = std::exp(-1.0f / (relSec * static_cast<float>(m_sampleRate)));
}

void EFXRack::setFlangerParams(bool enabled, float rateHz, float depthPct, float feedbackPct, float mixPct) {
    m_flangerEnabled = enabled;
    m_flangerRate = std::clamp(rateHz, 0.05f, 10.0f);
    m_flangerDepth = depthPct * 0.01f;
    m_flangerFeedback = feedbackPct * 0.01f;
    m_flangerMix = mixPct * 0.01f;
}

void EFXRack::setChorusParams(bool enabled, int mode, float rateHz, float depthPct,
                             float haasDelayMs, float haasWidthPct, float mixPct) {
    m_chorusEnabled = enabled;
    m_chorusMode = mode;
    m_chorusRate = std::clamp(rateHz, 0.05f, 8.0f);
    m_chorusDepth = depthPct * 0.01f;
    m_haasDelayMs = std::clamp(haasDelayMs, 1.0f, 25.0f);
    m_haasWidthPct = haasWidthPct * 0.01f;
    m_chorusMix = mixPct * 0.01f;

    // Trigger mono-compatibility phase warning when delay > 18ms
    m_haasPhaseWarning = (m_chorusMode == 1 && m_haasDelayMs > 18.0f && m_chorusEnabled);
}

void EFXRack::setDistParams(bool enabled, int mode, float drivePct, float toneFreqHz, float mixPct) {
    m_distEnabled = enabled;
    m_distMode = mode;
    // Logarithmic drive curve: 0% to 100% maps to 0 to +32 dB drive
    float driveDb = (std::clamp(drivePct, 0.0f, 100.0f) * 0.01f) * 32.0f;
    m_distDrive = juce::Decibels::decibelsToGain(driveDb);
    // Smooth auto-makeup normalization so higher drive adds rich harmonic thickness without volume drops or deafening bursts
    m_distMakeup = 1.0f / std::pow(m_distDrive, 0.65f);
    m_distToneFreq = std::clamp(toneFreqHz, 1000.0f, 20000.0f);
    m_distMix = std::clamp(mixPct, 0.0f, 100.0f) * 0.01f;

    if (std::abs(m_distToneFreq - m_prevDistToneFreq) > 1.0f || m_sampleRate != m_prevDistSampleRate) {
        updateDistToneFilter();
    }
}

void EFXRack::updateDistToneFilter() {
    m_prevDistToneFreq = m_distToneFreq;
    m_prevDistSampleRate = m_sampleRate;

    float nyquist = static_cast<float>(m_sampleRate * 0.499);
    float tone = std::clamp(m_distToneFreq, 500.0f, nyquist);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(m_sampleRate, tone, 0.707f);
    m_distToneL.coefficients = lpCoeffs;
    m_distToneR.coefficients = lpCoeffs;
}

void EFXRack::setDelayParams(bool enabled, int mode, float timeMs, float feedbackPct,
                            float freqShiftHz, float diffusionPct, float toneFreqHz, float mixPct) {
    m_delayEnabled = enabled;
    m_delayMode = mode;
    m_delayTimeMs = std::clamp(timeMs, 1.0f, 2000.0f);
    m_delayFeedback = std::clamp(feedbackPct * 0.01f, 0.0f, 0.95f);
    m_delayFreqShiftHz = std::clamp(freqShiftHz, -100.0f, 100.0f);
    m_delayDiffusion = std::clamp(diffusionPct * 0.01f, 0.0f, 1.0f);
    m_delayToneFreq = std::clamp(toneFreqHz, 1000.0f, 20000.0f);
    m_delayMix = std::clamp(mixPct * 0.01f, 0.0f, 1.0f);

    float baseSamples = m_delayTimeMs * 0.001f * static_cast<float>(m_sampleRate);
    m_targetDelaySamplesL = baseSamples;
    // In Ping-Pong mode, Right delay is offset for alternating ping-pong bounce
    m_targetDelaySamplesR = (m_delayMode == 1) ? (baseSamples * 0.75f) : baseSamples;

    // Feedback tone lowpass filter coefficient
    float dt = 1.0f / static_cast<float>(m_sampleRate);
    float rcLpf = 1.0f / (2.0f * 3.14159265f * m_delayToneFreq);
    m_delayLpfCoeff = dt / (rcLpf + dt);

    // Feedback highpass filter coefficient (cutoff ~80 Hz)
    float rcHpf = 1.0f / (2.0f * 3.14159265f * 80.0f);
    m_delayHpfCoeff = rcHpf / (rcHpf + dt);
}

void EFXRack::process(float* left, float* right, int numSamples) {
    if (m_masterBypass || left == nullptr || numSamples <= 0) {
        m_compGRDb = 0.0f;
        return;
    }

    bool isStereo = (right != nullptr);

    // 1. 4-Band Sculpt EQ
    if (m_eqEnabled) {
        for (int i = 0; i < numSamples; ++i) {
            float sL = m_eqAirL.processSample(m_eqHighMidL.processSample(m_eqLowMidL.processSample(m_eqHpfL.processSample(left[i]))));
            left[i] = sL * m_eqOutGainLinear;

            if (isStereo) {
                float sR = m_eqAirR.processSample(m_eqHighMidR.processSample(m_eqLowMidR.processSample(m_eqHpfR.processSample(right[i]))));
                right[i] = sR * m_eqOutGainLinear;
            }
        }
    }

    // 2. Vocal FET/Opto Compressor
    if (m_compEnabled) {
        float maxGR = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            float inLevel = std::abs(left[i]);
            if (isStereo) inLevel = std::max(inLevel, std::abs(right[i]));

            // Envelope follower
            float coeff = (inLevel > m_compEnvL) ? m_compAttackCoeff : m_compReleaseCoeff;
            m_compEnvL = coeff * m_compEnvL + (1.0f - coeff) * inLevel;

            float envDb = juce::Decibels::gainToDecibels(std::max(1e-5f, m_compEnvL));
            float grDb = 0.0f;
            if (envDb > m_compThresholdDb) {
                float overshoot = envDb - m_compThresholdDb;
                grDb = overshoot * (1.0f - 1.0f / m_compRatio);
            }
            if (grDb > maxGR) maxGR = grDb;

            float compGain = juce::Decibels::decibelsToGain(-grDb) * m_compMakeupLinear;
            left[i] *= compGain;
            if (isStereo) right[i] *= compGain;
        }
        m_compGRDb = maxGR;
    } else {
        m_compGRDb = 0.0f;
    }

    // 3. Flanger
    if (m_flangerEnabled && !m_flangerDelayBufferL.empty()) {
        int bufSize = static_cast<int>(m_flangerDelayBufferL.size());
        float phaseInc = static_cast<float>(2.0 * 3.14159265 * m_flangerRate / m_sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            // LFO sweep between 0.5 ms and 6.0 ms
            float lfoL = 0.5f + 0.5f * std::sin(m_flangerPhase);
            float delayMsL = 0.5f + lfoL * (5.5f * m_flangerDepth);
            float delaySamplesL = delayMsL * 0.001f * static_cast<float>(m_sampleRate);

            // Linear interpolation delay read
            float readPosL = static_cast<float>(m_flangerWritePos) - delaySamplesL;
            while (readPosL < 0.0f) readPosL += static_cast<float>(bufSize);
            int idx0 = static_cast<int>(readPosL);
            int idx1 = (idx0 + 1) % bufSize;
            float frac = readPosL - static_cast<float>(idx0);

            float wetL = (1.0f - frac) * m_flangerDelayBufferL[idx0] + frac * m_flangerDelayBufferL[idx1];
            m_flangerDelayBufferL[m_flangerWritePos] = left[i] + wetL * m_flangerFeedback;
            left[i] = (1.0f - m_flangerMix) * left[i] + m_flangerMix * wetL;

            if (isStereo) {
                float lfoR = 0.5f + 0.5f * std::cos(m_flangerPhase); // 90 deg quadrature
                float delayMsR = 0.5f + lfoR * (5.5f * m_flangerDepth);
                float delaySamplesR = delayMsR * 0.001f * static_cast<float>(m_sampleRate);

                float readPosR = static_cast<float>(m_flangerWritePos) - delaySamplesR;
                while (readPosR < 0.0f) readPosR += static_cast<float>(bufSize);
                int rIdx0 = static_cast<int>(readPosR);
                int rIdx1 = (rIdx0 + 1) % bufSize;
                float rFrac = readPosR - static_cast<float>(rIdx0);

                float wetR = (1.0f - rFrac) * m_flangerDelayBufferR[rIdx0] + rFrac * m_flangerDelayBufferR[rIdx1];
                m_flangerDelayBufferR[m_flangerWritePos] = right[i] + wetR * m_flangerFeedback;
                right[i] = (1.0f - m_flangerMix) * right[i] + m_flangerMix * wetR;
            }

            m_flangerWritePos = (m_flangerWritePos + 1) % bufSize;
            m_flangerPhase += phaseInc;
            if (m_flangerPhase > 6.2831853f) m_flangerPhase -= 6.2831853f;
        }
    }

    // 4. Chorus / Haas Widener
    if (m_chorusEnabled && !m_chorusDelayBufferL.empty()) {
        int bufSize = static_cast<int>(m_chorusDelayBufferL.size());
        float phaseInc = static_cast<float>(2.0 * 3.14159265 * m_chorusRate / m_sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float inL = left[i];
            float inR = isStereo ? right[i] : left[i];

            m_chorusDelayBufferL[m_chorusWritePos] = inL;
            m_chorusDelayBufferR[m_chorusWritePos] = inR;

            float wetL = inL;
            float wetR = inR;

            if (m_chorusMode == 0) {
                // Modulated Chorus (10-30 ms LFO delay)
                float lfoL = 0.5f + 0.5f * std::sin(m_chorusPhase);
                float lfoR = 0.5f + 0.5f * std::cos(m_chorusPhase);

                float dL = (10.0f + lfoL * 15.0f * m_chorusDepth) * 0.001f * static_cast<float>(m_sampleRate);
                float dR = (10.0f + lfoR * 15.0f * m_chorusDepth) * 0.001f * static_cast<float>(m_sampleRate);

                float rPosL = static_cast<float>(m_chorusWritePos) - dL;
                while (rPosL < 0.0f) rPosL += static_cast<float>(bufSize);
                int idxL0 = static_cast<int>(rPosL);
                int idxL1 = (idxL0 + 1) % bufSize;
                wetL = (1.0f - (rPosL - idxL0)) * m_chorusDelayBufferL[idxL0] + (rPosL - idxL0) * m_chorusDelayBufferL[idxL1];

                float rPosR = static_cast<float>(m_chorusWritePos) - dR;
                while (rPosR < 0.0f) rPosR += static_cast<float>(bufSize);
                int idxR0 = static_cast<int>(rPosR);
                int idxR1 = (idxR0 + 1) % bufSize;
                wetR = (1.0f - (rPosR - idxR0)) * m_chorusDelayBufferR[idxR0] + (rPosR - idxR0) * m_chorusDelayBufferR[idxR1];
            } else {
                // Haas Widener: Centered stereo Haas spread
                // Rather than delaying only R (which pulls stereo image lopsidedly to Left ear),
                // we use a center nominal delay (15 ms) and offset L by -delta and R by +delta.
                // This preserves center vocal focus while creating expansive 3D width!
                float deltaMs = (m_haasDelayMs * 0.5f) * m_haasWidthPct;
                float delayMsL = std::max(0.5f, 15.0f - deltaMs);
                float delayMsR = std::min(35.0f, 15.0f + deltaMs);

                float dSamplesL = delayMsL * 0.001f * static_cast<float>(m_sampleRate);
                float dSamplesR = delayMsR * 0.001f * static_cast<float>(m_sampleRate);

                float rPosL = static_cast<float>(m_chorusWritePos) - dSamplesL;
                while (rPosL < 0.0f) rPosL += static_cast<float>(bufSize);
                int idxL0 = static_cast<int>(rPosL);
                int idxL1 = (idxL0 + 1) % bufSize;
                float fracL = rPosL - static_cast<float>(idxL0);
                wetL = (1.0f - fracL) * m_chorusDelayBufferL[idxL0] + fracL * m_chorusDelayBufferL[idxL1];

                float rPosR = static_cast<float>(m_chorusWritePos) - dSamplesR;
                while (rPosR < 0.0f) rPosR += static_cast<float>(bufSize);
                int idxR0 = static_cast<int>(rPosR);
                int idxR1 = (idxR0 + 1) % bufSize;
                float fracR = rPosR - static_cast<float>(idxR0);
                wetR = (1.0f - fracR) * m_chorusDelayBufferR[idxR0] + fracR * m_chorusDelayBufferR[idxR1];
            }

            left[i] = (1.0f - m_chorusMix) * inL + m_chorusMix * wetL;
            if (isStereo) {
                right[i] = (1.0f - m_chorusMix) * inR + m_chorusMix * wetR;
            }

            m_chorusWritePos = (m_chorusWritePos + 1) % bufSize;
            m_chorusPhase += phaseInc;
            if (m_chorusPhase > 6.2831853f) m_chorusPhase -= 6.2831853f;
        }
    }

    // 5. Waveshaping Distortion
    if (m_distEnabled) {
        for (int i = 0; i < numSamples; ++i) {
            float inL = left[i];
            float driveL = inL * m_distDrive;
            float shapedL = 0.0f;

            if (m_distMode == 0) {
                // Warm Tube: asymmetric soft saturator with 2nd harmonic warmth
                float x = driveL;
                float y = (x >= 0.0f) ? (1.5f * x / (1.0f + 0.7f * x)) : (x / (1.0f - 0.3f * x));
                shapedL = y * m_distMakeup;
            } else if (m_distMode == 1) {
                // Tape: tanh with mild 3rd harmonic compression
                float x = driveL;
                float y = std::tanh(x);
                shapedL = y * m_distMakeup;
            } else {
                // Hard Clip: edgy vocal distortion
                float x = driveL;
                float threshold = 0.5f;
                float y = std::clamp(x, -threshold, threshold) / threshold;
                shapedL = y * (threshold * m_distMakeup * 1.5f);
            }

            // Post-drive tone lowpass filter
            float filteredL = m_distToneL.processSample(shapedL);
            left[i] = (1.0f - m_distMix) * inL + m_distMix * filteredL;

            if (isStereo) {
                float inR = right[i];
                float driveR = inR * m_distDrive;
                float shapedR = 0.0f;
                if (m_distMode == 0) {
                    float x = driveR;
                    float y = (x >= 0.0f) ? (1.5f * x / (1.0f + 0.7f * x)) : (x / (1.0f - 0.3f * x));
                    shapedR = y * m_distMakeup;
                } else if (m_distMode == 1) {
                    float x = driveR;
                    float y = std::tanh(x);
                    shapedR = y * m_distMakeup;
                } else {
                    float x = driveR;
                    float threshold = 0.5f;
                    float y = std::clamp(x, -threshold, threshold) / threshold;
                    shapedR = y * (threshold * m_distMakeup * 1.5f);
                }
                float filteredR = m_distToneR.processSample(shapedR);
                right[i] = (1.0f - m_distMix) * inR + m_distMix * filteredR;
            }
        }
    }

    // 6. Delay / Echo (H-Delay, Supermassive, FreqEcho)
    if (m_delayEnabled && !m_delayBufferL.empty()) {
        int bufSize = static_cast<int>(m_delayBufferL.size());
        float shiftPhaseInc = static_cast<float>(2.0 * 3.141592653589793 * m_delayFreqShiftHz / m_sampleRate);
        float diffG = 0.65f * m_delayDiffusion;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left[i];
            float inR = isStereo ? right[i] : left[i];

            // Smooth delay times for tape-style pitch-glide
            m_smoothedDelaySamplesL += 0.002f * (m_targetDelaySamplesL - m_smoothedDelaySamplesL);
            m_smoothedDelaySamplesR += 0.002f * (m_targetDelaySamplesR - m_smoothedDelaySamplesR);

            // Read Left channel from delay buffer with linear interpolation
            float rPosL = static_cast<float>(m_delayWritePos) - m_smoothedDelaySamplesL;
            while (rPosL < 0.0f) rPosL += static_cast<float>(bufSize);
            int idxL0 = static_cast<int>(rPosL);
            int idxL1 = (idxL0 + 1) % bufSize;
            float fracL = rPosL - static_cast<float>(idxL0);
            float wetL = (1.0f - fracL) * m_delayBufferL[idxL0] + fracL * m_delayBufferL[idxL1];

            // Read Right channel from delay buffer
            float rPosR = static_cast<float>(m_delayWritePos) - m_smoothedDelaySamplesR;
            while (rPosR < 0.0f) rPosR += static_cast<float>(bufSize);
            int idxR0 = static_cast<int>(rPosR);
            int idxR1 = (idxR0 + 1) % bufSize;
            float fracR = rPosR - static_cast<float>(idxR0);
            float wetR = (1.0f - fracR) * m_delayBufferR[idxR0] + fracR * m_delayBufferR[idxR1];

            // Feedback signal routing (Ping-Pong vs Stereo)
            float fbL = (m_delayMode == 1) ? wetR : wetL;
            float fbR = (m_delayMode == 1) ? wetL : wetR;

            // 1. Bode Frequency Shifter in feedback loop (Valhalla FreqEcho)
            if (std::abs(m_delayFreqShiftHz) > 0.05f) {
                float realL = 0.0f, imagL = 0.0f;
                m_hilbertL.process(fbL, realL, imagL);
                float cL = std::cos(m_shiftPhaseL);
                float sL = std::sin(m_shiftPhaseL);
                fbL = (m_delayFreqShiftHz >= 0.0f) ? (realL * cL - imagL * sL) : (realL * cL + imagL * sL);

                float realR = 0.0f, imagR = 0.0f;
                m_hilbertR.process(fbR, realR, imagR);
                float cR = std::cos(m_shiftPhaseR);
                float sR = std::sin(m_shiftPhaseR);
                fbR = (m_delayFreqShiftHz >= 0.0f) ? (realR * cR - imagR * sR) : (realR * cR + imagR * sR);

                m_shiftPhaseL += shiftPhaseInc;
                m_shiftPhaseR += shiftPhaseInc;
                if (m_shiftPhaseL > 6.2831853f) m_shiftPhaseL -= 6.2831853f;
                if (m_shiftPhaseL < -6.2831853f) m_shiftPhaseL += 6.2831853f;
                if (m_shiftPhaseR > 6.2831853f) m_shiftPhaseR -= 6.2831853f;
                if (m_shiftPhaseR < -6.2831853f) m_shiftPhaseR += 6.2831853f;
            }

            // 2. Spatial Diffusion in feedback loop (Valhalla Supermassive)
            if (m_delayDiffusion > 0.01f) {
                for (int s = 0; s < 4; ++s) {
                    fbL = m_diffusersL[s].process(fbL, diffG);
                    fbR = m_diffusersR[s].process(fbR, diffG);
                }
            }

            // 3. Feedback Tone Filtering: HPF (80 Hz) to remove sub mud + LPF (Tone) for tape damping
            float hpfL = m_delayHpfCoeff * (m_delayHpfStateL + fbL);
            m_delayHpfStateL = hpfL - fbL;
            fbL = hpfL;

            float hpfR = m_delayHpfCoeff * (m_delayHpfStateR + fbR);
            m_delayHpfStateR = hpfR - fbR;
            fbR = hpfR;

            m_delayLpfStateL += m_delayLpfCoeff * (fbL - m_delayLpfStateL);
            fbL = m_delayLpfStateL;

            m_delayLpfStateR += m_delayLpfCoeff * (fbR - m_delayLpfStateR);
            fbR = m_delayLpfStateR;

            // 4. Soft-Knee Analog Tape Saturation on feedback path (Waves H-Delay)
            float satFbL = std::tanh(1.2f * fbL) * 0.9f;
            float satFbR = std::tanh(1.2f * fbR) * 0.9f;

            // Write input + feedback into delay line
            m_delayBufferL[m_delayWritePos] = inL + satFbL * m_delayFeedback;
            m_delayBufferR[m_delayWritePos] = inR + satFbR * m_delayFeedback;

            m_delayWritePos = (m_delayWritePos + 1) % bufSize;

            // Wet / Dry Output
            left[i] = (1.0f - m_delayMix) * inL + m_delayMix * wetL;
            if (isStereo) {
                right[i] = (1.0f - m_delayMix) * inR + m_delayMix * wetR;
            }
        }
    }

    // Final safety sanitation
    for (int i = 0; i < numSamples; ++i) {
        if (!std::isfinite(left[i])) left[i] = 0.0f;
        if (isStereo && !std::isfinite(right[i])) right[i] = 0.0f;
    }
}
