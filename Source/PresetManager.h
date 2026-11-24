#pragma once

#include "Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

struct Preset {
    juce::String name;
    bool isFactory = true;
    std::map<juce::String, float> values;
};

class PresetManager {
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;

    void initPresets();

    int getNumPresets() const { return static_cast<int>(m_presets.size()); }
    juce::String getPresetName(int index) const;
    int getCurrentPresetIndex() const { return m_currentPresetIndex; }

    void loadPreset(int index);
    void nextPreset();
    void previousPreset();

    // A/B Comparison
    void toggleAB();
    bool isStateB() const { return m_isStateB; }

    // User Presets
    void saveUserPreset(const juce::String& name);
    void resetToDefault();

private:
    juce::AudioProcessorValueTreeState& m_apvts;
    std::vector<Preset> m_presets;
    int m_currentPresetIndex = 0;

    // A/B states
    juce::ValueTree m_stateA;
    juce::ValueTree m_stateB;
    bool m_isStateB = false;

    void applyPresetValues(const Preset& p);
};
