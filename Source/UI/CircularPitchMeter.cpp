#include "CircularPitchMeter.h"
#include <cmath>
#include <algorithm>

CircularPitchMeter::CircularPitchMeter() {
    setTooltip("Live Pitch Radar: Displays real-time pitch detection, cents deviation, in-tune status, and applied correction.");
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
        // Smoothly decay towards zero when unvoiced
        m_smoothCentsDev *= 0.82f;
        m_smoothAppliedCorr *= 0.82f;
        m_smoothConfidence *= 0.78f;
        m_smoothFrequency *= 0.85f;
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
    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    bool isVoiced = m_currentTelemetry.isVoiced && (m_smoothConfidence > 0.15f);
    const auto& p = FreePitchLookAndFeel::getPalette();

    // 1. Skeuomorphic Outer Bezel & Sunken Gauge Cavity
    // Top-left specular highlight shadow
    g.setColour(p.shadowLight);
    g.fillEllipse(centreX - radius - 2.5f, centreY - radius - 2.5f, (radius + 1.5f) * 2.0f, (radius + 1.5f) * 2.0f);

    // Bottom-right ambient shadow
    g.setColour(p.shadowDark);
    g.fillEllipse(centreX - radius + 3.0f, centreY - radius + 3.0f, (radius + 1.5f) * 2.0f, (radius + 1.5f) * 2.0f);

    // Sunken Gauge Cavity
    juce::Rectangle<float> cavityRect(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    g.setColour(p.panelSunken);
    g.fillEllipse(cavityRect);

    // Inner shadow of the gauge cavity
    juce::ColourGradient cavityInner(p.innerShadowDark, centreX - radius, centreY - radius,
                                     p.panelSunken, centreX, centreY, true);
    g.setGradientFill(cavityInner);
    g.drawEllipse(cavityRect.reduced(0.5f), 3.0f);

    // Hairline perimeter bezel
    g.setColour(p.panelBorder);
    g.drawEllipse(cavityRect, 1.0f);

    // 2. Pitch Dial Scale Markings (-50 ct to +50 ct across a 190-degree sweep)
    float maxAngle = juce::MathConstants<float>::pi * 0.55f; // ~99 degrees left & right of TDC
    float trackRadius = radius - 14.0f;

    // Background track arc (Recessed groove)
    if (trackRadius > 10.0f) {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, -maxAngle, maxAngle, true);
        g.setColour(p.panelBorder);
        g.strokePath(bgArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Tick Marks (-50, -25, 0, +25, +50)
    for (int ct = -50; ct <= 50; ct += 25) {
        float angle = (static_cast<float>(ct) / 50.0f) * maxAngle;
        float tickInner = trackRadius - 8.0f;
        float tickOuter = trackRadius + 7.0f;

        float x1 = centreX + std::sin(angle) * tickInner;
        float y1 = centreY - std::cos(angle) * tickInner;
        float x2 = centreX + std::sin(angle) * tickOuter;
        float y2 = centreY - std::cos(angle) * tickOuter;

        if (ct == 0) {
            // Zero cent marker (In-Tune Target)
            g.setColour(p.accentAmber);
            g.drawLine(x1, y1, x2, y2, 2.5f);

            // Glowing diamond pip at TDC
            float pipY = centreY - trackRadius - 9.0f;
            g.setColour(p.accentAmber.withAlpha(0.85f));
            g.fillEllipse(centreX - 2.5f, pipY - 2.5f, 5.0f, 5.0f);
        } else {
            g.setColour(p.textDim);
            g.drawLine(x1, y1, x2, y2, 1.2f);
        }
    }

    // Cent labels (-50, -25, 0, +25, +50)
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    for (int ct = -50; ct <= 50; ct += 25) {
        float angle = (static_cast<float>(ct) / 50.0f) * maxAngle;
        float labelR = trackRadius + 16.0f;
        float lx = centreX + std::sin(angle) * labelR;
        float ly = centreY - std::cos(angle) * labelR;

        juce::String str = (ct > 0 ? "+" : "") + juce::String(ct);
        g.setColour(ct == 0 ? p.accentAmber : p.textDim);
        g.drawText(str, static_cast<int>(lx - 15.0f), static_cast<int>(ly - 7.0f), 30, 14,
                   juce::Justification::centred);
    }

    // 3. Incoming Pitch Deviation Arc (Outer Track)
    float clampedDev = std::clamp(m_smoothCentsDev, -50.0f, 50.0f);
    float devAngle = (clampedDev / 50.0f) * maxAngle;

    if (std::abs(clampedDev) > 0.4f && isVoiced && trackRadius > 10.0f) {
        juce::Path devArc;
        juce::Colour arcColor = (devAngle < 0.0f) ? p.indicatorFlat : p.indicatorSharp;
        float alphaVal = std::clamp(m_smoothConfidence, 0.3f, 1.0f);

        if (devAngle < 0.0f) {
            devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, devAngle, 0.0f, true);
        } else {
            devArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, 0.0f, devAngle, true);
        }

        // Glowing outer bloom
        g.setColour(arcColor.withAlpha(0.35f * alphaVal));
        g.strokePath(devArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(arcColor.withAlpha(alphaVal));
        g.strokePath(devArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Applied Correction Arc (Inner Track)
    float corrRadius = trackRadius - 9.0f;
    float clampedCorr = std::clamp(m_smoothAppliedCorr, -50.0f, 50.0f);
    float corrAngle = (clampedCorr / 50.0f) * maxAngle;

    if (std::abs(clampedCorr) > 0.4f && isVoiced && corrRadius > 10.0f) {
        juce::Path corrArc;
        if (corrAngle < 0.0f) {
            corrArc.addCentredArc(centreX, centreY, corrRadius, corrRadius, 0.0f, corrAngle, 0.0f, true);
        } else {
            corrArc.addCentredArc(centreX, centreY, corrRadius, corrRadius, 0.0f, 0.0f, corrAngle, true);
        }
        g.setColour(p.accentAmber.withAlpha(0.90f));
        g.strokePath(corrArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 5. Central Raised Note Display Hardware Hub
    float centerDiscRadius = radius * 0.54f;
    juce::Rectangle<float> centerDisc(centreX - centerDiscRadius, centreY - centerDiscRadius,
                                      centerDiscRadius * 2.0f, centerDiscRadius * 2.0f);

    // Glowing Voiced Phosphor Aura
    if (isVoiced) {
        float auraAlpha = std::clamp(m_smoothConfidence * 0.35f, 0.05f, 0.35f);
        juce::Colour auraColor = (std::abs(m_smoothCentsDev) <= 5.0f)
                                 ? p.indicatorInTune
                                 : p.accentCyan;
        g.setColour(auraColor.withAlpha(auraAlpha));
        g.fillEllipse(centerDisc.expanded(8.0f));
    }

    // Raised hub drop shadow
    g.setColour(p.shadowDark);
    g.fillEllipse(centerDisc.translated(0.0f, 2.5f));

    // Metallic Hub Gradient (Machined Aluminum / Gunmetal)
    juce::ColourGradient discGrad(p.knobCapTop, centreX, centerDisc.getY(),
                                  p.knobCapBottom, centreX, centerDisc.getBottom(), false);
    g.setGradientFill(discGrad);
    g.fillEllipse(centerDisc);

    // Disc border (highlighted if in tune)
    if (isVoiced && std::abs(m_smoothCentsDev) <= 5.0f) {
        g.setColour(p.indicatorInTune.withAlpha(0.85f));
        g.drawEllipse(centerDisc, 1.8f);
    } else {
        g.setColour(p.knobRim);
        g.drawEllipse(centerDisc, 1.2f);
    }

    // Concentric machined hairline on hub
    float innerHubRing = centerDiscRadius * 0.88f;
    g.setColour(p.panelBorder.withAlpha(0.4f));
    g.drawEllipse(centreX - innerHubRing, centreY - innerHubRing, innerHubRing * 2.0f, innerHubRing * 2.0f, 0.8f);

    // 6. Center Hero Note Display
    juce::String noteName = isVoiced ? midiToNoteName(m_currentTelemetry.targetMidi) : "--";

    g.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), centerDiscRadius * 0.70f, juce::Font::bold));
    if (isVoiced) {
        if (std::abs(m_smoothCentsDev) <= 5.0f) {
            g.setColour(p.indicatorInTune);
        } else {
            g.setColour(p.textPrimary);
        }
    } else {
        g.setColour(p.textDim);
    }

    float noteY = centreY - centerDiscRadius * 0.48f;
    g.drawText(noteName, static_cast<int>(centreX - centerDiscRadius), static_cast<int>(noteY),
               static_cast<int>(centerDiscRadius * 2.0f), static_cast<int>(centerDiscRadius * 0.65f),
               juce::Justification::centred);

    // 7. Cents Deviation Pill Badge (Skeuomorphic Sunken Status Screen)
    float pillW = centerDiscRadius * 1.15f;
    float pillH = 18.0f;
    float pillX = centreX - pillW * 0.5f;
    float pillY = centreY + centerDiscRadius * 0.16f;
    juce::Rectangle<float> pillBounds(pillX, pillY, pillW, pillH);

    FreePitchLookAndFeel::drawNeumorphicSunken(g, pillBounds, 9.0f, 2.0f);

    juce::String pillText;
    juce::Colour pillTextCol;

    if (isVoiced) {
        float absDev = std::abs(m_smoothCentsDev);
        if (absDev <= 4.0f) {
            pillText = "IN TUNE";
            pillTextCol = p.indicatorInTune;
        } else {
            int devInt = static_cast<int>(std::round(m_smoothCentsDev));
            pillText = (devInt > 0 ? "+" : "") + juce::String(devInt) + " ct";
            pillTextCol = (devInt < 0) ? p.indicatorFlat : p.indicatorSharp;
        }
    } else {
        pillText = "UNVOICED";
        pillTextCol = p.textDim;
    }

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.setColour(pillTextCol);
    g.drawText(pillText, pillBounds.toNearestInt(), juce::Justification::centred);

    // 8. Hz Readout on bottom
    if (isVoiced && m_smoothFrequency > 20.0f) {
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
        g.setColour(p.textMuted);
        juce::String hzStr = juce::String(m_smoothFrequency, 1) + " Hz";
        g.drawText(hzStr, static_cast<int>(centreX - 50.0f), static_cast<int>(pillY + pillH + 2.0f), 100, 14,
                   juce::Justification::centred);
    }

    // 9. Convex Domed Glass Specular Glare Reflection (Signature Skeuomorphism)
    juce::Rectangle<float> glassArea(centreX - radius + 2.0f, centreY - radius + 2.0f,
                                     (radius - 2.0f) * 2.0f, (radius - 2.0f) * 2.0f);
    FreePitchLookAndFeel::drawGlassGlare(g, glassArea, radius);
}
