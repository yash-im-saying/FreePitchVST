#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "FreePitchLookAndFeel.h"

class CircularPitchMeter : public juce::Component,
                           public juce::SettableTooltipClient {
public:
    CircularPitchMeter();
    ~CircularPitchMeter() override = default;

    void updateTelemetry(const PitchTelemetry& telemetry);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PitchTelemetry m_currentTelemetry;

    // Smoothed visual ballistics for 60 FPS animation
    float m_smoothCentsDev = 0.0f;
    float m_smoothAppliedCorr = 0.0f;
    float m_smoothConfidence = 0.0f;
    float m_smoothFrequency = 0.0f;

    // Pre-rendered static dish and dial ticks cache
    juce::Image m_cachedMeterBase;

    void renderCachedMeterBase();
    static juce::String midiToNoteName(int midiNote);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularPitchMeter)
};

using PitchMeterComponent = CircularPitchMeter;
