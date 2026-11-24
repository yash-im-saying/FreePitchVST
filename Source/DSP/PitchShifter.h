#pragma once

#include "FormantEngine.h"
#include "Parameters.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

class PitchShifter {
public:
    PitchShifter();
    ~PitchShifter() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setLookaheadMode(bool lookaheadEnabled);
    bool isLookaheadMode() const { return m_lookaheadEnabled; }

    int getLatencySamples() const { return m_latencySamples + m_formantEngine.getLatencySamples(); }
    float getLatencyMs() const { return (static_cast<float>(getLatencySamples()) / static_cast<float>(m_sampleRate)) * 1000.0f; }

    void setVocalMode(VocalMode mode);

    // Processes mono input audio through synchronous splice pitch shifting
    void process(const float* input, float* output, int numSamples,
                 float inputPitchHz, float targetPitchHz, bool isVoiced,
                 bool formantsEnabled, float formantShiftSt);

private:
    double m_sampleRate = 48000.0;
    bool m_lookaheadEnabled = false;
    int m_latencySamples = 1024; // dynamically sized to vocal mode

    VocalMode m_vocalMode = VocalMode::Tenor;
    FormantEngine m_formantEngine;

    // Power-of-2 circular buffer: 65536 samples (1.36s at 48kHz, 341ms at 192kHz)
    static constexpr int kBufferSize = 65536;
    static constexpr int kBufferMask = kBufferSize - 1;

    std::vector<float> m_inputBuffer;
    std::vector<uint8_t> m_voicedBuffer;

    int64_t m_writeSampleCount = 0;

    // Dual-tap delay values (in fractional samples)
    double m_tapDelay1 = 1024.0;
    double m_tapDelay2 = 1024.0;

    bool m_crossfading = false;
    int  m_fadePos = 0;
    int  m_fadeLen = 256;

    // Per-sample smoothed pitch ratio to eliminate block-boundary rate kinks
    float m_smoothedAlpha = 1.0f;

    // Smoothed pitch periods
    float m_currentPitchPeriod = 240.0f;
    float m_targetPitchPeriod = 240.0f;

    // Voiced / Unvoiced smooth crossfade
    float m_voicedGain = 0.0f;

    void updateLatency();

    inline float readHermite(int64_t currentSample, double delaySamples) const {
        double readPos = static_cast<double>(currentSample) - delaySamples;
        int64_t i1 = static_cast<int64_t>(std::floor(readPos));
        float frac = static_cast<float>(readPos - static_cast<double>(i1));

        float y0 = m_inputBuffer[(i1 - 1) & kBufferMask];
        float y1 = m_inputBuffer[i1 & kBufferMask];
        float y2 = m_inputBuffer[(i1 + 1) & kBufferMask];
        float y3 = m_inputBuffer[(i1 + 2) & kBufferMask];

        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }
};
