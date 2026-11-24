#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "FreePitchLookAndFeel.h"

class CircularPitchMeter : public juce::Component {
public:
    CircularPitchMeter();
    ~CircularPitchMeter() override = default;

    void updateTelemetry(const PitchTelemetry& telemetry);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PitchTelemetry m_currentTelemetry;

    // Smoothed visual ballistics
    float m_smoothCentsDev = 0.0f;
    float m_smoothAppliedCorr = 0.0f;
    float m_smoothConfidence = 0.0f;

    static juce::String midiToNoteName(int midiNote);
};
