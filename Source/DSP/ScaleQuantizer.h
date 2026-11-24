#pragma once

#include "Parameters.h"
#include <vector>
#include <cstdint>

class ScaleQuantizer {
public:
    ScaleQuantizer();
    ~ScaleQuantizer() = default;

    void prepare(double sampleRate);
    void reset();

    void setKeyRoot(int rootIndex); // 0 = C, 1 = C#, ... 11 = B
    void setScaleType(ScaleType scale);
    void setCustomMask(uint32_t mask12Bit); // Bit 0 = C, Bit 1 = C#, etc.

    void setRetuneSpeedMs(float speedMs);
    void setHumanize(float humanizePercent);
    void setFlexTune(float flexTunePercent);
    void setPitchShift(float semitones);

    // Updates tuning state given detected fundamental frequency in Hz
    // Returns target frequency in Hz, and populates cents deviation & applied correction
    float processTuning(float detectedFreqHz, bool isVoiced, float dtSeconds,
                        float& outCentsDev, float& outAppliedCorrectionCents,
                        int& outDetectedMidi, int& outTargetMidi);

    bool isNoteInScale(int midiNote) const;

private:
    double m_sampleRate = 48000.0;

    int m_keyRoot = 0; // C
    ScaleType m_scaleType = ScaleType::Chromatic;
    uint32_t m_customMask = 0x0FFF;

    float m_retuneSpeedMs = 20.0f;
    float m_humanize = 30.0f;
    float m_flexTune = 20.0f;
    float m_pitchShift = 0.0f;

    // Internal state
    float m_currentMidiPitch = 60.0f; // Smoothed current pitch
    int   m_currentTargetMidi = -1;
    float m_timeOnCurrentNoteSec = 0.0f;
    bool  m_wasVoiced = false;

    // Pitch stability / humanize tracker
    static constexpr int kHistorySize = 32;
    float m_pitchHistory[kHistorySize];
    int m_historyIndex = 0;
    int m_historyCount = 0;
    float m_sustainedTimeSec = 0.0f;

    int findNearestScaleMidi(float continuousMidi) const;
    uint16_t getScaleBitmask() const;
};
