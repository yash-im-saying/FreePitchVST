#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FreePitchLookAndFeel.h"

class RotaryKnob : public juce::Component,
                   public juce::SettableTooltipClient {
public:
    RotaryKnob(juce::RangedAudioParameter& param, const juce::String& title, const juce::String& unit = "");
    ~RotaryKnob() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getSlider() { return m_slider; }

    void setAccentAmber(bool isAmber);
    void setHero(bool isHero);
    void setTooltipText(const juce::String& text);
    void updateThemeColors();

private:
    juce::Slider m_slider;
    juce::String m_title;
    juce::String m_unit;
    juce::Label m_titleLabel;
    juce::Label m_valueLabel;
    bool m_isHero = false;

    void updateValueDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
