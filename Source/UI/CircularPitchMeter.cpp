#include "CircularPitchMeter.h"
#include <cmath>
#include <algorithm>

CircularPitchMeter::CircularPitchMeter() {
    setTooltip("Live Pitch Radar: Real-time pitch detection, cent deviation, and musical scale targeting.");
}

void CircularPitchMeter::updateTelemetry(const PitchTelemetry& telemetry) {
    m_currentTelemetry = telemetry;

    float alpha = 0.28f;
    if (telemetry.isVoiced) {
        m_smoothCentsDev += alpha * (telemetry.centsDeviation - m_smoothCentsDev);
        m_smoothAppliedCorr += alpha * (telemetry.appliedCorrectionCents - m_smoothAppliedCorr);
        m_smoothConfidence += alpha * (telemetry.voicedConfidence - m_smoothConfidence);
        m_smoothFrequency += alpha * (telemetry.detectedFreqHz - m_smoothFrequency);
    } else {
        m_smoothCentsDev *= 0.82f;
        m_smoothAppliedCorr *= 0.82f;
        m_smoothConfidence *= 0.78f;
        m_smoothFrequency *= 0.85f;
    }

    repaint();
}

void CircularPitchMeter::resized() {
    renderCachedMeterBase();
}

void CircularPitchMeter::renderCachedMeterBase() {
    int w = getWidth();
    int h = getHeight();
    if (w <= 0 || h <= 0) return;

    m_cachedMeterBase = juce::Image(juce::Image::ARGB, w, h, true);
    juce::Graphics g(m_cachedMeterBase);

    using namespace FreePitchColors;

    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // 1. Deeply recessed circular well (#101215) with inner shadow bevels
    juce::Rectangle<float> dishBounds(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    FreePitchLookAndFeel::drawNeumorphicSunken(g, dishBounds, radius, 4.0f);

    // Subtle outer rim bevel
    g.setColour(rimHighlight.withAlpha(0.65f));
    g.drawEllipse(dishBounds.reduced(0.5f), 1.0f);

    // 2. Dial track groove and radial tick marks (-50 ct to +50 ct across a 210-degree sweep)
    float maxAngle = juce::MathConstants<float>::pi * 0.58f; // ~104 degrees each side of TDC
    float trackRadius = radius - 14.0f;

    if (trackRadius > 15.0f) {
        // Recessed track arc
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, -maxAngle, maxAngle, true);
        g.setColour(outerKnobTrack);
        g.strokePath(bgArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Radial Tick Marks
        for (int ct = -50; ct <= 50; ct += 5) {
            float frac = static_cast<float>(ct) / 50.0f;
            float angle = frac * maxAngle;
            bool isMajor = (ct % 25 == 0);
            bool isCenter = (ct == 0);

            float tickLen = isCenter ? 10.0f : (isMajor ? 7.0f : 4.0f);
            float innerR = trackRadius - 5.0f;
            float outerR = innerR + tickLen;

            float sinA = std::sin(angle);
            float cosA = -std::cos(angle);

            float x1 = centreX + sinA * innerR;
            float y1 = centreY + cosA * innerR;
            float x2 = centreX + sinA * outerR;
            float y2 = centreY + cosA * outerR;

            if (isCenter) {
                g.setColour(signalRed);
                g.drawLine(x1, y1, x2, y2, 2.0f);
                // Glowing pip at TDC
                g.setColour(signalRed.withAlpha(0.85f));
                g.fillEllipse(centreX - 2.0f, y1 - 4.0f, 4.0f, 4.0f);
            } else if (isMajor) {
                g.setColour(textSecondary);
                g.drawLine(x1, y1, x2, y2, 1.4f);
            } else {
                g.setColour(textDim.withAlpha(0.7f));
                g.drawLine(x1, y1, x2, y2, 0.9f);
            }
        }
    }
}

juce::String CircularPitchMeter::midiToNoteName(int midiNote) {
    if (midiNote < 0 || midiNote > 127) return "--";
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (midiNote / 12) - 1;
    int note = midiNote % 12;
    return juce::String(names[note]) + juce::String(octave);
}

void CircularPitchMeter::paint(juce::Graphics& g) {
    using namespace FreePitchColors;

    // 1. Draw cached heavy background (zero heap allocation)
    if (m_cachedMeterBase.isValid()) {
        g.drawImageAt(m_cachedMeterBase, 0, 0);
    }

    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float trackRadius = radius - 14.0f;
    float maxAngle = juce::MathConstants<float>::pi * 0.58f;

    bool isVoiced = m_currentTelemetry.isVoiced && (m_smoothConfidence > 0.15f);

    // 2. Animated Signal-Red Value Arc & Needle
    if (isVoiced && trackRadius > 15.0f) {
        float clampedDev = std::clamp(m_smoothCentsDev, -50.0f, 50.0f);
        float devAngle = (clampedDev / 50.0f) * maxAngle;

        if (std::abs(clampedDev) > 0.3f) {
            juce::Path devArc;
            if (devAngle < 0.0f) {
                devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, devAngle, 0.0f, true);
            } else {
                devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, 0.0f, devAngle, true);
            }

            // Soft outer bloom (0x44FF2E3B)
            g.setColour(signalRedBloom);
            g.strokePath(devArc, juce::PathStrokeType(7.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Sharp Signal Red tracking arc (#FF2E3B)
            g.setColour(signalRed);
            g.strokePath(devArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Tracking needle / marker
        float needleAngle = devAngle;
        float nSin = std::sin(needleAngle);
        float nCos = -std::cos(needleAngle);
        float nx1 = centreX + nSin * (trackRadius - 8.0f);
        float ny1 = centreY + nCos * (trackRadius - 8.0f);
        float nx2 = centreX + nSin * (trackRadius + 8.0f);
        float ny2 = centreY + nCos * (trackRadius + 8.0f);

        g.setColour(signalRed);
        g.drawLine(nx1, ny1, nx2, ny2, 2.5f);
    }

    // 3. Centered Bold Live Note Readout (e.g., A4)
    juce::String noteName = isVoiced ? midiToNoteName(m_currentTelemetry.targetMidi) : "--";

    float noteFontSize = std::clamp(radius * 0.44f, 24.0f, 48.0f);
    g.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), noteFontSize, juce::Font::bold));
    g.setColour(isVoiced ? textPrimary : textDim);

    auto noteArea = juce::Rectangle<float>(centreX - radius * 0.6f, centreY - radius * 0.34f,
                                           radius * 1.2f, noteFontSize + 4.0f);
    g.drawText(noteName, noteArea, juce::Justification::centred, false);

    // 4. Centered Live Cent Offset (e.g., -4 cents)
    juce::String centText;
    if (isVoiced) {
        int roundedCents = static_cast<int>(std::round(m_smoothCentsDev));
        centText = (roundedCents > 0 ? "+" : "") + juce::String(roundedCents) + " cents";
    } else {
        centText = "0 cents";
    }

    float centFontSize = std::clamp(radius * 0.15f, 10.0f, 15.0f);
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), centFontSize, juce::Font::plain));
    g.setColour(isVoiced ? textSecondary : textDim);

    auto centArea = juce::Rectangle<float>(centreX - radius * 0.6f, centreY + noteFontSize * 0.35f,
                                           radius * 1.2f, centFontSize + 4.0f);
    g.drawText(centText, centArea, juce::Justification::centred, false);
}
