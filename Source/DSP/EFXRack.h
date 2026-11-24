#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

class EFXRack {
public:
    EFXRack();
    ~EFXRack() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Master & module bypasses
    void setMasterBypass(bool bypass) { m_masterBypass = bypass; }

    // EQ parameters
    void setEqParams(bool enabled, float hpfFreq, float lowMidFreq, float lowMidGainDb,
                     float highMidFreq, float highMidGainDb, float airFreq, float airGainDb,
                     float outGainDb);
    float getEqResponseAt(float freqHz) const;

    // Compressor parameters
    void setCompParams(bool enabled, float thresholdDb, float ratio, float attackMs,
                       float releaseMs, float makeupDb);
    float getCompressorGainReductionDb() const { return m_compGRDb; }

    // Flanger parameters
    void setFlangerParams(bool enabled, float rateHz, float depthPct, float feedbackPct, float mixPct);

    // Chorus / Haas parameters
    // mode: 0 = Chorus, 1 = Haas
    void setChorusParams(bool enabled, int mode, float rateHz, float depthPct,
                         float haasDelayMs, float haasWidthPct, float mixPct);
    bool getHaasPhaseWarning() const { return m_haasPhaseWarning; }

    // Distortion parameters
    // mode: 0 = Warm Tube, 1 = Tape, 2 = Hard Clip
    void setDistParams(bool enabled, int mode, float drivePct, float toneFreqHz, float mixPct);

    // Delay / Echo (H-Delay, Supermassive, FreqEcho)
    // mode: 0 = Stereo, 1 = Ping-Pong
    void setDelayParams(bool enabled, int mode, float timeMs, float feedbackPct,
                        float freqShiftHz, float diffusionPct, float toneFreqHz, float mixPct);

    // Main stereo in/out processing block
    void process(float* leftChannel, float* rightChannel, int numSamples);

private:
    double m_sampleRate = 48000.0;
    bool m_masterBypass = false;

    // 1. EQ
    bool m_eqEnabled = true;
    float m_hpfFreq = 80.0f;
    float m_lowMidFreq = 500.0f;
    float m_lowMidGainDb = 0.0f;
    float m_highMidFreq = 3000.0f;
    float m_highMidGainDb = 0.0f;
    float m_airFreq = 12000.0f;
    float m_airGainDb = 0.0f;
    float m_eqOutGainLinear = 1.0f;

    juce::dsp::IIR::Filter<float> m_eqHpfL, m_eqHpfR;
    juce::dsp::IIR::Filter<float> m_eqLowMidL, m_eqLowMidR;
    juce::dsp::IIR::Filter<float> m_eqHighMidL, m_eqHighMidR;
    juce::dsp::IIR::Filter<float> m_eqAirL, m_eqAirR;
    void updateEqCoefficients();

    // 2. Compressor
    bool m_compEnabled = true;
    float m_compThresholdDb = -18.0f;
    float m_compRatio = 3.5f;
    float m_compAttackCoeff = 0.0f;
    float m_compReleaseCoeff = 0.0f;
    float m_compMakeupLinear = 1.0f;
    float m_compEnvL = 0.0f;
    float m_compEnvR = 0.0f;
    float m_compGRDb = 0.0f;

    // 3. Flanger
    bool m_flangerEnabled = false;
    float m_flangerRate = 0.5f;
    float m_flangerDepth = 0.5f;
    float m_flangerFeedback = 0.3f;
    float m_flangerMix = 0.4f;
    float m_flangerPhase = 0.0f;
    std::vector<float> m_flangerDelayBufferL;
    std::vector<float> m_flangerDelayBufferR;
    int m_flangerWritePos = 0;

    // 4. Chorus / Haas
    bool m_chorusEnabled = false;
    int m_chorusMode = 0; // 0 = Chorus, 1 = Haas
    float m_chorusRate = 1.2f;
    float m_chorusDepth = 0.4f;
    float m_haasDelayMs = 12.0f;
    float m_haasWidthPct = 0.8f;
    float m_chorusMix = 0.5f;
    float m_chorusPhase = 0.0f;
    bool m_haasPhaseWarning = false;
    std::vector<float> m_chorusDelayBufferL;
    std::vector<float> m_chorusDelayBufferR;
    int m_chorusWritePos = 0;

    // 5. Distortion
    bool m_distEnabled = false;
    int m_distMode = 0; // 0 = Warm Tube, 1 = Tape, 2 = Hard Clip
    float m_distDrive = 1.0f;
    float m_distMakeup = 1.0f;
    float m_distMix = 0.3f;
    float m_distToneFreq = 8000.0f;
    juce::dsp::IIR::Filter<float> m_distToneL, m_distToneR;
    void updateDistToneFilter();

    // Dirty-flag caches for zero-allocation steady state
    float m_prevHpfFreq = -1.0f;
    float m_prevLowMidFreq = -1.0f;
    float m_prevLowMidGainDb = -999.0f;
    float m_prevHighMidFreq = -1.0f;
    float m_prevHighMidGainDb = -999.0f;
    float m_prevAirFreq = -1.0f;
    float m_prevAirGainDb = -999.0f;
    double m_prevEqSampleRate = -1.0;

    float m_prevDistToneFreq = -1.0f;
    double m_prevDistSampleRate = -1.0;

    // 6. Delay / Echo (H-Delay, Supermassive, FreqEcho)
    bool m_delayEnabled = false;
    int m_delayMode = 0; // 0 = Stereo, 1 = Ping-Pong
    float m_delayTimeMs = 250.0f;
    float m_targetDelaySamplesL = 12000.0f;
    float m_targetDelaySamplesR = 12000.0f;
    float m_smoothedDelaySamplesL = 12000.0f;
    float m_smoothedDelaySamplesR = 12000.0f;
    float m_delayFeedback = 0.35f;
    float m_delayFreqShiftHz = 0.0f;
    float m_delayDiffusion = 0.0f;
    float m_delayToneFreq = 6000.0f;
    float m_delayMix = 0.30f;

    std::vector<float> m_delayBufferL;
    std::vector<float> m_delayBufferR;
    int m_delayWritePos = 0;

    // Hilbert 90-degree phase splitter state for FreqEcho frequency shifting
    struct HilbertPair {
        float x1_a[2] = {0.0f, 0.0f};
        float y1_a[2] = {0.0f, 0.0f};
        float x1_b[2] = {0.0f, 0.0f};
        float y1_b[2] = {0.0f, 0.0f};

        void reset() {
            x1_a[0] = x1_a[1] = y1_a[0] = y1_a[1] = 0.0f;
            x1_b[0] = x1_b[1] = y1_b[0] = y1_b[1] = 0.0f;
        }

        void process(float in, float& outReal, float& outImag) {
            // Path A (in-phase)
            float c0 = 0.6923878f;
            float c1 = 0.9360654f;
            float w0 = c0 * in + x1_a[0] - c0 * y1_a[0];
            x1_a[0] = in;
            y1_a[0] = w0;
            float w1 = c1 * w0 + x1_a[1] - c1 * y1_a[1];
            x1_a[1] = w0;
            y1_a[1] = w1;
            outReal = w1;

            // Path B (quadrature 90 deg)
            float d0 = 0.4021921f;
            float d1 = 0.8561710f;
            float u0 = d0 * in + x1_b[0] - d0 * y1_b[0];
            x1_b[0] = in;
            y1_b[0] = u0;
            float u1 = d1 * u0 + x1_b[1] - d1 * y1_b[1];
            x1_b[1] = u0;
            y1_b[1] = u1;
            outImag = u1;
        }
    };

    HilbertPair m_hilbertL, m_hilbertR;
    float m_shiftPhaseL = 0.0f;
    float m_shiftPhaseR = 0.0f;

    // Supermassive Allpass Diffusers: 4 stages per channel
    struct AllpassDiffuser {
        std::vector<float> buffer;
        int writePos = 0;
        int delay = 149;

        void init(int len) {
            delay = len;
            buffer.assign(len + 4, 0.0f);
            writePos = 0;
        }

        void reset() {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writePos = 0;
        }

        inline float process(float in, float g) {
            if (g <= 0.001f || buffer.empty()) return in;
            int readPos = writePos - delay;
            if (readPos < 0) readPos += static_cast<int>(buffer.size());
            float bufVal = buffer[readPos];
            float out = -g * in + bufVal;
            buffer[writePos] = in + g * out;
            writePos = (writePos + 1) % static_cast<int>(buffer.size());
            return out;
        }
    };

    AllpassDiffuser m_diffusersL[4];
    AllpassDiffuser m_diffusersR[4];

    // Feedback tone filter (One-pole HPF + One-pole LPF)
    float m_delayHpfStateL = 0.0f;
    float m_delayHpfStateR = 0.0f;
    float m_delayLpfStateL = 0.0f;
    float m_delayLpfStateR = 0.0f;
    float m_delayLpfCoeff = 0.8f;
    float m_delayHpfCoeff = 0.985f;
};
