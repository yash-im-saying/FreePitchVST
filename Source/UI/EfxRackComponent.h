#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "FreePitchLookAndFeel.h"
#include "RotaryKnob.h"
#include "DSP/EFXRack.h"

class EfxCardComponent : public juce::Component {
public:
    EfxCardComponent(const juce::String& title, const char* enabledParamID,
                     juce::AudioProcessorValueTreeState& apvts);
    ~EfxCardComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    juce::ToggleButton& getPowerToggle() { return m_powerToggle; }
    void addControl(std::unique_ptr<RotaryKnob> knob);
    void addComboBox(std::unique_ptr<juce::ComboBox> box);

private:
    juce::String m_title;
    juce::ToggleButton m_powerToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_powerAttachment;

    std::vector<std::unique_ptr<RotaryKnob>> m_knobs;
    std::vector<std::unique_ptr<juce::ComboBox>> m_boxes;

    juce::Image m_cachedCardBg;
    void renderCachedCardBg();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EfxCardComponent)
};

class EfxRackComponent : public juce::Component {
public:
    EfxRackComponent(juce::AudioProcessorValueTreeState& apvts, EFXRack& efxRack);
    ~EfxRackComponent() override = default;

    void updateTelemetry(const PitchTelemetry& telemetry);
    void updateThemeColors();
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& m_apvts;
    EFXRack& m_efxRack;
    PitchTelemetry m_telemetry;

    // 6 Modular Cards
    std::unique_ptr<EfxCardComponent> m_eqCard;
    std::unique_ptr<EfxCardComponent> m_compCard;
    std::unique_ptr<EfxCardComponent> m_chorusCard;
    std::unique_ptr<EfxCardComponent> m_flangerCard;
    std::unique_ptr<EfxCardComponent> m_distCard;
    std::unique_ptr<EfxCardComponent> m_delayCard;

    // Attachments storage
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> m_sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> m_comboAttachments;

    void setupEqCard();
    void setupCompCard();
    void setupChorusCard();
    void setupFlangerCard();
    void setupDistCard();
    void setupDelayCard();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EfxRackComponent)
};

using EFXDrawerComponent = EfxRackComponent;
