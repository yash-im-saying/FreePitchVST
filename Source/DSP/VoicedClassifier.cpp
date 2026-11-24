#include "VoicedClassifier.h"
#include <cmath>
#include <algorithm>

void VoicedClassifier::prepare(double sampleRate) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;
    reset();
}

void VoicedClassifier::reset() {
    m_zcr = 0.0f;
    m_isConsonant = false;
    m_voicedProb = 0.0f;
    m_unvoicedHoldFrames = 0;
    m_lastVoicedState = false;
}

bool VoicedClassifier::classify(const float* buffer, int numSamples, float mpmConfidence, float rmsDb) {
    if (buffer == nullptr || numSamples <= 1) {
        m_lastVoicedState = false;
        m_voicedProb = 0.0f;
        return false;
    }

    // 1. Check Noise Gate (-65 dB in standard mode, -50 dB in Choosy mode for noisy/breathy vocals)
    float gateThreshold = m_choosyTracking ? -50.0f : -65.0f;
    if (rmsDb < gateThreshold) {
        m_isConsonant = false;
        m_voicedProb = 0.0f;
        m_lastVoicedState = false;
        m_unvoicedHoldFrames = 0;
        return false;
    }

    // 2. Compute Zero-Crossing Rate (ZCR)
    int zeroCrossings = 0;
    for (int i = 1; i < numSamples; ++i) {
        if ((buffer[i] >= 0.0f && buffer[i - 1] < 0.0f) ||
            (buffer[i] < 0.0f && buffer[i - 1] >= 0.0f)) {
            zeroCrossings++;
        }
    }
    m_zcr = static_cast<float>(zeroCrossings) / static_cast<float>(numSamples - 1);

    // Sibilants/consonants ('s', 'sh', 't', 'ch', 'f') typically exhibit high ZCR and low MPM confidence
    float zcrLimit = m_choosyTracking ? 0.20f : 0.28f;
    float confLimit = m_choosyTracking ? 0.70f : 0.55f;
    bool highZcr = (m_zcr > zcrLimit);
    bool lowConfidence = (mpmConfidence < confLimit);

    m_isConsonant = highZcr && lowConfidence;

    // Voiced condition: Confidence and moderate ZCR without sibilance
    float minVoicedConf = m_choosyTracking ? 0.72f : 0.58f;
    bool currentlyVoiced = (mpmConfidence >= minVoicedConf) && (m_zcr <= zcrLimit) && (!m_isConsonant);

    // Continuous Voiced Probability (0.0 to 1.0)
    float zcrFactor = std::clamp(1.0f - (m_zcr / 0.30f), 0.0f, 1.0f);
    m_voicedProb = std::clamp(mpmConfidence * zcrFactor, 0.0f, 1.0f);

    // Hysteresis: Hold pitch state across micro-dropouts (up to 5 frames ~ 13ms),
    // but do NOT hold if it's an explicit consonant
    if (currentlyVoiced) {
        m_lastVoicedState = true;
        m_unvoicedHoldFrames = m_choosyTracking ? 1 : m_maxHoldFrames;
        return true;
    } else {
        if (m_isConsonant) {
            // Immediate cut for consonants to prevent pitch-mangling sibilants
            m_unvoicedHoldFrames = 0;
            m_lastVoicedState = false;
            return false;
        }

        if (m_unvoicedHoldFrames > 0 && m_lastVoicedState) {
            m_unvoicedHoldFrames--;
            return true; // bridge temporary dropout
        } else {
            m_lastVoicedState = false;
            return false;
        }
    }
}
