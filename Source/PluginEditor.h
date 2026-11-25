#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/FreePitchLookAndFeel.h"
#include "UI/CircularPitchMeter.h"
#include "UI/PianoKeyboardComponent.h"
#include "UI/EfxRackComponent.h"
#include "UI/RotaryKnob.h"
#include "UI/TactileWidgets.h"

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

    juce::TooltipWindow m_tooltipWindow { this, 400 };

    // --- Header Bar Components ---
    juce::Label m_titleLabel;
    juce::Label m_vst3Badge;

    // Preset Selector
    std::unique_ptr<HeaderPillSelector> m_presetSelector;

    // Key & Scale Selectors
    std::unique_ptr<HeaderPillSelector> m_keyScaleSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_keyRootAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_scaleTypeAttachment;

    // Latency Toggle (TRACK vs LOOKAHEAD)
    std::unique_ptr<SegmentedLatencyToggle> m_latencyToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_lookaheadAttachment;

    // Vocal Mode Selector
    std::unique_ptr<HeaderPillSelector> m_vocalModeSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_vocalModeAttachment;

    // Master Bypass Button
    std::unique_ptr<CircularBypassButton> m_bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_masterEfxAttachment;

    // --- Main Pitch Engine (Left 30% / Center 40% / Right 30%) ---
    // Left Column: Primary Tuning Knobs + Choosy Tracking Pill
    std::unique_ptr<RotaryKnob> m_retuneSpeedKnob;
    std::unique_ptr<RotaryKnob> m_humanizeKnob;
    std::unique_ptr<RotaryKnob> m_flexTuneKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_retuneSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_humanizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_flexTuneAttachment;

    std::unique_ptr<NeumorphicTogglePill> m_choosyTrackingPill;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_choosyTrackingAttachment;

    // Center Focal Point: Circular Pitch Meter
    CircularPitchMeter m_pitchMeter;

    // Right Column: Secondary Tone/Pitch Knobs + Formant Lock Pill
    std::unique_ptr<RotaryKnob> m_formantKnob;
    std::unique_ptr<RotaryKnob> m_transposeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_formantAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_transposeAttachment;

    std::unique_ptr<NeumorphicTogglePill> m_formantLockPill;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_formantsAttachment;

    // --- Post-Pitch EFX Rack ---
    EfxRackComponent m_efxRack;

    // --- Bottom Piano Keyboard ---
    PianoKeyboardComponent m_pianoKeyboard;

    void updatePresetBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FreePitchAudioProcessorEditor)
};
