#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FreePitchLookAndFeel : public juce::LookAndFeel_V4 {
public:
    FreePitchLookAndFeel();
    ~FreePitchLookAndFeel() override = default;

    // Classic Studio Pitch Correction Palette
    static inline const juce::Colour kBackgroundDark    = juce::Colour(0xFF0C0E12); // Deep obsidian chassis
    static inline const juce::Colour kPanelDark         = juce::Colour(0xFF13161C); // Studio rack bay surface
    static inline const juce::Colour kPanelCard         = juce::Colour(0xFF191D26); // Elevated card container
    static inline const juce::Colour kPanelCardHover    = juce::Colour(0xFF202531); // Hover card
    static inline const juce::Colour kPanelBorder       = juce::Colour(0xFF282F3D); // Hairline frame
    static inline const juce::Colour kPanelBorderLight  = juce::Colour(0xFF3B4559); // Highlight frame
    static inline const juce::Colour kPanelBorderGlow   = juce::Colour(0x6600D2FF); // Cyan glow border

    static inline const juce::Colour kTextWhite         = juce::Colour(0xFFF8FAFC); // Crisp white
    static inline const juce::Colour kTextLight         = juce::Colour(0xFFCBD5E1); // Clean slate
    static inline const juce::Colour kTextMuted         = juce::Colour(0xFF8B9BB4); // Technical grey
    static inline const juce::Colour kTextDim           = juce::Colour(0xFF4C566A); // Subdued text

    static inline const juce::Colour kAccentCyan        = juce::Colour(0xFF00D2FF); // Neon cyan (live pitch / primary)
    static inline const juce::Colour kAccentCyanGlow    = juce::Colour(0x4000D2FF); // Soft cyan halo
    static inline const juce::Colour kAccentAmber       = juce::Colour(0xFFF59E0B); // Studio amber/gold (quantize / correction)
    static inline const juce::Colour kAccentAmberGlow   = juce::Colour(0x40F59E0B); // Soft amber halo
    static inline const juce::Colour kIndicatorInTune   = juce::Colour(0xFF10B981); // Emerald green (in-tune / active LED)
    static inline const juce::Colour kIndicatorSharp    = juce::Colour(0xFFFB923C); // Warm orange for sharp
    static inline const juce::Colour kIndicatorFlat     = juce::Colour(0xFF38BDF8); // Electric sky for flat
    static inline const juce::Colour kIndicatorWarning  = juce::Colour(0xFFEF4444); // Red warning

    static inline const juce::Colour kKnobBodyTop       = juce::Colour(0xFF242936); // Metallic dial gradient top
    static inline const juce::Colour kKnobBodyBottom    = juce::Colour(0xFF151821); // Metallic dial gradient bottom
    static inline const juce::Colour kKnobRim           = juce::Colour(0xFF373E4F); // Outer machined rim

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
};
