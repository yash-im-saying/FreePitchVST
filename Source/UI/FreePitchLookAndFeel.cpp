#include "FreePitchLookAndFeel.h"
#include <cmath>

FreePitchLookAndFeel::FreePitchLookAndFeel() {
    setThemeMode(ThemeMode::Dark);
}

void FreePitchLookAndFeel::setThemeMode(ThemeMode mode) {
    m_themeMode = mode;

    setColour(juce::ResizableWindow::backgroundColourId, FreePitchColors::bgChassis);
    setColour(juce::PopupMenu::backgroundColourId, FreePitchColors::surfaceElevated);
    setColour(juce::PopupMenu::textColourId, FreePitchColors::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, FreePitchColors::cavityRecessed);
    setColour(juce::PopupMenu::highlightedTextColourId, FreePitchColors::signalRed);
    setColour(juce::TextButton::textColourOffId, FreePitchColors::textSecondary);
    setColour(juce::TextButton::textColourOnId, FreePitchColors::textPrimary);
    setColour(juce::TooltipWindow::backgroundColourId, FreePitchColors::cavityRecessed);
    setColour(juce::TooltipWindow::textColourId, FreePitchColors::textPrimary);
    setColour(juce::TooltipWindow::outlineColourId, FreePitchColors::rimHighlight);
}

void FreePitchLookAndFeel::drawNeumorphicRaised(juce::Graphics& g, juce::Rectangle<float> bounds,
                                                float cornerRadius, float depth,
                                                bool isHovered, bool isPressed) {
    using namespace FreePitchColors;

    if (isPressed) {
        drawNeumorphicSunken(g, bounds, cornerRadius, depth * 0.75f);
        return;
    }

    float off = depth * 0.75f;

    // 1. Dual opposing shadows
    // Top-left subtle cool rim highlight (-offset)
    g.setColour(rimHighlight.withAlpha(0.45f));
    g.fillRoundedRectangle(bounds.translated(-off * 0.75f, -off * 0.75f), cornerRadius);

    // Bottom-right deep drop shadow (+offset)
    g.setColour(deepShadow.withAlpha(0.90f));
    g.fillRoundedRectangle(bounds.translated(off, off), cornerRadius);

    // 2. Tactile extruded surface face (Top-left-to-bottom-right gradient: #22262D -> #191C21)
    juce::Colour topCol = isHovered ? surfaceGradTop.brighter(0.04f) : surfaceGradTop;
    juce::Colour botCol = isHovered ? surfaceGradBot.brighter(0.03f) : surfaceGradBot;
    juce::ColourGradient grad(topCol, bounds.getX(), bounds.getY(),
                              botCol, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // 3. Subtle 3D chamfer rim hairline (rimHighlight at top-left, deepShadow at bottom-right)
    juce::ColourGradient rimGrad(rimHighlight.withAlpha(0.70f), bounds.getX(), bounds.getY(),
                                 deepShadow.withAlpha(0.85f), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(rimGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void FreePitchLookAndFeel::drawNeumorphicSunken(juce::Graphics& g, juce::Rectangle<float> bounds,
                                                float cornerRadius, float depth) {
    using namespace FreePitchColors;

    // 1. Recessed cavity fill (#101215)
    g.setColour(cavityRecessed);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // 2. Inner shadow bevel: #08090B top-left inner stroke
    juce::ColourGradient darkGrad(innerCavityDark, bounds.getX(), bounds.getY(),
                                  cavityRecessed, bounds.getX() + depth * 2.0f, bounds.getY() + depth * 2.0f, false);
    g.setGradientFill(darkGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, depth);

    // 3. Inner shadow bevel: 1px #262A33 bottom-right inner rim
    juce::ColourGradient lightGrad(cavityRecessed, bounds.getRight() - depth * 1.5f, bounds.getBottom() - depth * 1.5f,
                                   innerCavityRim, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(lightGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void FreePitchLookAndFeel::drawTactileLed(juce::Graphics& g, float cx, float cy, float radius,
                                          const juce::Colour& color, bool isOn) {
    using namespace FreePitchColors;
    juce::Rectangle<float> ledBounds(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    if (isOn) {
        // Soft outer bloom (0x44FF2E3B)
        g.setColour(color.withAlpha(0.28f));
        g.fillEllipse(ledBounds.expanded(radius * 1.2f));

        g.setColour(color.withAlpha(0.55f));
        g.fillEllipse(ledBounds.expanded(radius * 0.5f));

        // Core Signal Red LED (#FF2E3B)
        g.setColour(color);
        g.fillEllipse(ledBounds);

        // Crisp specular highlight reflection
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.fillEllipse(cx - radius * 0.35f, cy - radius * 0.35f, radius * 0.55f, radius * 0.55f);
    } else {
        // Inactive dark socket
        g.setColour(cavityRecessed);
        g.fillEllipse(ledBounds);
        g.setColour(rimHighlight.withAlpha(0.5f));
        g.drawEllipse(ledBounds, 0.8f);
    }
}

void FreePitchLookAndFeel::drawMachinedCap(juce::Graphics& g, juce::Rectangle<float> dialBounds,
                                           float currentAngle, bool isHovered, bool isPressed) {
    using namespace FreePitchColors;
    float radius = dialBounds.getWidth() * 0.5f;
    float centreX = dialBounds.getCentreX();
    float centreY = dialBounds.getCentreY();

    float off = isPressed ? 1.0f : 2.5f;

    // Dual opposing shadows for the circular knob cap
    // Top-left rim highlight
    g.setColour(rimHighlight.withAlpha(0.55f));
    g.fillEllipse(dialBounds.translated(-off * 0.8f, -off * 0.8f));

    // Bottom-right deep drop shadow
    g.setColour(deepShadow.withAlpha(0.95f));
    g.fillEllipse(dialBounds.translated(off * 1.2f, off * 1.2f));

    // Extruded machined circular cap surface (#22262D -> #191C21)
    juce::Colour topCol = isHovered ? surfaceGradTop.brighter(0.04f) : surfaceGradTop;
    juce::Colour botCol = isHovered ? surfaceGradBot.brighter(0.03f) : surfaceGradBot;
    juce::ColourGradient capGrad(topCol, centreX - radius, centreY - radius,
                                 botCol, centreX + radius, centreY + radius, false);
    g.setGradientFill(capGrad);
    g.fillEllipse(dialBounds);

    // Beveled rim hairline
    juce::ColourGradient rimGrad(rimHighlight, centreX - radius, centreY - radius,
                                 deepShadow, centreX + radius, centreY + radius, false);
    g.setGradientFill(rimGrad);
    g.drawEllipse(dialBounds, 1.0f);

    // Subtle concave center finger dimple
    float dimpleRadius = radius * 0.32f;
    juce::Rectangle<float> dimpleBounds(centreX - dimpleRadius, centreY - dimpleRadius,
                                        dimpleRadius * 2.0f, dimpleRadius * 2.0f);
    drawNeumorphicSunken(g, dimpleBounds, dimpleRadius, 1.2f);

    // Crisp white radial indicator line pointing along currentAngle
    float r1 = radius * 0.38f;
    float r2 = radius * 0.86f;
    float sinA = std::sin(currentAngle);
    float cosA = -std::cos(currentAngle);

    float x1 = centreX + sinA * r1;
    float y1 = centreY + cosA * r1;
    float x2 = centreX + sinA * r2;
    float y2 = centreY + cosA * r2;

    float strokeW = std::clamp(radius * 0.10f, 1.6f, 2.5f);
    g.setColour(textPrimary);
    g.drawLine(x1, y1, x2, y2, strokeW);
}

void FreePitchLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider) {
    using namespace FreePitchColors;
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();

    bool isHovered = slider.isMouseOverOrDragging();
    bool isDown = slider.isMouseButtonDown();

    // 1. Recessed dark outer track well (#0D0F12)
    juce::Rectangle<float> socketBounds(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    drawNeumorphicSunken(g, socketBounds, radius, 2.2f);

    // 2. Background track groove
    float trackRadius = radius - 3.0f;
    if (trackRadius > 4.0f) {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(outerKnobTrack);
        g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Glowing #FF2E3B Value Arc with soft bloom (0x44FF2E3B)
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPosProportional > 0.002f && trackRadius > 4.0f) {
        juce::Path valArc;
        valArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, currentAngle, true);

        // Soft outer bloom
        g.setColour(signalRedBloom);
        g.strokePath(valArc, juce::PathStrokeType(6.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Sharp value arc
        g.setColour(signalRed);
        g.strokePath(valArc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Machined Circular Cap
    float capRadius = radius - 7.5f;
    if (capRadius > 4.0f) {
        juce::Rectangle<float> capBounds(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        drawMachinedCap(g, capBounds, currentAngle, isHovered, isDown);
    }
}

void FreePitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box) {
    using namespace FreePitchColors;
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    bool isHovered = box.isMouseOver() || isButtonDown;

    // Recessed well for dropdown pill (#101215)
    drawNeumorphicSunken(g, bounds, bounds.getHeight() * 0.5f, 2.0f);

    if (isHovered) {
        g.setColour(rimHighlight.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), bounds.getHeight() * 0.5f, 1.0f);
    }

    // Modern down-chevron arrow on right
    float arrowX = static_cast<float>(width) - 15.0f;
    float arrowY = static_cast<float>(height) * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 3.5f, arrowY - 2.0f);
    arrow.lineTo(arrowX, arrowY + 2.0f);
    arrow.lineTo(arrowX + 3.5f, arrowY - 2.0f);

    g.setColour(isHovered ? textPrimary : textSecondary);
    g.strokePath(arrow, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}

void FreePitchLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) {
    using namespace FreePitchColors;
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();

    // Tactile LED dot
    float pipSize = 8.0f;
    float pipY = bounds.getCentreY();
    drawTactileLed(g, 9.0f, pipY, pipSize * 0.5f, signalRed, isOn);

    // Tracked uppercase label
    g.setColour(isOn ? textPrimary : (shouldDrawButtonAsHighlighted ? textSecondary : textDim));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(button.getButtonText().toUpperCase(), 18, 0, button.getWidth() - 18, button.getHeight(),
               juce::Justification::centredLeft, true);
}

void FreePitchLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& /*backgroundColour*/,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    using namespace FreePitchColors;
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    bool isPressed = shouldDrawButtonAsDown || isOn;

    if (isPressed) {
        drawNeumorphicSunken(g, bounds, 5.0f, 2.5f);
        if (isOn) {
            g.setColour(signalRedBloom);
            g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);
        }
    } else {
        drawNeumorphicRaised(g, bounds, 5.0f, 2.5f, shouldDrawButtonAsHighlighted);
    }
}

void FreePitchLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    drawNeumorphicRaised(g, bounds, 6.0f, 4.0f);
}

void FreePitchLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                             bool isSeparator, bool isActive, bool isHighlighted,
                                             bool isTicked, bool hasSubMenu, const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* icon, const juce::Colour* textColour) {
    juce::ignoreUnused(icon, shortcutKeyText, hasSubMenu);
    using namespace FreePitchColors;

    if (isSeparator) {
        g.setColour(rimHighlight.withAlpha(0.6f));
        g.fillRect(area.reduced(6, 0).withHeight(1));
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour(cavityRecessed);
        g.fillRect(area.reduced(2, 1));

        // Signal red left indicator pip
        g.setColour(signalRed);
        g.fillRect(area.getX() + 2, area.getY() + 3, 3, area.getHeight() - 6);
    }

    juce::Colour textColor = isActive ? (isHighlighted ? signalRed : textPrimary) : textDim;
    if (textColour != nullptr) {
        textColor = *textColour;
    }

    g.setColour(textColor);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));

    int leftMargin = isTicked ? 24 : 14;
    g.drawText(text, area.getX() + leftMargin, area.getY(), area.getWidth() - leftMargin - 10, area.getHeight(),
               juce::Justification::centredLeft, true);

    if (isTicked) {
        g.setColour(signalRed);
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
    using namespace FreePitchColors;
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(cavityRecessed.withAlpha(0.97f));
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(rimHighlight);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);

    g.setColour(textPrimary);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawFittedText(text, bounds.reduced(8.0f, 5.0f).toNearestInt(), juce::Justification::centredLeft, 4);
}

juce::Rectangle<int> FreePitchLookAndFeel::getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos,
                                                            juce::Rectangle<int> parentArea) {
    juce::Font font(juce::FontOptions(11.0f, juce::Font::bold));
    juce::GlyphArrangement ga;
    ga.addLineOfText(font, tipText, 0.0f, 0.0f);
    int textWidth = static_cast<int>(std::ceil(ga.getBoundingBox(0, -1, false).getWidth()));

    int maxWidth = std::min(300, parentArea.getWidth() - 20);
    int width = std::min(maxWidth, std::max(120, textWidth + 20));

    int numLines = (textWidth / (width - 16)) + 1;
    int height = std::max(26, numLines * 15 + 12);

    int x = screenPos.x + 12;
    int y = screenPos.y + 16;

    if (x + width > parentArea.getRight())  x = parentArea.getRight() - width - 8;
    if (y + height > parentArea.getBottom()) y = screenPos.y - height - 8;
    if (x < parentArea.getX()) x = parentArea.getX() + 8;
    if (y < parentArea.getY()) y = parentArea.getY() + 8;

    return { x, y, width, height };
}

juce::Font FreePitchLookAndFeel::getLabelFont(juce::Label&) {
    return juce::Font(juce::FontOptions(11.0f, juce::Font::bold));
}

juce::Font FreePitchLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(juce::FontOptions(12.0f, juce::Font::bold));
}
