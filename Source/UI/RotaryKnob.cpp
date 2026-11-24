#include "RotaryKnob.h"

RotaryKnob::RotaryKnob(juce::RangedAudioParameter& /*param*/, const juce::String& title, const juce::String& unit)
    : m_title(title), m_unit(unit) {
    m_slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    m_slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    m_slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                 juce::MathConstants<float>::pi * 2.8f, true);
    m_slider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(m_slider);

    m_titleLabel.setText(title.toUpperCase(), juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(10.0f));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_titleLabel);

    m_valueLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));
    m_valueLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextWhite);
    m_valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_valueLabel);

    m_slider.onValueChange = [this]() {
        updateValueDisplay();
    };
    updateValueDisplay();
}

void RotaryKnob::resized() {
    auto bounds = getLocalBounds();
    m_titleLabel.setBounds(bounds.removeFromTop(16));
    m_valueLabel.setBounds(bounds.removeFromBottom(16));
    m_slider.setBounds(bounds.reduced(2));
}

void RotaryKnob::paint(juce::Graphics& /*g*/) {
}

void RotaryKnob::updateValueDisplay() {
    float val = static_cast<float>(m_slider.getValue());
    juce::String text;
    if (std::abs(val) < 10.0f && std::floor(val) != val) {
        text = juce::String(val, 1);
    } else {
        text = juce::String(static_cast<int>(std::round(val)));
    }
    if (m_unit.isNotEmpty()) {
        text += " " + m_unit;
    }
    m_valueLabel.setText(text, juce::dontSendNotification);
}
