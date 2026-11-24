#include "FreePitchLookAndFeel.h"
#include <cmath>

FreePitchLookAndFeel::FreePitchLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, kBackgroundDark);
    setColour(juce::PopupMenu::backgroundColourId, kPanelGraphite);
    setColour(juce::PopupMenu::textColourId, kTextWhite);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, kPanelBorderLight);
    setColour(juce::PopupMenu::highlightedTextColourId, kAccentAmber);
    setColour(juce::TextButton::textColourOffId, kTextWhite);
    setColour(juce::TextButton::textColourOnId, kAccentAmber);
}

void FreePitchLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(3.0f);
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float rx = centreX - radius;
    float ry = centreY - radius;
    float rw = radius * 2.0f;

    // 1. Background Track Arc (Hairline)
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, radius - 3.0f, radius - 3.0f,
                        0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(kPanelBorder);
    g.strokePath(bgArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active Value Track Arc (Precision Amber)
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPosProportional > 0.001f) {
        juce::Path valArc;
        valArc.addCentredArc(centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, currentAngle, true);
        g.setColour(slider.isEnabled() ? kAccentAmber : kTextDim);
        g.strokePath(valArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Dial Body (Matte Graphite Disc with subtle 1px border)
    float dialRadius = radius - 8.0f;
    if (dialRadius > 4.0f) {
        juce::Rectangle<float> dialBounds(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);
        g.setColour(kPanelGraphite);
        g.fillEllipse(dialBounds);

        g.setColour(slider.isMouseOverOrDragging() ? kPanelBorderLight : kPanelBorder);
        g.drawEllipse(dialBounds, 1.0f);

        // 4. Indicator Needle / Pip
        juce::Path needle;
        float needleLength = dialRadius * 0.65f;
        needle.startNewSubPath(centreX + std::sin(currentAngle) * (dialRadius * 0.25f),
                               centreY - std::cos(currentAngle) * (dialRadius * 0.25f));
        needle.lineTo(centreX + std::sin(currentAngle) * needleLength,
                      centreY - std::cos(currentAngle) * needleLength);

        g.setColour(slider.isEnabled() ? (slider.isMouseOverOrDragging() ? kTextWhite : kAccentAmber) : kTextDim);
        g.strokePath(needle, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
    }
}

void FreePitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    // Flat graphite background
    g.setColour(kPanelGraphite);
    g.fillRoundedRectangle(bounds, 3.0f);

    // 1px hairline border
    g.setColour(box.isMouseOver() ? kPanelBorderLight : kPanelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    // Minimal chevron arrow on right
    float arrowX = width - 14.0f;
    float arrowY = height * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 3.5f, arrowY - 2.0f);
    arrow.lineTo(arrowX, arrowY + 2.0f);
    arrow.lineTo(arrowX + 3.5f, arrowY - 2.0f);

    g.setColour(box.isEnabled() ? kTextMuted : kTextDim);
    g.strokePath(arrow, juce::PathStrokeType(1.2f));
}

void FreePitchLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();

    // Draw illuminated indicator pip on left
    float pipSize = 8.0f;
    float pipY = bounds.getCentreY() - pipSize * 0.5f;
    juce::Rectangle<float> pipBounds(4.0f, pipY, pipSize, pipSize);

    if (isOn) {
        g.setColour(kAccentAmber);
        g.fillEllipse(pipBounds);
    } else {
        g.setColour(kPanelBorder);
        g.fillEllipse(pipBounds);
    }

    g.setColour(kPanelBorderLight);
    g.drawEllipse(pipBounds, 1.0f);

    // Text label
    g.setColour(isOn ? kTextWhite : (shouldDrawButtonAsHighlighted ? kTextMuted : kTextDim));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText(button.getButtonText(), 18, 0, button.getWidth() - 18, button.getHeight(),
               juce::Justification::centredLeft, true);
}

void FreePitchLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& /*backgroundColour*/,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

    bool isOn = button.getToggleState();
    juce::Colour bg = kPanelGraphite;
    if (isOn) {
        bg = kPanelBorderLight;
    } else if (shouldDrawButtonAsDown) {
        bg = kPanelBorder;
    } else if (shouldDrawButtonAsHighlighted) {
        bg = juce::Colour(0xFF24282F);
    }

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(isOn ? kAccentAmber : (shouldDrawButtonAsHighlighted ? kPanelBorderLight : kPanelBorder));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

juce::Font FreePitchLookAndFeel::getLabelFont(juce::Label&) {
    return juce::FontOptions(11.0f);
}

juce::Font FreePitchLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::FontOptions(11.5f);
}
