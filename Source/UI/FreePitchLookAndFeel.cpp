#include "FreePitchLookAndFeel.h"
#include <cmath>

FreePitchLookAndFeel::ThemePalette FreePitchLookAndFeel::makeDarkPalette() {
    ThemePalette p;
    p.bgBase            = juce::Colour(0xFF16191F); // Deep obsidian slate
    p.panelRaised       = juce::Colour(0xFF1C2028); // Elevated tactile card
    p.panelSunken       = juce::Colour(0xFF12141A); // Sunken well / recessed cavity
    p.panelBorder       = juce::Colour(0xFF252B37); // Frame hairline
    p.panelBorderLight  = juce::Colour(0xFF323B4C); // Bevel top-left rim highlight
    p.shadowDark        = juce::Colour(0x99080A0E); // Deep ambient drop shadow
    p.shadowLight       = juce::Colour(0x3348546A); // Specular highlight shadow
    p.innerShadowDark   = juce::Colour(0xCC090B0F); // Recessed inner shadow
    p.innerShadowLight  = juce::Colour(0x2B3B485E); // Inset specular rim

    p.textPrimary       = juce::Colour(0xFFF8FAFC); // Crisp off-white
    p.textSecondary     = juce::Colour(0xFFCBD5E1); // Clean slate
    p.textMuted         = juce::Colour(0xFF8492A6); // Technical grey
    p.textDim           = juce::Colour(0xFF475569); // Muted inactive

    p.accentCyan        = juce::Colour(0xFF00E5FF); // Radiant electric cyan
    p.accentCyanGlow    = juce::Colour(0x4000E5FF);
    p.accentAmber       = juce::Colour(0xFFFFB300); // Warm studio amber gold
    p.accentAmberGlow   = juce::Colour(0x40FFB300);
    p.indicatorInTune   = juce::Colour(0xFF00E676); // Emerald phosphor LED
    p.indicatorSharp    = juce::Colour(0xFFFB923C);
    p.indicatorFlat     = juce::Colour(0xFF38BDF8);
    p.indicatorWarning  = juce::Colour(0xFFEF4444);

    p.knobCapTop        = juce::Colour(0xFF2C3240); // Machined dark gunmetal top
    p.knobCapBottom     = juce::Colour(0xFF181B23); // Machined dark gunmetal bottom
    p.knobRim           = juce::Colour(0xFF3B4457); // Milled edge rim
    p.knobRimHighlight  = juce::Colour(0xFF54627E); // Edge knurl specular tooth
    p.glassGlare        = juce::Colour(0x22FFFFFF); // Convex glass lens glare

    return p;
}

FreePitchLookAndFeel::ThemePalette FreePitchLookAndFeel::makeLightPalette() {
    ThemePalette p;
    p.bgBase            = juce::Colour(0xFFE0E5EC); // Classic studio neumorphic silver cream
    p.panelRaised       = juce::Colour(0xFFE8EDF4); // Soft extruded card
    p.panelSunken       = juce::Colour(0xFFD3D9E2); // Sunken well / recessed cavity
    p.panelBorder       = juce::Colour(0xFFBAC5D3); // Subtle hairline
    p.panelBorderLight  = juce::Colour(0xFFFFFFFF); // Specular top-left rim highlight
    p.shadowDark        = juce::Colour(0x7797A5B8); // Soft ambient drop shadow
    p.shadowLight       = juce::Colour(0xEEFFFFFF); // Pure white specular highlight shadow
    p.innerShadowDark   = juce::Colour(0x668695A8); // Recessed inner shadow
    p.innerShadowLight  = juce::Colour(0xDDFFFFFF); // Inset specular rim

    p.textPrimary       = juce::Colour(0xFF1E293B); // Deep charcoal
    p.textSecondary     = juce::Colour(0xFF334155); // Slate
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

    p.knobCapTop        = juce::Colour(0xFFFFFFFF); // Brushed satin aluminum top
    p.knobCapBottom     = juce::Colour(0xFFD1D8E3); // Brushed satin aluminum bottom
    p.knobRim           = juce::Colour(0xFFBAC5D4);
    p.knobRimHighlight  = juce::Colour(0xFFFFFFFF);
    p.glassGlare        = juce::Colour(0x55FFFFFF); // Crisp glass reflection

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
    kPanelCardHover   = (s_themeMode == ThemeMode::Dark) ? p.panelRaised.brighter(0.06f) : p.panelRaised.darker(0.04f);
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

    kKnobBodyTop      = p.knobCapTop;
    kKnobBodyBottom   = p.knobCapBottom;
    kKnobRim          = p.knobRim;

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

    // 1. Specular highlight shadow on top-left (-x, -y)
    g.setColour(p.shadowLight);
    g.fillRoundedRectangle(bounds.translated(-off, -off), cornerRadius);

    // 2. Ambient drop shadow on bottom-right (+x, +y)
    g.setColour(p.shadowDark);
    g.fillRoundedRectangle(bounds.translated(off, off), cornerRadius);

    // 3. Raised surface face
    g.setColour(isHovered ? p.panelRaised.brighter(0.04f) : p.panelRaised);
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

    // 1. Sunken well base fill
    g.setColour(p.panelSunken);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // 2. Inner dark shadow from top-left
    juce::ColourGradient darkGrad(p.innerShadowDark, bounds.getX(), bounds.getY(),
                                  p.panelSunken, bounds.getX() + depth * 2.2f, bounds.getY() + depth * 2.2f, false);
    g.setGradientFill(darkGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, depth);

    // 3. Inner light specular rim from bottom-right
    juce::ColourGradient lightGrad(p.panelSunken, bounds.getRight() - depth * 1.5f, bounds.getBottom() - depth * 1.5f,
                                   p.innerShadowLight, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(lightGrad);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void FreePitchLookAndFeel::drawGlassGlare(juce::Graphics& g, juce::Rectangle<float> bounds, float cornerRadius) {
    const auto& p = s_currentPalette;
    auto glareArea = bounds.removeFromTop(bounds.getHeight() * 0.44f).reduced(2.0f, 2.0f);

    juce::ColourGradient glareGrad(p.glassGlare, glareArea.getCentreX(), glareArea.getY(),
                                   p.glassGlare.withAlpha(0.0f), glareArea.getCentreX(), glareArea.getBottom(), false);
    g.setGradientFill(glareGrad);
    g.fillRoundedRectangle(glareArea, cornerRadius);
}

void FreePitchLookAndFeel::drawTactileLed(juce::Graphics& g, float cx, float cy, float radius,
                                          const juce::Colour& color, bool isOn) {
    const auto& p = s_currentPalette;
    juce::Rectangle<float> ledBounds(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    if (isOn) {
        // Wide ambient glow
        g.setColour(color.withAlpha(0.38f));
        g.fillEllipse(ledBounds.expanded(radius * 0.7f));

        // Lens body
        juce::ColourGradient lensGrad(color.brighter(0.2f), cx - radius * 0.3f, cy - radius * 0.3f,
                                      color.darker(0.3f), cx + radius * 0.5f, cy + radius * 0.5f, true);
        g.setGradientFill(lensGrad);
        g.fillEllipse(ledBounds);

        // Specular highlight hotspot
        g.setColour(juce::Colours::white.withAlpha(0.90f));
        g.fillEllipse(cx - radius * 0.45f, cy - radius * 0.45f, radius * 0.7f, radius * 0.7f);
    } else {
        // Inset dark socket
        g.setColour(p.panelSunken);
        g.fillEllipse(ledBounds);
        g.setColour(p.panelBorder);
        g.drawEllipse(ledBounds, 1.0f);

        // Unlit glass lens
        g.setColour(p.panelSunken.withAlpha(0.6f));
        g.fillEllipse(ledBounds.reduced(1.0f));
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
    juce::Colour activeGlow  = useAmber ? p.accentAmberGlow : p.accentCyanGlow;

    // 1. Debossed Circular Socket Well (Neumorphic Inset Circle)
    juce::Rectangle<float> socketBounds(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    g.setColour(p.panelSunken);
    g.fillEllipse(socketBounds);

    // Inner shadow of the socket
    juce::ColourGradient socketInnerDark(p.innerShadowDark, centreX - radius, centreY - radius,
                                         p.panelSunken, centreX, centreY, true);
    g.setGradientFill(socketInnerDark);
    g.drawEllipse(socketBounds.reduced(0.5f), 2.5f);

    // 2. Background Track Arc (Sunken groove)
    float trackRadius = radius - 3.5f;
    if (trackRadius > 4.0f) {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(p.panelBorder);
        g.strokePath(bgArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Active Illuminated Track Arc
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPosProportional > 0.001f && trackRadius > 4.0f) {
        juce::Path valArc;
        valArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, currentAngle, true);

        if (isEnabled) {
            // Radiant bloom under arc
            g.setColour(activeColor.withAlpha(0.35f));
            g.strokePath(valArc, juce::PathStrokeType(6.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(isEnabled ? activeColor : p.textDim);
        g.strokePath(valArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Skeuomorphic Cylindrical Dial Cap
    float dialRadius = radius - 9.0f;
    if (dialRadius > 6.0f) {
        juce::Rectangle<float> dialBounds(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

        // Soft drop shadow cast onto socket
        g.setColour(p.shadowDark);
        g.fillEllipse(dialBounds.translated(0.0f, 3.0f));

        // Milled Knurl Texture around dial edge (16 teeth notches)
        int numTeeth = 16;
        for (int i = 0; i < numTeeth; ++i) {
            float ang = (static_cast<float>(i) / static_cast<float>(numTeeth)) * juce::MathConstants<float>::twoPi;
            float cosA = std::cos(ang);
            float sinA = std::sin(ang);
            float rInner = dialRadius - 2.5f;
            float rOuter = dialRadius;
            g.setColour(i % 2 == 0 ? p.knobRimHighlight : p.knobRim);
            g.drawLine(centreX + cosA * rInner, centreY + sinA * rInner,
                       centreX + cosA * rOuter, centreY + sinA * rOuter, 1.2f);
        }

        // Machined metallic face gradient (simulating brushed aluminum or gunmetal)
        juce::ColourGradient dialGrad(isHovered ? p.knobCapTop.brighter(0.05f) : p.knobCapTop,
                                      centreX - dialRadius * 0.5f, centreY - dialRadius * 0.5f,
                                      p.knobCapBottom, centreX + dialRadius * 0.5f, centreY + dialRadius * 0.5f, false);
        g.setGradientFill(dialGrad);
        g.fillEllipse(dialBounds.reduced(1.2f));

        // Chamfered Bevel Rim
        g.setColour(isHovered ? activeColor.withAlpha(0.6f) : p.knobRim);
        g.drawEllipse(dialBounds.reduced(1.2f), 1.0f);

        // Concentric hairline machined ring
        float innerRingR = dialRadius * 0.65f;
        g.setColour(p.panelBorder.withAlpha(0.5f));
        g.drawEllipse(centreX - innerRingR, centreY - innerRingR, innerRingR * 2.0f, innerRingR * 2.0f, 0.8f);

        // 5. Indicator Needle Pointer (Etched groove with luminous tip)
        juce::Path needle;
        float needleStart = dialRadius * 0.28f;
        float needleEnd   = dialRadius * 0.86f;

        float sinA = std::sin(currentAngle);
        float cosA = std::cos(currentAngle);

        needle.startNewSubPath(centreX + sinA * needleStart, centreY - cosA * needleStart);
        needle.lineTo(centreX + sinA * needleEnd,   centreY - cosA * needleEnd);

        if (isEnabled) {
            g.setColour(activeColor.withAlpha(0.45f));
            g.strokePath(needle, juce::PathStrokeType(3.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

            g.setColour(isHovered ? juce::Colours::white : activeColor);
            g.strokePath(needle, juce::PathStrokeType(2.2f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        } else {
            g.setColour(p.textDim);
            g.strokePath(needle, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }

        // Center jewel dot
        float dotRadius = 2.2f;
        g.setColour(isEnabled ? (isHovered ? juce::Colours::white : activeColor) : p.textDim);
        g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
}

void FreePitchLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    bool isHovered = box.isMouseOver() || isButtonDown;
    const auto& p = s_currentPalette;

    if (isButtonDown) {
        drawNeumorphicSunken(g, bounds, 4.0f, 2.5f);
    } else {
        drawNeumorphicRaised(g, bounds, 4.0f, 2.5f, isHovered);
    }

    // 3D metallic chevron arrow on right
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

    // Draw tactile jewel LED lamp
    float pipSize = 9.0f;
    float pipY = bounds.getCentreY();
    drawTactileLed(g, 9.0f, pipY, pipSize * 0.5f, activeColor, isOn && isEnabled);

    // Text Label with active theme color
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
        // Sunken / depressed into chassis
        drawNeumorphicSunken(g, bounds, 4.0f, 2.5f);
        if (isOn) {
            g.setColour(s_currentPalette.accentCyan.withAlpha(0.6f));
            g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
        }
    } else {
        // Raised extruded tile
        drawNeumorphicRaised(g, bounds, 4.0f, 3.0f, shouldDrawButtonAsHighlighted);
    }
}

void FreePitchLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    drawNeumorphicRaised(g, bounds, 5.0f, 4.0f);
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

    // Dark glass / frosted surface with dual shadows
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
