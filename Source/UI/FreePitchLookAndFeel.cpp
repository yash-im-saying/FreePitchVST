#include "FreePitchLookAndFeel.h"
#include <cmath>

FreePitchLookAndFeel::FreePitchLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, kBackgroundDark);
    setColour(juce::PopupMenu::backgroundColourId, kPanelCard);
    setColour(juce::PopupMenu::textColourId, kTextWhite);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, kPanelCardHover);
    setColour(juce::PopupMenu::highlightedTextColourId, kAccentCyan);
    setColour(juce::TextButton::textColourOffId, kTextLight);
    setColour(juce::TextButton::textColourOnId, kAccentAmber);
    setColour(juce::TooltipWindow::backgroundColourId, kBackgroundDark);
    setColour(juce::TooltipWindow::textColourId, kTextWhite);
    setColour(juce::TooltipWindow::outlineColourId, kPanelBorderLight);
}

void FreePitchLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();

    bool isHovered = slider.isMouseOverOrDragging();
    bool isEnabled = slider.isEnabled();

    // Check if slider is configured for Amber or Cyan accent via component property
    bool useAmber = slider.getProperties().contains("accentAmber") && static_cast<bool>(slider.getProperties()["accentAmber"]);
    juce::Colour activeColor = useAmber ? kAccentAmber : kAccentCyan;
    juce::Colour activeGlow  = useAmber ? kAccentAmberGlow : kAccentCyanGlow;

    // 1. Outer Glow on Hover/Drag
    if (isHovered && isEnabled) {
        g.setColour(activeGlow);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    }

    // 2. Background Track Arc (Dark inset groove)
    float trackRadius = radius - 3.5f;
    if (trackRadius > 4.0f) {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(kPanelBorder);
        g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Active Illuminated Track Arc
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPosProportional > 0.001f && trackRadius > 4.0f) {
        juce::Path valArc;
        valArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, currentAngle, true);

        // Subtle glow under active track
        if (isEnabled) {
            g.setColour(activeColor.withAlpha(0.35f));
            g.strokePath(valArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(isEnabled ? activeColor : kTextDim);
        g.strokePath(valArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Dial Cap (Machined Metallic Disc)
    float dialRadius = radius - 9.0f;
    if (dialRadius > 6.0f) {
        juce::Rectangle<float> dialBounds(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

        // Soft drop shadow under knob
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillEllipse(dialBounds.translated(0.0f, 2.0f));

        // Machined metal linear gradient
        juce::ColourGradient dialGrad(kKnobBodyTop, centreX, dialBounds.getY(),
                                      kKnobBodyBottom, centreX, dialBounds.getBottom(), false);
        g.setGradientFill(dialGrad);
        g.fillEllipse(dialBounds);

        // Outer machined bevel rim
        g.setColour(isHovered ? activeColor.withAlpha(0.6f) : kKnobRim);
        g.drawEllipse(dialBounds, 1.2f);

        // Inner subtle concentric groove
        float innerGrooveRadius = dialRadius * 0.78f;
        g.setColour(kPanelDark.withAlpha(0.7f));
        g.drawEllipse(centreX - innerGrooveRadius, centreY - innerGrooveRadius,
                      innerGrooveRadius * 2.0f, innerGrooveRadius * 2.0f, 0.8f);

        // 5. Indicator Needle Pointer (Sleek tapered needle)
        juce::Path needle;
        float needleStart = dialRadius * 0.30f;
        float needleEnd   = dialRadius * 0.88f;

        float sinA = std::sin(currentAngle);
        float cosA = std::cos(currentAngle);

        needle.startNewSubPath(centreX + sinA * needleStart, centreY - cosA * needleStart);
        needle.lineTo(centreX + sinA * needleEnd,   centreY - cosA * needleEnd);

        if (isEnabled) {
            // Needle glow
            g.setColour(activeColor.withAlpha(0.5f));
            g.strokePath(needle, juce::PathStrokeType(3.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

            g.setColour(isHovered ? kTextWhite : activeColor);
            g.strokePath(needle, juce::PathStrokeType(2.2f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        } else {
            g.setColour(kTextDim);
            g.strokePath(needle, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }

        // Center jewel dot
        float dotRadius = 2.2f;
        g.setColour(isEnabled ? (isHovered ? kTextWhite : activeColor) : kTextDim);
        g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
}

void FreePitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    bool isHovered = box.isMouseOver() || isButtonDown;

    // Gradient background
    juce::ColourGradient bgGrad(isHovered ? kPanelCardHover : kPanelCard, 0.0f, 0.0f,
                                kPanelDark, 0.0f, static_cast<float>(height), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border with hover glow
    g.setColour(isHovered ? kAccentCyan.withAlpha(0.7f) : kPanelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Modern geometric chevron on right
    float arrowX = width - 15.0f;
    float arrowY = height * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 4.0f, arrowY - 2.5f);
    arrow.lineTo(arrowX, arrowY + 2.5f);
    arrow.lineTo(arrowX + 4.0f, arrowY - 2.5f);

    g.setColour(box.isEnabled() ? (isHovered ? kAccentCyan : kTextMuted) : kTextDim);
    g.strokePath(arrow, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}

void FreePitchLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    bool isEnabled = button.isEnabled();

    // Check if button has custom green / amber accent
    bool useAmber = button.getProperties().contains("accentAmber") && static_cast<bool>(button.getProperties()["accentAmber"]);
    juce::Colour activeColor = useAmber ? kAccentAmber : kIndicatorInTune;

    // Draw illuminated LED pip with glow
    float pipSize = 9.0f;
    float pipY = bounds.getCentreY() - pipSize * 0.5f;
    juce::Rectangle<float> pipBounds(4.0f, pipY, pipSize, pipSize);

    if (isOn && isEnabled) {
        // Glowing halo
        g.setColour(activeColor.withAlpha(0.35f));
        g.fillEllipse(pipBounds.expanded(3.0f));

        g.setColour(activeColor);
        g.fillEllipse(pipBounds);

        // Core bright highlight
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.fillEllipse(pipBounds.reduced(2.2f));
    } else {
        // Inset dark socket
        g.setColour(kPanelDark);
        g.fillEllipse(pipBounds);
        g.setColour(kPanelBorder);
        g.drawEllipse(pipBounds, 1.0f);
    }

    // Text Label
    g.setColour(isOn ? kTextWhite : (shouldDrawButtonAsHighlighted ? kTextLight : kTextMuted));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText(button.getButtonText(), 19, 0, button.getWidth() - 19, button.getHeight(),
               juce::Justification::centredLeft, true);
}

void FreePitchLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& /*backgroundColour*/,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();

    juce::Colour baseColor = shouldDrawButtonAsDown ? kPanelDark : (shouldDrawButtonAsHighlighted ? kPanelCardHover : kPanelCard);
    if (isOn) {
        baseColor = kPanelCardHover;
    }

    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 4.0f);

    juce::Colour borderColor = isOn ? kAccentCyan : (shouldDrawButtonAsHighlighted ? kPanelBorderLight : kPanelBorder);
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

void FreePitchLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(kPanelCard);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(kPanelBorderLight);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);
}

void FreePitchLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                             bool isSeparator, bool isActive, bool isHighlighted,
                                             bool isTicked, bool hasSubMenu, const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* icon, const juce::Colour* textColour) {
    juce::ignoreUnused(icon, shortcutKeyText, hasSubMenu);
    if (isSeparator) {
        g.setColour(kPanelBorder);
        g.fillRect(area.reduced(6, 0).withHeight(1));
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour(kPanelCardHover);
        g.fillRect(area.reduced(2, 1));

        // Cyan left accent pill
        g.setColour(kAccentCyan);
        g.fillRect(area.getX() + 2, area.getY() + 2, 3, area.getHeight() - 4);
    }

    juce::Colour textColor = isActive ? (isHighlighted ? kAccentCyan : kTextWhite) : kTextDim;
    if (textColour != nullptr) {
        textColor = *textColour;
    }

    g.setColour(textColor);
    g.setFont(juce::FontOptions(12.5f));

    int leftMargin = isTicked ? 24 : 14;
    g.drawText(text, area.getX() + leftMargin, area.getY(), area.getWidth() - leftMargin - 10, area.getHeight(),
               juce::Justification::centredLeft, true);

    if (isTicked) {
        g.setColour(kAccentCyan);
        juce::Path tick;
        float tx = static_cast<float>(area.getX()) + 9.0f;
        float ty = static_cast<float>(area.getCentreY());
        tick.startNewSubPath(tx - 3.0f, ty);
        tick.lineTo(tx - 1.0f, ty + 3.0f);
        tick.lineTo(tx + 4.0f, ty - 3.0f);
        g.strokePath(tick, juce::PathStrokeType(1.8f));
    }
}

void FreePitchLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    // Studio dark glass with subtle shadow
    g.setColour(kBackgroundDark.withAlpha(0.96f));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Neon cyan hairline border with soft glow
    g.setColour(kAccentCyan.withAlpha(0.55f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Crisp typography
    g.setColour(kTextWhite);
    g.setFont(juce::FontOptions(11.5f));
    g.drawFittedText(text, bounds.reduced(8.0f, 6.0f).toNearestInt(), juce::Justification::centredLeft, 6);
}

juce::Rectangle<int> FreePitchLookAndFeel::getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos,
                                                            juce::Rectangle<int> parentArea) {
    juce::Font font(juce::FontOptions(11.5f));
    int textWidth = font.getStringWidth(tipText);

    int maxWidth = std::min(320, parentArea.getWidth() - 20);
    int width = std::min(maxWidth, std::max(140, textWidth + 24));

    // Approximate height based on line wrap
    int numLines = (textWidth / (width - 16)) + 1;
    int height = std::max(28, numLines * 16 + 14);

    int x = screenPos.x + 12;
    int y = screenPos.y + 16;

    if (x + width > parentArea.getRight())  x = parentArea.getRight() - width - 8;
    if (y + height > parentArea.getBottom()) y = screenPos.y - height - 8;
    if (x < parentArea.getX()) x = parentArea.getX() + 8;
    if (y < parentArea.getY()) y = parentArea.getY() + 8;

    return { x, y, width, height };
}

juce::Font FreePitchLookAndFeel::getLabelFont(juce::Label&) {
    return juce::Font(juce::FontOptions(11.5f));
}

juce::Font FreePitchLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(juce::FontOptions(12.0f, juce::Font::bold));
}
