#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>
#include <algorithm>

class FormantEngine {
public:
    FormantEngine();
    ~FormantEngine() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    int getLatencySamples() const { return m_latencySamples; }

    // Continuous real-time STFT overlap-add spectral envelope formant processor
    void process(float* buffer, int numSamples, float inputPitchHz,
                 float pitchRatio, float formantShiftSemitones, bool formantsEnabled);

private:
    double m_sampleRate = 48000.0;
    int m_fftOrder = 10; // 2^10 = 1024
    int m_fftSize = 1024;
    int m_hopSize = 256; // 75% overlap
    int m_latencySamples = 1024; // m_fftSize

    std::unique_ptr<juce::dsp::FFT> m_fft;

    // Hann analysis and synthesis windows
    std::vector<float> m_analysisWindow;
    std::vector<float> m_synthesisWindow;

    // Power-of-2 circular FIFOs for continuous overlap-add
    static constexpr int kFifoSize = 8192;
    static constexpr int kFifoMask = kFifoSize - 1;

    std::vector<float> m_inputFifo;
    std::vector<float> m_outputFifo;

    int64_t m_writePos = 0;
    int m_hopCounter = 0;

    // FFT scratch buffers
    std::vector<float> m_fftBuffer;
    std::vector<float> m_magSpectrum;
    std::vector<float> m_smoothedEnvelope;
    std::vector<float> m_targetEnvelope;
    std::vector<float> m_peakEnv;

    float m_smoothedWarpFactor = 1.0f;

    void computeSpectralEnvelope(const float* mag, float* envOut, int numBins, float pitchHz);
};
