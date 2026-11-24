#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "FreePitchLookAndFeel.h"
#include "RotaryKnob.h"
#include "DSP/EFXRack.h"

class EFXDrawerComponent : public juce::Component {
public:
    EFXDrawerComponent(juce::AudioProcessorValueTreeState& apvts, EFXRack& efxRack);
    ~EFXDrawerComponent() override = default;

    void updateTelemetry(const PitchTelemetry& telemetry);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& m_apvts;
    EFXRack& m_efxRack;
    PitchTelemetry m_telemetry;

    // Slot IDs: 0 = None (collapsed), 1 = EQ, 2 = Comp, 3 = Flanger, 4 = Chorus/Haas, 5 = Dist
    int m_activeDrawer = 1; // EQ open by default

    struct SlotButton {
        juce::ToggleButton powerBtn;
        juce::TextButton editBtn;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;
    };

    SlotButton m_eqSlot;
    SlotButton m_compSlot;
    SlotButton m_flangerSlot;
    SlotButton m_chorusSlot;
    SlotButton m_distSlot;
    SlotButton m_delaySlot;

    // --- Inline Drawer Controls ---
    // EQ Controls
    std::unique_ptr<RotaryKnob> m_eqHpfKnob;
    std::unique_ptr<RotaryKnob> m_eqLowMidGainKnob;
    std::unique_ptr<RotaryKnob> m_eqHighMidGainKnob;
    std::unique_ptr<RotaryKnob> m_eqAirGainKnob;
    std::unique_ptr<RotaryKnob> m_eqOutGainKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_eqAttachments;

    // Comp Controls
    std::unique_ptr<RotaryKnob> m_compThreshKnob;
    std::unique_ptr<RotaryKnob> m_compRatioKnob;
    std::unique_ptr<RotaryKnob> m_compAttackKnob;
    std::unique_ptr<RotaryKnob> m_compReleaseKnob;
    std::unique_ptr<RotaryKnob> m_compMakeupKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_compAttachments;

    // Flanger Controls
    std::unique_ptr<RotaryKnob> m_flangerRateKnob;
    std::unique_ptr<RotaryKnob> m_flangerDepthKnob;
    std::unique_ptr<RotaryKnob> m_flangerFeedbackKnob;
    std::unique_ptr<RotaryKnob> m_flangerMixKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_flangerAttachments;

    // Chorus / Haas Controls
    juce::ComboBox m_chorusModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_chorusModeAttachment;
    std::unique_ptr<RotaryKnob> m_chorusRateKnob;
    std::unique_ptr<RotaryKnob> m_chorusDepthKnob;
    std::unique_ptr<RotaryKnob> m_haasDelayKnob;
    std::unique_ptr<RotaryKnob> m_haasWidthKnob;
    std::unique_ptr<RotaryKnob> m_chorusMixKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_chorusAttachments;

    // Distortion Controls
    juce::ComboBox m_distModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_distModeAttachment;
    std::unique_ptr<RotaryKnob> m_distDriveKnob;
    std::unique_ptr<RotaryKnob> m_distToneKnob;
    std::unique_ptr<RotaryKnob> m_distMixKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_distAttachments;

    // Delay / Echo Controls (H-Delay, Supermassive, FreqEcho)
    juce::ComboBox m_delayModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_delayModeAttachment;
    std::unique_ptr<RotaryKnob> m_delayTimeKnob;
    std::unique_ptr<RotaryKnob> m_delayFeedbackKnob;
    std::unique_ptr<RotaryKnob> m_delayFreqShiftKnob;
    std::unique_ptr<RotaryKnob> m_delayDiffusionKnob;
    std::unique_ptr<RotaryKnob> m_delayToneKnob;
    std::unique_ptr<RotaryKnob> m_delayMixKnob;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_delayAttachments;

    void setupSlot(SlotButton& slot, const juce::String& name, const char* paramID, int slotIndex);
    void setActiveDrawer(int index);
    void updateChorusControlsVisibility();

    void paintEqCurve(juce::Graphics& g, juce::Rectangle<float> area);
    void paintCompMeter(juce::Graphics& g, juce::Rectangle<float> area);
};
