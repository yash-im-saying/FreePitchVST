#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FreePitchLookAndFeel : public juce::LookAndFeel_V4 {
public:
    enum class ThemeMode {
        Dark,
        Light
    };

    struct ThemePalette {
        juce::Colour bgBase;             // Main chassis background
        juce::Colour panelRaised;        // Extruded neumorphic card/surface
        juce::Colour panelSunken;        // Inset cavity / debossed well
        juce::Colour panelBorder;        // Soft perimeter hairline
        juce::Colour panelBorderLight;   // Top-left specular hairline
        juce::Colour shadowDark;         // Ambient soft drop shadow (bottom-right)
        juce::Colour shadowLight;        // Ambient soft highlight shadow (top-left)
        juce::Colour innerShadowDark;    // Inset cavity dark shadow (top-left)
        juce::Colour innerShadowLight;   // Inset cavity highlight (bottom-right)

        // Text
        juce::Colour textPrimary;        // Hero text / values
        juce::Colour textSecondary;      // Labels / titles
        juce::Colour textMuted;          // Subdued / descriptions
        juce::Colour textDim;            // Disabled / inactive

        // Accents & Hardware Indicators
        juce::Colour accentCyan;         // Primary neon cyan (pitch / active)
        juce::Colour accentCyanGlow;     // Primary halo
        juce::Colour accentAmber;        // Secondary gold/amber (quantize / correction)
        juce::Colour accentAmberGlow;    // Secondary halo
        juce::Colour indicatorInTune;    // Emerald green (in-tune / powered ON)
        juce::Colour indicatorSharp;     // Orange
        juce::Colour indicatorFlat;      // Sky blue
        juce::Colour indicatorWarning;   // Crimson alert
    };

    FreePitchLookAndFeel();
    ~FreePitchLookAndFeel() override = default;

    // Theme Management
    void setThemeMode(ThemeMode mode);
    ThemeMode getThemeMode() const { return m_themeMode; }
    bool isDarkTheme() const { return m_themeMode == ThemeMode::Dark; }

    static const ThemePalette& getPalette() { return s_currentPalette; }
    static bool isDark() { return s_themeMode == ThemeMode::Dark; }

    // Pure Neumorphic Primitives
    static void drawNeumorphicRaised(juce::Graphics& g, juce::Rectangle<float> bounds,
                                     float cornerRadius, float depth = 4.0f, bool isHovered = false);

    static void drawNeumorphicSunken(juce::Graphics& g, juce::Rectangle<float> bounds,
                                     float cornerRadius, float depth = 3.0f);

    static void drawTactileLed(juce::Graphics& g, float cx, float cy, float radius,
                               const juce::Colour& color, bool isOn);

    // Dynamically updated static colors for backward compatibility
    static inline juce::Colour kBackgroundDark    = juce::Colour(0xFF181B20);
    static inline juce::Colour kPanelDark         = juce::Colour(0xFF14161A);
    static inline juce::Colour kPanelCard         = juce::Colour(0xFF1E2229);
    static inline juce::Colour kPanelCardHover    = juce::Colour(0xFF242932);
    static inline juce::Colour kPanelBorder       = juce::Colour(0xFF262C36);
    static inline juce::Colour kPanelBorderLight  = juce::Colour(0xFF303846);
    static inline juce::Colour kPanelBorderGlow   = juce::Colour(0x6600E5FF);

    static inline juce::Colour kTextWhite         = juce::Colour(0xFFF8FAFC);
    static inline juce::Colour kTextLight         = juce::Colour(0xFFCBD5E1);
    static inline juce::Colour kTextMuted         = juce::Colour(0xFF8492A6);
    static inline juce::Colour kTextDim           = juce::Colour(0xFF475569);

    static inline juce::Colour kAccentCyan        = juce::Colour(0xFF00E5FF);
    static inline juce::Colour kAccentCyanGlow    = juce::Colour(0x4000E5FF);
    static inline juce::Colour kAccentAmber       = juce::Colour(0xFFFFB300);
    static inline juce::Colour kAccentAmberGlow   = juce::Colour(0x40FFB300);
    static inline juce::Colour kIndicatorInTune   = juce::Colour(0xFF00E676);
    static inline juce::Colour kIndicatorSharp    = juce::Colour(0xFFFB923C);
    static inline juce::Colour kIndicatorFlat     = juce::Colour(0xFF38BDF8);
    static inline juce::Colour kIndicatorWarning  = juce::Colour(0xFFEF4444);

    static inline juce::Colour kKnobBodyTop       = juce::Colour(0xFF1E2229);
    static inline juce::Colour kKnobBodyBottom    = juce::Colour(0xFF181B20);
    static inline juce::Colour kKnobRim           = juce::Colour(0xFF2A313D);

    // JUCE LookAndFeel Overrides
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

private:
    ThemeMode m_themeMode = ThemeMode::Dark;
    static inline ThemeMode s_themeMode = ThemeMode::Dark;
    static inline ThemePalette s_currentPalette;

    static ThemePalette makeDarkPalette();
    static ThemePalette makeLightPalette();
    void applyPaletteToColours();
};
