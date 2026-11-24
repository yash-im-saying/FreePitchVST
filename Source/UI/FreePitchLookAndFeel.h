#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FreePitchLookAndFeel : public juce::LookAndFeel_V4 {
public:
    FreePitchLookAndFeel();
    ~FreePitchLookAndFeel() override = default;

    // Palette Constants
    static inline const juce::Colour kBackgroundDark    = juce::Colour(0xFF141619); // Matte anodized charcoal
    static inline const juce::Colour kPanelGraphite     = juce::Colour(0xFF1D2025); // Graphite sub-panel
    static inline const juce::Colour kPanelBorder       = juce::Colour(0xFF2C313A); // 1px hairline border
    static inline const juce::Colour kPanelBorderLight  = juce::Colour(0xFF3B424E);
    static inline const juce::Colour kTextWhite         = juce::Colour(0xFFEDEFF2); // Warm off-white
    static inline const juce::Colour kTextMuted         = juce::Colour(0xFF8B929E); // Muted technical grey
    static inline const juce::Colour kTextDim           = juce::Colour(0xFF555B66);
    static inline const juce::Colour kAccentAmber       = juce::Colour(0xFFF59E0B); // Precision amber accent
    static inline const juce::Colour kAccentAmberDim    = juce::Colour(0xFF784D06);
    static inline const juce::Colour kIndicatorFlat     = juce::Colour(0xFF38BDF8); // Cyan for flat
    static inline const juce::Colour kIndicatorSharp    = juce::Colour(0xFFFB923C); // Orange for sharp
    static inline const juce::Colour kIndicatorInTune   = juce::Colour(0xFF10B981); // Emerald green in-tune

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

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
};
