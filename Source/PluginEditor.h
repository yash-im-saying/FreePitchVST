#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/FreePitchLookAndFeel.h"
#include "UI/CircularPitchMeter.h"
#include "UI/PianoKeyboardComponent.h"
#include "UI/EFXDrawerComponent.h"
#include "UI/RotaryKnob.h"

class FreePitchAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer {
public:
    explicit FreePitchAudioProcessorEditor(FreePitchAudioProcessor&);
    ~FreePitchAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    FreePitchAudioProcessor& m_processor;
    FreePitchLookAndFeel m_lookAndFeel;

    // Integrated Studio Tooltip Window
    juce::TooltipWindow m_tooltipWindow { this, 400 };

    // --- Top Bar Components ---
    juce::Label m_titleLabel;
    juce::Label m_subtitleLabel;
    juce::Label m_versionLabel;

    // Presets
    juce::TextButton m_prevPresetBtn { "<" };
    juce::TextButton m_nextPresetBtn { ">" };
    juce::ComboBox   m_presetBox;
    juce::TextButton m_savePresetBtn { "SAVE" };
    juce::TextButton m_abCompareBtn  { "A / B" };

    // Latency & Mode Switch
    juce::ToggleButton m_lookaheadToggle { "LOOKAHEAD" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_lookaheadAttachment;
    juce::Label m_latencyLabel;

    // Key & Scale
    juce::ComboBox m_keyRootBox;
    juce::ComboBox m_scaleTypeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_keyRootAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_scaleTypeAttachment;

    // Master EFX Bypass
    juce::ToggleButton m_masterEfxToggle { "MASTER EFX" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_masterEfxAttachment;

    // --- Left Control Bank (Tuning Dynamics) ---
    std::unique_ptr<RotaryKnob> m_retuneSpeedKnob;
    std::unique_ptr<RotaryKnob> m_humanizeKnob;
    std::unique_ptr<RotaryKnob> m_flexTuneKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_retuneSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_humanizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_flexTuneAttachment;

    // --- Center Focal Point (Pitch Radar) ---
    CircularPitchMeter m_circularMeter;
    juce::ComboBox m_vocalModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_vocalModeAttachment;
    juce::ToggleButton m_choosyTrackingToggle { "CHOOSY TRACKING" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_choosyTrackingAttachment;

    // --- Right Control Bank (Tone & Modular EFX) ---
    std::unique_ptr<RotaryKnob> m_pitchShiftKnob;
    juce::Label m_formantsTitleLabel;
    juce::ToggleButton m_formantsToggle { "PRESERVE" };
    juce::Label m_formantsSubLabel;
    std::unique_ptr<RotaryKnob> m_formantShiftKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_pitchShiftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_formantsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_formantShiftAttachment;

    // Voiced Confidence Meter
    float m_smoothConfidence = 0.0f;
    float m_smoothInputRms = -100.0f;
    juce::Rectangle<float> m_confidenceMeterBounds;

    // Modular Post-Pitch EFX Rack Bay
    EFXDrawerComponent m_efxDrawer;

    // --- Bottom Piano Keyboard ---
    PianoKeyboardComponent m_pianoKeyboard;

    void updatePresetBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FreePitchAudioProcessorEditor)
};
