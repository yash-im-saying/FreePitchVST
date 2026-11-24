#pragma once

#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include <vector>

class PitchDetector {
public:
    PitchDetector();
    ~PitchDetector() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setVocalMode(VocalMode mode);
    void setChoosyTracking(bool choosy) { m_choosyTracking = choosy; }

    // Feeds incoming audio samples into the internal analysis ring buffer
    void pushSamples(const float* input, int numSamples);

    // Analyzes the current window and computes pitch
    // Returns true if a valid voiced pitch was detected within mode bounds
    bool processPitch(float& outPitchHz, float& outConfidence, float& outRms);

    float getSampleRate() const { return static_cast<float>(m_sampleRate); }

private:
    double m_sampleRate = 48000.0;
    VocalMode m_vocalMode = VocalMode::Tenor;
    bool m_choosyTracking = false;

    int m_windowSize = 2048;
    int m_fftSize = 4096;
    int m_fftOrder = 12; // 2^12 = 4096

    std::unique_ptr<juce::dsp::FFT> m_fft;

    std::vector<float> m_inputRingBuffer;
    int m_ringWritePos = 0;

    std::vector<float> m_analysisWindow;
    std::vector<float> m_fftBuffer;
    std::vector<float> m_nsdf;
    std::vector<float> m_squareSum;

    int m_minLag = 40;
    int m_maxLag = 800;

    int m_hopSize = 128;
    int m_samplesSinceLastAnalysis = 9999;
    float m_lastDetectedHz = 0.0f;
    float m_lastConfidence = 0.0f;
    float m_lastRms = 0.0f;
    bool  m_lastVoiced = false;

    void updateLagBounds();
    void computeNSDF();
};
