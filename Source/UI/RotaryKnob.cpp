#include "RotaryKnob.h"
#include <cmath>

RotaryKnob::RotaryKnob(juce::RangedAudioParameter& param, const juce::String& title, const juce::String& unit)
    : m_title(title), m_unit(unit) {
    m_slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    m_slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    m_slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                 juce::MathConstants<float>::pi * 2.75f, true);
    
    // Set double-click to default parameter value
    float defaultVal = param.convertFrom0to1(param.getDefaultValue());
    m_slider.setDoubleClickReturnValue(true, static_cast<double>(defaultVal));
    addAndMakeVisible(m_slider);

    m_titleLabel.setText(title.toUpperCase(), juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_titleLabel);

    m_valueLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::bold));
    m_valueLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextWhite);
    m_valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_valueLabel);

    m_slider.onValueChange = [this]() {
        updateValueDisplay();
    };
    updateValueDisplay();
}

void RotaryKnob::setAccentAmber(bool isAmber) {
    m_slider.getProperties().set("accentAmber", isAmber);
    m_slider.repaint();
}

void RotaryKnob::setHero(bool isHero) {
    m_isHero = isHero;
    m_titleLabel.setFont(juce::FontOptions(isHero ? 11.5f : 10.5f, juce::Font::bold));
    m_valueLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), isHero ? 13.0f : 11.5f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, isHero ? FreePitchLookAndFeel::kTextLight : FreePitchLookAndFeel::kTextMuted);
    resized();
}

void RotaryKnob::setTooltipText(const juce::String& text) {
    setTooltip(text);
    m_slider.setTooltip(text);
    m_titleLabel.setTooltip(text);
    m_valueLabel.setTooltip(text);
}

void RotaryKnob::resized() {
    auto bounds = getLocalBounds();
    int titleHeight = m_isHero ? 20 : 16;
    int valueHeight = m_isHero ? 22 : 18;

    m_titleLabel.setBounds(bounds.removeFromTop(titleHeight));
    m_valueLabel.setBounds(bounds.removeFromBottom(valueHeight));
    m_slider.setBounds(bounds.reduced(m_isHero ? 2 : 1));
}

void RotaryKnob::paint(juce::Graphics& g) {
    // Subtle value badge container on bottom
    auto valArea = m_valueLabel.getBounds().toFloat().reduced(2.0f, 1.0f);
    g.setColour(FreePitchLookAndFeel::kPanelDark);
    g.fillRoundedRectangle(valArea, 3.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(valArea, 3.0f, 0.8f);
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
