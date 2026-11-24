#include "CircularPitchMeter.h"
#include <cmath>
#include <algorithm>

CircularPitchMeter::CircularPitchMeter() {
}

void CircularPitchMeter::updateTelemetry(const PitchTelemetry& telemetry) {
    m_currentTelemetry = telemetry;

    // Damped ballistics for smooth 60fps rendering
    float alpha = 0.25f;
    if (telemetry.isVoiced) {
        m_smoothCentsDev += alpha * (telemetry.centsDeviation - m_smoothCentsDev);
        m_smoothAppliedCorr += alpha * (telemetry.appliedCorrectionCents - m_smoothAppliedCorr);
        m_smoothConfidence += alpha * (telemetry.voicedConfidence - m_smoothConfidence);
    } else {
        // Decay to zero when unvoiced
        m_smoothCentsDev *= 0.85f;
        m_smoothAppliedCorr *= 0.85f;
        m_smoothConfidence *= 0.80f;
    }

    repaint();
}

void CircularPitchMeter::resized() {
}

juce::String CircularPitchMeter::midiToNoteName(int midiNote) {
    if (midiNote < 0 || midiNote > 127) return "--";
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (midiNote / 12) - 1;
    int note = midiNote % 12;
    return juce::String(names[note]) + juce::String(octave);
}

void CircularPitchMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // 1. Dark sub-panel circle
    g.setColour(FreePitchLookAndFeel::kPanelGraphite);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // 2. Dial scale markings (-50 ct to +50 ct across a 180-degree sweep from -90 deg to +90 deg, TDC at top)
    // TDC (0 ct) is angle 0.0 (top)
    float maxAngle = juce::MathConstants<float>::pi * 0.55f; // ~100 degrees left & right

    // Background track arc
    juce::Path bgArc;
    float trackRadius = radius - 14.0f;
    bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, -maxAngle, maxAngle, true);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Tick marks: -50, -25, 0, +25, +50
    for (int ct = -50; ct <= 50; ct += 25) {
        float angle = (static_cast<float>(ct) / 50.0f) * maxAngle;
        float tickInner = trackRadius - 6.0f;
        float tickOuter = trackRadius + 6.0f;

        float x1 = centreX + std::sin(angle) * tickInner;
        float y1 = centreY - std::cos(angle) * tickInner;
        float x2 = centreX + std::sin(angle) * tickOuter;
        float y2 = centreY - std::cos(angle) * tickOuter;

        g.setColour(ct == 0 ? FreePitchLookAndFeel::kAccentAmber : FreePitchLookAndFeel::kTextMuted);
        g.drawLine(x1, y1, x2, y2, ct == 0 ? 2.0f : 1.0f);
    }

    // 3. Incoming Pitch Deviation Arc (Outer Track)
    // Range: -50 ct to +50 ct clamped
    float clampedDev = std::clamp(m_smoothCentsDev, -50.0f, 50.0f);
    float devAngle = (clampedDev / 50.0f) * maxAngle;

    if (std::abs(clampedDev) > 0.5f && m_smoothConfidence > 0.15f) {
        juce::Path devArc;
        if (devAngle < 0.0f) {
            devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, devAngle, 0.0f, true);
            g.setColour(FreePitchLookAndFeel::kIndicatorFlat.withAlpha(std::clamp(m_smoothConfidence, 0.2f, 1.0f)));
        } else {
            devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, 0.0f, devAngle, true);
            g.setColour(FreePitchLookAndFeel::kIndicatorSharp.withAlpha(std::clamp(m_smoothConfidence, 0.2f, 1.0f)));
        }
        g.strokePath(devArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Secondary Applied Correction Arc (Inner Track)
    float corrTrackRadius = trackRadius - 10.0f;
    float clampedCorr = std::clamp(m_smoothAppliedCorr, -50.0f, 50.0f);
    float corrAngle = (clampedCorr / 50.0f) * maxAngle;

    if (std::abs(clampedCorr) > 0.5f && m_smoothConfidence > 0.15f) {
        juce::Path corrArc;
        if (corrAngle < 0.0f) {
            corrArc.addCentredArc(centreX, centreY, corrTrackRadius, corrTrackRadius, 0.0f, corrAngle, 0.0f, true);
        } else {
            corrArc.addCentredArc(centreX, centreY, corrTrackRadius, corrTrackRadius, 0.0f, 0.0f, corrAngle, true);
        }
        g.setColour(FreePitchLookAndFeel::kAccentAmber.withAlpha(std::clamp(m_smoothConfidence * 0.9f, 0.2f, 0.9f)));
        g.strokePath(corrArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 5. In-Tune Indicator Pip at TDC (glowing green/amber when within +-3 cents)
    if (m_currentTelemetry.isVoiced && std::abs(m_currentTelemetry.centsDeviation) <= 3.0f) {
        g.setColour(FreePitchLookAndFeel::kIndicatorInTune);
        g.fillEllipse(centreX - 3.5f, centreY - trackRadius - 3.5f, 7.0f, 7.0f);
    }

    // 6. Central Readout Hub
    float hubRadius = radius * 0.52f;
    juce::Rectangle<float> hubBounds(centreX - hubRadius, centreY - hubRadius + 12.0f, hubRadius * 2.0f, hubRadius * 2.0f);
    g.setColour(FreePitchLookAndFeel::kBackgroundDark);
    g.fillEllipse(hubBounds);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawEllipse(hubBounds, 1.0f);

    // Large Detected Note Readout (e.g. "A4")
    juce::String noteStr = m_currentTelemetry.isVoiced ? midiToNoteName(m_currentTelemetry.detectedMidi) : "--";
    g.setColour(m_currentTelemetry.isVoiced ? FreePitchLookAndFeel::kTextWhite : FreePitchLookAndFeel::kTextDim);
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 36.0f, juce::Font::bold));
    g.drawText(noteStr, juce::Rectangle<float>(centreX - hubRadius, centreY - 14.0f, hubRadius * 2.0f, 38.0f), juce::Justification::centred);

    // Frequency & Cents in Tabular Monospace
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::plain));
    if (m_currentTelemetry.isVoiced) {
        juce::String freqStr = juce::String(m_currentTelemetry.detectedFreqHz, 1) + " Hz";
        g.setColour(FreePitchLookAndFeel::kTextMuted);
        g.drawText(freqStr, juce::Rectangle<float>(centreX - hubRadius, centreY + 24.0f, hubRadius * 2.0f, 16.0f), juce::Justification::centred);

        int centsVal = static_cast<int>(std::round(m_currentTelemetry.centsDeviation));
        juce::String centsStr = (centsVal > 0 ? "+" : "") + juce::String(centsVal) + " ct";
        juce::Colour centsCol = (std::abs(centsVal) <= 3) ? FreePitchLookAndFeel::kIndicatorInTune :
                                (centsVal < 0 ? FreePitchLookAndFeel::kIndicatorFlat : FreePitchLookAndFeel::kIndicatorSharp);
        g.setColour(centsCol);
        g.drawText(centsStr, juce::Rectangle<float>(centreX - hubRadius, centreY + 40.0f, hubRadius * 2.0f, 16.0f), juce::Justification::centred);
    } else {
        g.setColour(FreePitchLookAndFeel::kTextDim);
        g.drawText("UNVOICED", juce::Rectangle<float>(centreX - hubRadius, centreY + 26.0f, hubRadius * 2.0f, 16.0f), juce::Justification::centred);
    }
}
