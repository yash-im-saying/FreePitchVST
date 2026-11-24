#include "FreePitchLookAndFeel.h"
#include <cmath>

FreePitchLookAndFeel::ThemePalette FreePitchLookAndFeel::makeDarkPalette() {
    ThemePalette p;
    p.bgBase            = juce::Colour(0xFF181B20); // Neutral dark slate chassis
    p.panelRaised       = juce::Colour(0xFF1E2229); // Extruded neumorphic card surface
    p.panelSunken       = juce::Colour(0xFF131519); // Concave dish / inset well
    p.panelBorder       = juce::Colour(0xFF242A34); // Soft perimeter hairline
    p.panelBorderLight  = juce::Colour(0xFF2E3542); // Soft top-left specular hairline
    p.shadowDark        = juce::Colour(0x990A0C0E); // Diffuse ambient drop shadow
    p.shadowLight       = juce::Colour(0x2E404D60); // Soft ambient highlight shadow
    p.innerShadowDark   = juce::Colour(0xCC08090C); // Recessed cavity dark shadow
    p.innerShadowLight  = juce::Colour(0x26374456); // Inset cavity specular rim

    p.textPrimary       = juce::Colour(0xFFF8FAFC); // Crisp off-white
    p.textSecondary     = juce::Colour(0xFF94A3B8); // Clean slate
    p.textMuted         = juce::Colour(0xFF64748B); // Technical grey
    p.textDim           = juce::Colour(0xFF475569); // Muted inactive

    p.accentCyan        = juce::Colour(0xFF00E5FF); // Radiant electric cyan
    p.accentCyanGlow    = juce::Colour(0x4000E5FF);
    p.accentAmber       = juce::Colour(0xFFFFB300); // Warm studio amber gold
    p.accentAmberGlow   = juce::Colour(0x40FFB300);
    p.indicatorInTune   = juce::Colour(0xFF00E676); // Emerald phosphor LED
    p.indicatorSharp    = juce::Colour(0xFFFB923C);
    p.indicatorFlat     = juce::Colour(0xFF38BDF8);
    p.indicatorWarning  = juce::Colour(0xFFEF4444);

    return p;
}

FreePitchLookAndFeel::ThemePalette FreePitchLookAndFeel::makeLightPalette() {
    ThemePalette p;
    p.bgBase            = juce::Colour(0xFFE0E5EC); // Iconic uiverse neumorphic silver cream
    p.panelRaised       = juce::Colour(0xFFE0E5EC); // Same matte material!
    p.panelSunken       = juce::Colour(0xFFD2D8E2); // Concave dish / inset well
    p.panelBorder       = juce::Colour(0xFFBAC3CF); // Soft perimeter hairline
    p.panelBorderLight  = juce::Colour(0xFFFFFFFF); // Specular top-left hairline
    p.shadowDark        = juce::Colour(0x77A3B1C6); // Soft ambient drop shadow
    p.shadowLight       = juce::Colour(0xEEFFFFFF); // Pure white specular highlight shadow
    p.innerShadowDark   = juce::Colour(0x6697A6BB); // Recessed cavity dark shadow
    p.innerShadowLight  = juce::Colour(0xEEFFFFFF); // Inset cavity specular rim

    p.textPrimary       = juce::Colour(0xFF1E293B); // Deep charcoal
    p.textSecondary     = juce::Colour(0xFF475569); // Slate
    p.textMuted         = juce::Colour(0xFF64748B); // Muted neutral
    p.textDim           = juce::Colour(0xFF94A3B8); // Soft grey

    p.accentCyan        = juce::Colour(0xFF0284C7); // Vivid sapphire cyan
    p.accentCyanGlow    = juce::Colour(0x330284C7);
    p.accentAmber       = juce::Colour(0xFFD97706); // Warm studio amber bronze
    p.accentAmberGlow   = juce::Colour(0x33D97706);
    p.indicatorInTune   = juce::Colour(0xFF059669); // Studio emerald
    p.indicatorSharp    = juce::Colour(0xFFEA580C);
    p.indicatorFlat     = juce::Colour(0xFF0284C7);
    p.indicatorWarning  = juce::Colour(0xFFDC2626);

    return p;
}

FreePitchLookAndFeel::FreePitchLookAndFeel() {
    setThemeMode(ThemeMode::Dark);
}

void FreePitchLookAndFeel::setThemeMode(ThemeMode mode) {
    m_themeMode = mode;
    s_themeMode = mode;
    s_currentPalette = (mode == ThemeMode::Dark) ? makeDarkPalette() : makeLightPalette();
    applyPaletteToColours();
}

void FreePitchLookAndFeel::applyPaletteToColours() {
    const auto& p = s_currentPalette;

    // Update static backward-compatible aliases
    kBackgroundDark   = p.bgBase;
    kPanelDark        = p.panelSunken;
    kPanelCard        = p.panelRaised;
    kPanelCardHover   = (s_themeMode == ThemeMode::Dark) ? p.panelRaised.brighter(0.05f) : p.panelRaised.darker(0.03f);
    kPanelBorder      = p.panelBorder;
    kPanelBorderLight = p.panelBorderLight;
    kPanelBorderGlow  = p.accentCyanGlow;

    kTextWhite        = p.textPrimary;
    kTextLight        = p.textSecondary;
    kTextMuted        = p.textMuted;
    kTextDim          = p.textDim;

    kAccentCyan       = p.accentCyan;
    kAccentCyanGlow   = p.accentCyanGlow;
    kAccentAmber      = p.accentAmber;
    kAccentAmberGlow  = p.accentAmberGlow;
    kIndicatorInTune  = p.indicatorInTune;
    kIndicatorSharp   = p.indicatorSharp;
    kIndicatorFlat    = p.indicatorFlat;
    kIndicatorWarning = p.indicatorWarning;

    kKnobBodyTop      = p.panelRaised;
    kKnobBodyBottom   = p.panelSunken;
    kKnobRim          = p.panelBorder;

    // JUCE Component standard colours
    setColour(juce::ResizableWindow::backgroundColourId, p.bgBase);
    setColour(juce::PopupMenu::backgroundColourId, p.panelRaised);
    setColour(juce::PopupMenu::textColourId, p.textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, (s_themeMode == ThemeMode::Dark) ? p.panelRaised.brighter(0.08f) : p.panelSunken);
    setColour(juce::PopupMenu::highlightedTextColourId, p.accentCyan);
    setColour(juce::TextButton::textColourOffId, p.textSecondary);
    setColour(juce::TextButton::textColourOnId, p.accentAmber);
    setColour(juce::TooltipWindow::backgroundColourId, p.panelSunken);
    setColour(juce::TooltipWindow::textColourId, p.textPrimary);
    setColour(juce::TooltipWindow::outlineColourId, p.panelBorderLight);
}

void FreePitchLookAndFeel::drawNeumorphicRaised(juce::Graphics& g, juce::Rectangle<float> bounds,
                                                float cornerRadius, float depth, bool isHovered) {
    const auto& p = s_currentPalette;
    float off = depth * 0.7f;

    // 1. Soft specular highlight on top-left (-x, -y)
    g.setColour(p.shadowLight);
    g.fillRoundedRectangle(bounds.translated(-off, -off), cornerRadius);

    // 2. Soft ambient drop shadow on bottom-right (+x, +y)
    g.setColour(p.shadowDark);
    g.fillRoundedRectangle(bounds.translated(off, off), cornerRadius);

    // 3. Extruded surface face (Mono-material)
    g.setColour(isHovered ? p.panelRaised.brighter(0.03f) : p.panelRaised);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // 4. Subtle 3D chamfer hairline rim (light on top-left, darker on bottom-right)
    juce::ColourGradient borderGrad(p.panelBorderLight, bounds.getX(), bounds.getY(),
                                    p.panelBorder, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(borderGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void FreePitchLookAndFeel::drawNeumorphicSunken(juce::Graphics& g, juce::Rectangle<float> bounds,
                                                float cornerRadius, float depth) {
    const auto& p = s_currentPalette;

    // 1. Inset well base fill
    g.setColour(p.panelSunken);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // 2. Inner dark shadow from top-left
    juce::ColourGradient darkGrad(p.innerShadowDark, bounds.getX(), bounds.getY(),
                                  p.panelSunken, bounds.getX() + depth * 2.0f, bounds.getY() + depth * 2.0f, false);
    g.setGradientFill(darkGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, depth);

    // 3. Inner light specular rim from bottom-right
    juce::ColourGradient lightGrad(p.panelSunken, bounds.getRight() - depth * 1.5f, bounds.getBottom() - depth * 1.5f,
                                   p.innerShadowLight, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(lightGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void FreePitchLookAndFeel::drawTactileLed(juce::Graphics& g, float cx, float cy, float radius,
                                          const juce::Colour& color, bool isOn) {
    const auto& p = s_currentPalette;
    juce::Rectangle<float> ledBounds(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    if (isOn) {
        // Soft ambient glow
        g.setColour(color.withAlpha(0.35f));
        g.fillEllipse(ledBounds.expanded(radius * 0.6f));

        // Lens body
        g.setColour(color);
        g.fillEllipse(ledBounds);

        // Subtle specular highlight hotspot
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.fillEllipse(cx - radius * 0.4f, cy - radius * 0.4f, radius * 0.6f, radius * 0.6f);
    } else {
        // Inset dark socket
        g.setColour(p.panelSunken);
        g.fillEllipse(ledBounds);
        g.setColour(p.panelBorder);
        g.drawEllipse(ledBounds, 0.8f);
    }
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

    const auto& p = s_currentPalette;

    bool useAmber = slider.getProperties().contains("accentAmber") && static_cast<bool>(slider.getProperties()["accentAmber"]);
    juce::Colour activeColor = useAmber ? p.accentAmber : p.accentCyan;

    // 1. Debossed Concave Dish / Socket Well
    juce::Rectangle<float> socketBounds(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    drawNeumorphicSunken(g, socketBounds, radius, 2.5f);

    // 2. Background Track Groove (Subtle recessed arc)
    float trackRadius = radius - 3.5f;
    if (trackRadius > 4.0f) {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(p.panelBorder.withAlpha(0.45f));
        g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Active Illuminated Track Arc
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPosProportional > 0.001f && trackRadius > 4.0f) {
        juce::Path valArc;
        valArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, currentAngle, true);

        if (isEnabled) {
            // Radiant soft bloom
            g.setColour(activeColor.withAlpha(0.30f));
            g.strokePath(valArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(isEnabled ? activeColor : p.textDim);
        g.strokePath(valArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Pure Neumorphic Extruded Knob Button (Molded from same surface material)
    float dialRadius = radius - 8.5f;
    if (dialRadius > 6.0f) {
        juce::Rectangle<float> dialBounds(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

        // Soft ambient drop shadow (bottom-right)
        g.setColour(p.shadowDark);
        g.fillEllipse(dialBounds.translated(2.2f, 2.2f));

        // Soft specular highlight (top-left)
        g.setColour(p.shadowLight);
        g.fillEllipse(dialBounds.translated(-1.8f, -1.8f));

        // Extruded knob cap body (Exact same material as panel)
        g.setColour(isHovered ? p.panelRaised.brighter(0.03f) : p.panelRaised);
        g.fillEllipse(dialBounds);

        // Subtle 3D chamfer rim hairline
        juce::ColourGradient dialRim(p.panelBorderLight, centreX - dialRadius, centreY - dialRadius,
                                     p.panelBorder, centreX + dialRadius, centreY + dialRadius, false);
        g.setGradientFill(dialRim);
        g.drawEllipse(dialBounds, 1.0f);

        // 5. Center Finger Dimple (Soft concave dish)
        float dimpleRadius = dialRadius * 0.38f;
        juce::Rectangle<float> dimpleBounds(centreX - dimpleRadius, centreY - dimpleRadius,
                                            dimpleRadius * 2.0f, dimpleRadius * 2.0f);
        drawNeumorphicSunken(g, dimpleBounds, dimpleRadius, 1.5f);

        // 6. Minimalist Illuminated Indicator Pip
        float pipDist = dialRadius * 0.70f;
        float pipX = centreX + std::sin(currentAngle) * pipDist;
        float pipY = centreY - std::cos(currentAngle) * pipDist;
        float pipR = isHovered ? 2.6f : 2.0f;

        if (isEnabled) {
            // Subtle neon glow
            g.setColour(activeColor.withAlpha(0.35f));
            g.fillEllipse(pipX - pipR - 1.5f, pipY - pipR - 1.5f, (pipR + 1.5f) * 2.0f, (pipR + 1.5f) * 2.0f);

            g.setColour(isHovered ? juce::Colours::white : activeColor);
            g.fillEllipse(pipX - pipR, pipY - pipR, pipR * 2.0f, pipR * 2.0f);
        } else {
            g.setColour(p.textDim);
            g.fillEllipse(pipX - pipR, pipY - pipR, pipR * 2.0f, pipR * 2.0f);
        }
    }
}

void FreePitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    bool isHovered = box.isMouseOver() || isButtonDown;
    const auto& p = s_currentPalette;

    if (isButtonDown) {
        drawNeumorphicSunken(g, bounds, 6.0f, 2.5f);
    } else {
        drawNeumorphicRaised(g, bounds, 6.0f, 2.5f, isHovered);
    }

    // Clean modern chevron arrow on right
    float arrowX = static_cast<float>(width) - 15.0f;
    float arrowY = static_cast<float>(height) * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 4.0f, arrowY - 2.5f);
    arrow.lineTo(arrowX, arrowY + 2.5f);
    arrow.lineTo(arrowX + 4.0f, arrowY - 2.5f);

    g.setColour(box.isEnabled() ? (isHovered ? p.accentCyan : p.textMuted) : p.textDim);
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}

void FreePitchLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    bool isEnabled = button.isEnabled();
    const auto& p = s_currentPalette;

    bool useAmber = button.getProperties().contains("accentAmber") && static_cast<bool>(button.getProperties()["accentAmber"]);
    juce::Colour activeColor = useAmber ? p.accentAmber : p.indicatorInTune;

    // Draw tactile circular LED pip
    float pipSize = 8.5f;
    float pipY = bounds.getCentreY();
    drawTactileLed(g, 9.0f, pipY, pipSize * 0.5f, activeColor, isOn && isEnabled);

    // Typography
    g.setColour(isOn ? p.textPrimary : (shouldDrawButtonAsHighlighted ? p.textSecondary : p.textMuted));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText(button.getButtonText(), 19, 0, button.getWidth() - 19, button.getHeight(),
               juce::Justification::centredLeft, true);
}

void FreePitchLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& /*backgroundColour*/,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    bool isPressed = shouldDrawButtonAsDown || isOn;

    if (isPressed) {
        // Depressed into surface
        drawNeumorphicSunken(g, bounds, 6.0f, 2.5f);
        if (isOn) {
            g.setColour(s_currentPalette.accentCyan.withAlpha(0.6f));
            g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        }
    } else {
        // Raised extruded tile
        drawNeumorphicRaised(g, bounds, 6.0f, 2.5f, shouldDrawButtonAsHighlighted);
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
    const auto& p = s_currentPalette;

    if (isSeparator) {
        g.setColour(p.panelBorder);
        g.fillRect(area.reduced(6, 0).withHeight(1));
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour((s_themeMode == ThemeMode::Dark) ? p.panelRaised.brighter(0.08f) : p.panelSunken);
        g.fillRect(area.reduced(2, 1));

        // Cyan left accent pill
        g.setColour(p.accentCyan);
        g.fillRect(area.getX() + 2, area.getY() + 2, 3, area.getHeight() - 4);
    }

    juce::Colour textColor = isActive ? (isHighlighted ? p.accentCyan : p.textPrimary) : p.textDim;
    if (textColour != nullptr) {
        textColor = *textColour;
    }

    g.setColour(textColor);
    g.setFont(juce::FontOptions(12.5f));

    int leftMargin = isTicked ? 24 : 14;
    g.drawText(text, area.getX() + leftMargin, area.getY(), area.getWidth() - leftMargin - 10, area.getHeight(),
               juce::Justification::centredLeft, true);

    if (isTicked) {
        g.setColour(p.accentCyan);
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
    const auto& p = s_currentPalette;

    // Neumorphic dark glass floating badge
    g.setColour(p.panelSunken.withAlpha(0.96f));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Neon cyan hairline border with soft glow
    g.setColour(p.accentCyan.withAlpha(0.55f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Crisp typography
    g.setColour(p.textPrimary);
    g.setFont(juce::FontOptions(11.5f));
    g.drawFittedText(text, bounds.reduced(8.0f, 6.0f).toNearestInt(), juce::Justification::centredLeft, 6);
}

juce::Rectangle<int> FreePitchLookAndFeel::getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos,
                                                            juce::Rectangle<int> parentArea) {
    juce::Font font(juce::FontOptions(11.5f));
    int textWidth = font.getStringWidth(tipText);

    int maxWidth = std::min(320, parentArea.getWidth() - 20);
    int width = std::min(maxWidth, std::max(140, textWidth + 24));

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
