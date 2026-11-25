#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace FreePitchColors {
    inline const juce::Colour bgChassis       { 0xFF16181C }; // Chassis Background #16181C
    inline const juce::Colour surfaceElevated { 0xFF1E2127 }; // Elevated Surface (Knob Caps, Cards, Active Pills) #1E2127
    inline const juce::Colour surfaceGradTop  { 0xFF22262D }; // Surface Gradient Top-Left #22262D
    inline const juce::Colour surfaceGradBot  { 0xFF191C21 }; // Surface Gradient Bottom-Right #191C21
    inline const juce::Colour cavityRecessed  { 0xFF101215 }; // Recessed Cavity (Pitch Well, Dropdowns, Troughs) #101215
    inline const juce::Colour textPrimary     { 0xFFF2F4F8 }; // Primary Text & Active Ticks #F2F4F8
    inline const juce::Colour textSecondary   { 0xFF7E8492 }; // Secondary Text & Inactive Markers #7E8492
    inline const juce::Colour textDim         { 0xFF424754 }; // Dim Text / Unlit Markers
    inline const juce::Colour signalRed       { 0xFFFF2E3B }; // Signal Accent / Active LED / Value Arc #FF2E3B
    inline const juce::Colour signalRedBloom  { 0x44FF2E3B }; // Soft Outer Bloom 0x44FF2E3B
    inline const juce::Colour rimHighlight    { 0xFF2A2F3A }; // Neumorphic Rim Highlight (Top-Left) #2A2F3A
    inline const juce::Colour deepShadow      { 0xFF090A0C }; // Neumorphic Deep Shadow (Bottom-Right) #090A0C
    inline const juce::Colour innerCavityDark { 0xFF08090B }; // Cavity Inner Shadow Top-Left #08090B
    inline const juce::Colour innerCavityRim  { 0xFF262A33 }; // Cavity Inner Rim Bottom-Right 1px #262A33
    inline const juce::Colour outerKnobTrack  { 0xFF0D0F12 }; // Recessed 3px Dark Outer Track #0D0F12
}

class FreePitchLookAndFeel : public juce::LookAndFeel_V4 {
public:
    enum class ThemeMode {
        Dark,
        Light
    };

    FreePitchLookAndFeel();
    ~FreePitchLookAndFeel() override = default;

    void setThemeMode(ThemeMode mode);
    ThemeMode getThemeMode() const { return m_themeMode; }
    bool isDarkTheme() const { return m_themeMode == ThemeMode::Dark; }
    static bool isDark() { return true; }

    // --- Neumorphic Tactile Primitives ---
    static void drawNeumorphicRaised(juce::Graphics& g, juce::Rectangle<float> bounds,
                                     float cornerRadius, float depth = 4.0f,
                                     bool isHovered = false, bool isPressed = false);

    static void drawNeumorphicSunken(juce::Graphics& g, juce::Rectangle<float> bounds,
                                     float cornerRadius, float depth = 3.0f);

    static void drawTactileLed(juce::Graphics& g, float cx, float cy, float radius,
                               const juce::Colour& color, bool isOn);

    static void drawMachinedCap(juce::Graphics& g, juce::Rectangle<float> dialBounds,
                                float currentAngle, bool isHovered, bool isPressed);

    static void drawRecessedWell(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 float cornerRadius);

    static void drawPillBadge(juce::Graphics& g, juce::Rectangle<float> bounds,
                              const juce::String& text, bool isHovered = false);

    // --- JUCE LookAndFeel Overrides ---
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;

    // Backward-compatibility aliases for legacy code
    static inline juce::Colour kBackgroundDark    = FreePitchColors::bgChassis;
    static inline juce::Colour kPanelDark         = FreePitchColors::cavityRecessed;
    static inline juce::Colour kPanelCard         = FreePitchColors::surfaceElevated;
    static inline juce::Colour kPanelCardHover    = juce::Colour(0xFF242932);
    static inline juce::Colour kPanelBorder       = FreePitchColors::rimHighlight;
    static inline juce::Colour kPanelBorderLight  = juce::Colour(0xFF353B47);
    static inline juce::Colour kPanelBorderGlow   = FreePitchColors::signalRedBloom;
    static inline juce::Colour kTextWhite         = FreePitchColors::textPrimary;
    static inline juce::Colour kTextLight         = FreePitchColors::textPrimary;
    static inline juce::Colour kTextMuted         = FreePitchColors::textSecondary;
    static inline juce::Colour kTextDim           = FreePitchColors::textDim;
    static inline juce::Colour kAccentCyan        = FreePitchColors::signalRed;
    static inline juce::Colour kAccentCyanGlow    = FreePitchColors::signalRedBloom;
    static inline juce::Colour kAccentAmber       = FreePitchColors::signalRed;
    static inline juce::Colour kAccentAmberGlow   = FreePitchColors::signalRedBloom;
    static inline juce::Colour kIndicatorInTune   = FreePitchColors::signalRed;
    static inline juce::Colour kIndicatorSharp    = FreePitchColors::signalRed;
    static inline juce::Colour kIndicatorFlat     = FreePitchColors::signalRed;
    static inline juce::Colour kIndicatorWarning  = FreePitchColors::signalRed;

private:
    ThemeMode m_themeMode = ThemeMode::Dark;
};
