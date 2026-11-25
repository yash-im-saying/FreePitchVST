#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FreePitchLookAndFeel.h"

// ============================================================================
// NeumorphicTogglePill (e.g., FORMANT LOCK, CHOOSY TRACKING)
// ============================================================================
class NeumorphicTogglePill : public juce::Component,
                             public juce::SettableTooltipClient {
public:
    NeumorphicTogglePill(const juce::String& labelText, const juce::String& tooltipText = "")
        : m_label(labelText) {
        setTooltip(tooltipText);
        m_toggleButton.setButtonText("");
        m_toggleButton.setTooltip(tooltipText);
        addAndMakeVisible(m_toggleButton);
        m_toggleButton.setAlpha(0.0f); // invisible hit-target
        m_toggleButton.onStateChange = [this]() { repaint(); };
    }

    juce::ToggleButton& getToggleButton() { return m_toggleButton; }

    void paint(juce::Graphics& g) override {
        using namespace FreePitchColors;
        auto bounds = getLocalBounds().toFloat();

        // Top portion: switch track (height ~18px)
        float trackH = 18.0f;
        float trackW = 38.0f;
        float trackX = bounds.getCentreX() - trackW * 0.5f;
        float trackY = 2.0f;
        auto trackRect = juce::Rectangle<float>(trackX, trackY, trackW, trackH);

        // Recessed cavity trough (#101215)
        FreePitchLookAndFeel::drawNeumorphicSunken(g, trackRect, trackH * 0.5f, 2.0f);

        bool isOn = m_toggleButton.getToggleState();

        // Sliding pill cap
        float pillW = 16.0f;
        float pillH = 14.0f;
        float pillX = isOn ? (trackRect.getRight() - pillW - 2.0f) : (trackRect.getX() + 2.0f);
        float pillY = trackRect.getY() + (trackH - pillH) * 0.5f;
        auto pillRect = juce::Rectangle<float>(pillX, pillY, pillW, pillH);

        // Elevated surface cap
        FreePitchLookAndFeel::drawNeumorphicRaised(g, pillRect, pillH * 0.5f, 2.0f);

        // LED dot inside the pill
        float ledR = 2.5f;
        float ledCx = pillRect.getCentreX();
        float ledCy = pillRect.getCentreY();
        FreePitchLookAndFeel::drawTactileLed(g, ledCx, ledCy, ledR, signalRed, isOn);

        // Label below
        float labelY = trackRect.getBottom() + 3.0f;
        g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        g.setColour(isOn ? textPrimary : textSecondary);
        g.drawText(m_label.toUpperCase(), 0, static_cast<int>(labelY), getWidth(), 14,
                   juce::Justification::centred, false);
    }

    void resized() override {
        m_toggleButton.setBounds(getLocalBounds());
    }

    void mouseDown(const juce::MouseEvent&) override {
        m_toggleButton.setToggleState(!m_toggleButton.getToggleState(), juce::sendNotification);
    }

private:
    juce::String m_label;
    juce::ToggleButton m_toggleButton;
};

// ============================================================================
// SegmentedLatencyToggle (TRACK vs LOOKAHEAD)
// ============================================================================
class SegmentedLatencyToggle : public juce::Component,
                               public juce::SettableTooltipClient {
public:
    SegmentedLatencyToggle() {
        setTooltip("Latency Mode: TRACK (ultra-low latency live tracking) vs LOOKAHEAD (20 ms lookahead for mixing).");
        m_button.setButtonText("");
        addAndMakeVisible(m_button);
        m_button.setAlpha(0.0f);
        m_button.onStateChange = [this]() { repaint(); };
    }

    juce::ToggleButton& getButton() { return m_button; }

    void paint(juce::Graphics& g) override {
        using namespace FreePitchColors;
        auto bounds = getLocalBounds().toFloat();

        float capsuleH = 24.0f;
        auto capsuleRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), bounds.getWidth(), capsuleH);

        // 1. Recessed capsule trough (#101215)
        FreePitchLookAndFeel::drawNeumorphicSunken(g, capsuleRect, capsuleH * 0.5f, 2.5f);

        bool isLookahead = m_button.getToggleState();
        float halfW = capsuleRect.getWidth() * 0.5f;

        // 2. Active elevated pill over selected option
        auto activePill = isLookahead
            ? juce::Rectangle<float>(capsuleRect.getX() + halfW + 1.0f, capsuleRect.getY() + 2.0f, halfW - 3.0f, capsuleH - 4.0f)
            : juce::Rectangle<float>(capsuleRect.getX() + 2.0f, capsuleRect.getY() + 2.0f, halfW - 3.0f, capsuleH - 4.0f);

        FreePitchLookAndFeel::drawNeumorphicRaised(g, activePill, activePill.getHeight() * 0.5f, 2.0f);

        // 3. Option texts
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));

        // Left: TRACK
        g.setColour(!isLookahead ? textPrimary : textSecondary);
        g.drawText("TRACK", static_cast<int>(capsuleRect.getX()), static_cast<int>(capsuleRect.getY()),
                   static_cast<int>(halfW), static_cast<int>(capsuleH), juce::Justification::centred, false);

        // Right: LOOKAHEAD
        g.setColour(isLookahead ? textPrimary : textSecondary);
        g.drawText("LOOKAHEAD", static_cast<int>(capsuleRect.getX() + halfW), static_cast<int>(capsuleRect.getY()),
                   static_cast<int>(halfW), static_cast<int>(capsuleH), juce::Justification::centred, false);

        // 4. Subtitles below
        float subY = capsuleRect.getBottom() + 3.0f;
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        g.setColour(textSecondary);
        g.drawText("LATENCY", static_cast<int>(capsuleRect.getX()), static_cast<int>(subY),
                   static_cast<int>(halfW), 12, juce::Justification::centred, false);
        g.drawText("LOOKAHEAD", static_cast<int>(capsuleRect.getX() + halfW), static_cast<int>(subY),
                   static_cast<int>(halfW), 12, juce::Justification::centred, false);
    }

    void resized() override {
        m_button.setBounds(getLocalBounds());
    }

    void mouseDown(const juce::MouseEvent& e) override {
        bool chooseLookahead = (e.x > getWidth() / 2);
        m_button.setToggleState(chooseLookahead, juce::sendNotification);
    }

private:
    juce::ToggleButton m_button;
};

// ============================================================================
// CircularBypassButton (Power Symbol ⏻)
// ============================================================================
class CircularBypassButton : public juce::Component,
                             public juce::SettableTooltipClient {
public:
    CircularBypassButton() {
        setTooltip("Master EFX / Processor Bypass");
        m_button.setButtonText("");
        addAndMakeVisible(m_button);
        m_button.setAlpha(0.0f);
        m_button.onStateChange = [this]() { repaint(); };
    }

    juce::ToggleButton& getButton() { return m_button; }

    void paint(juce::Graphics& g) override {
        using namespace FreePitchColors;
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();

        bool isOn = m_button.getToggleState();

        // 1. Recessed outer socket (#101215)
        juce::Rectangle<float> socket(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        FreePitchLookAndFeel::drawNeumorphicSunken(g, socket, radius, 2.0f);

        // 2. Elevated circular button cap (#1E2127)
        float capR = radius - 3.5f;
        juce::Rectangle<float> cap(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
        FreePitchLookAndFeel::drawNeumorphicRaised(g, cap, capR, 2.0f, isMouseOver(), false);

        // 3. Power icon (IEC 5009 ⏻)
        juce::Colour iconCol = isOn ? signalRed : textDim;
        float iconR = capR * 0.50f;

        juce::Path powerArc;
        powerArc.addCentredArc(cx, cy, iconR, iconR, 0.0f,
                               juce::MathConstants<float>::pi * 0.25f,
                               juce::MathConstants<float>::pi * 1.75f, true);

        if (isOn) {
            g.setColour(signalRedBloom);
            g.strokePath(powerArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.drawLine(cx, cy - iconR - 2.0f, cx, cy - 1.0f, 4.5f);
        }

        g.setColour(iconCol);
        g.strokePath(powerArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawLine(cx, cy - iconR - 2.0f, cx, cy - 1.0f, 2.0f);
    }

    void resized() override {
        m_button.setBounds(getLocalBounds());
    }

    void mouseDown(const juce::MouseEvent&) override {
        m_button.setToggleState(!m_button.getToggleState(), juce::sendNotification);
    }

private:
    juce::ToggleButton m_button;
};

// ============================================================================
// HeaderPillSelector (Clean recessed capsule dropdown with subtitle)
// ============================================================================
class HeaderPillSelector : public juce::Component,
                           public juce::SettableTooltipClient {
public:
    HeaderPillSelector(const juce::String& subtitleText)
        : m_subtitle(subtitleText) {
        addAndMakeVisible(m_comboBox);
        m_comboBox.setAlpha(0.0f); // invisible hit target; custom rendering below
        m_comboBox.onChange = [this]() { repaint(); };
    }

    juce::ComboBox& getComboBox() { return m_comboBox; }

    void paint(juce::Graphics& g) override {
        using namespace FreePitchColors;
        auto bounds = getLocalBounds().toFloat();

        float pillH = 26.0f;
        auto pillRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), bounds.getWidth(), pillH);

        // 1. Recessed capsule dish (#101215)
        FreePitchLookAndFeel::drawNeumorphicSunken(g, pillRect, pillH * 0.5f, 2.0f);

        if (isMouseOver()) {
            g.setColour(rimHighlight.withAlpha(0.7f));
            g.drawRoundedRectangle(pillRect.reduced(0.5f), pillH * 0.5f, 1.0f);
        }

        // 2. Selected text
        juce::String selectedText = m_comboBox.getText();
        if (selectedText.isEmpty() && m_comboBox.getNumItems() > 0) {
            selectedText = m_comboBox.getItemText(0);
        }

        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(textPrimary);
        g.drawText(selectedText, static_cast<int>(pillRect.getX() + 10.0f), static_cast<int>(pillRect.getY()),
                   static_cast<int>(pillRect.getWidth() - 26.0f), static_cast<int>(pillH),
                   juce::Justification::centredLeft, true);

        // 3. Modern down chevron
        float arrowX = pillRect.getRight() - 12.0f;
        float arrowY = pillRect.getCentreY();
        juce::Path arrow;
        arrow.startNewSubPath(arrowX - 3.5f, arrowY - 2.0f);
        arrow.lineTo(arrowX, arrowY + 2.0f);
        arrow.lineTo(arrowX + 3.5f, arrowY - 2.0f);

        g.setColour(isMouseOver() ? textPrimary : textSecondary);
        g.strokePath(arrow, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

        // 4. Subtitle label below (if specified)
        if (m_subtitle.isNotEmpty()) {
            float subY = pillRect.getBottom() + 3.0f;
            g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
            g.setColour(textSecondary);
            g.drawText(m_subtitle.toUpperCase(), static_cast<int>(bounds.getX()), static_cast<int>(subY),
                       static_cast<int>(bounds.getWidth()), 12, juce::Justification::centred, false);
        }
    }

    void resized() override {
        m_comboBox.setBounds(getLocalBounds());
    }

private:
    juce::String m_subtitle;
    juce::ComboBox m_comboBox;
};
