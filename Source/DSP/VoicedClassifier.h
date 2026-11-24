#pragma once

#include <juce_core/juce_core.h>

class VoicedClassifier {
public:
    VoicedClassifier() = default;
    ~VoicedClassifier() = default;

    void prepare(double sampleRate);
    void reset();

    void setChoosyTracking(bool choosy) { m_choosyTracking = choosy; }

    // Evaluates whether current frame is Voiced (vowel/harmonic) or Unvoiced (consonant/sibilant/silence)
    bool classify(const float* buffer, int numSamples, float mpmConfidence, float rmsDb);

    float getZeroCrossingRate() const { return m_zcr; }
    bool isConsonant() const { return m_isConsonant; }
    float getVoicedProbability() const { return m_voicedProb; }

private:
    double m_sampleRate = 48000.0;
    bool m_choosyTracking = false;
    float m_zcr = 0.0f;
    bool m_isConsonant = false;
    float m_voicedProb = 0.0f;

    // Hysteresis counters
    int m_unvoicedHoldFrames = 0;
    int m_maxHoldFrames = 4; // ~15-20 ms at typical hop sizes
    bool m_lastVoicedState = false;
};
