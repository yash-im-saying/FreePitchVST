#include "RotaryKnob.h"
#include <cmath>

RotaryKnob::RotaryKnob(juce::RangedAudioParameter& param, const juce::String& title, const juce::String& unit)
    : m_title(title), m_unit(unit) {

    m_slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    m_slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    m_slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                 juce::MathConstants<float>::pi * 2.75f, true);

    float defaultVal = param.convertFrom0to1(param.getDefaultValue());
    m_slider.setDoubleClickReturnValue(true, static_cast<double>(defaultVal));
    addAndMakeVisible(m_slider);

    m_titleLabel.setText(title.toUpperCase(), juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchColors::textSecondary);
    m_titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_titleLabel);

    m_valueLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
    m_valueLabel.setColour(juce::Label::textColourId, FreePitchColors::textPrimary);
    m_valueLabel.setJustificationType(juce::Justification::centred);
    addChildComponent(m_valueLabel);

    m_slider.onValueChange = [this]() {
        updateValueDisplay();
    };
    updateValueDisplay();
}

void RotaryKnob::setHero(bool isHero) {
    m_isHero = isHero;
    m_titleLabel.setFont(juce::FontOptions(isHero ? 11.5f : 10.0f, juce::Font::bold));
    resized();
}

void RotaryKnob::setMini(bool isMini) {
    m_isMini = isMini;
    m_titleLabel.setFont(juce::FontOptions(isMini ? 8.5f : 10.0f, juce::Font::bold));
    resized();
}

void RotaryKnob::setAccentAmber(bool /*isAmber*/) {
    // Kept for backward compatibility; standard design uses strict signal-red
    m_slider.repaint();
}

void RotaryKnob::setTooltipText(const juce::String& text) {
    setTooltip(text);
    m_slider.setTooltip(text);
    m_titleLabel.setTooltip(text);
    m_valueLabel.setTooltip(text);
}

void RotaryKnob::updateThemeColors() {
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchColors::textSecondary);
    m_valueLabel.setColour(juce::Label::textColourId, FreePitchColors::textPrimary);
    renderCachedBackground();
    m_slider.repaint();
    repaint();
}

void RotaryKnob::resized() {
    auto bounds = getLocalBounds();
    if (bounds.isEmpty()) return;

    int titleHeight = m_isMini ? 12 : (m_isHero ? 16 : 14);
    int spacing = m_isMini ? 2 : 4;

    int availH = bounds.getHeight() - (titleHeight + spacing);
    int availW = bounds.getWidth() - (m_isMini ? 2 : 4);
    int dialSize = std::max(18, std::min(availW, availH));

    int totalH = dialSize + spacing + titleHeight;
    int startY = std::max(0, (bounds.getHeight() - totalH) / 2);
    int startX = (bounds.getWidth() - dialSize) / 2;

    m_slider.setBounds(startX, startY, dialSize, dialSize);
    m_titleLabel.setBounds(0, startY + dialSize + spacing, bounds.getWidth(), titleHeight);

    renderCachedBackground();
}

void RotaryKnob::renderCachedBackground() {
    int w = getWidth();
    int h = getHeight();
    if (w <= 0 || h <= 0) return;

    m_cachedBackground = juce::Image(juce::Image::ARGB, w, h, true);
    juce::Graphics g(m_cachedBackground);

    // Pre-render the static socket well behind the dial to eliminate redraw cost
    auto sliderBounds = m_slider.getBounds().toFloat().reduced(1.0f);
    if (!sliderBounds.isEmpty()) {
        float radius = std::min(sliderBounds.getWidth(), sliderBounds.getHeight()) * 0.5f;
        juce::Rectangle<float> socketBounds(sliderBounds.getCentreX() - radius, sliderBounds.getCentreY() - radius,
                                            radius * 2.0f, radius * 2.0f);
        FreePitchLookAndFeel::drawNeumorphicSunken(g, socketBounds, radius, m_isMini ? 1.5f : 2.5f);
    }
}

void RotaryKnob::paint(juce::Graphics& g) {
    if (m_cachedBackground.isValid()) {
        g.drawImageAt(m_cachedBackground, 0, 0);
    }
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
