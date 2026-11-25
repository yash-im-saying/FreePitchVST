#include "PianoKeyboardComponent.h"
#include <cmath>

PianoKeyboardComponent::PianoKeyboardComponent() {
    setTooltip("Interactive Piano: Click any key to toggle it in or out of the active scale. Highlights real-time detected vocal note.");
}

void PianoKeyboardComponent::setVocalMode(VocalMode mode) {
    m_vocalMode = mode;
    repaint();
}

void PianoKeyboardComponent::setScale(int keyRoot, ScaleType scaleType, uint32_t customMask) {
    m_keyRoot = keyRoot;
    m_scaleType = scaleType;
    m_customMask = customMask;
    repaint();
}

void PianoKeyboardComponent::updateLiveNotes(int detectedMidi, int targetMidi, bool isVoiced) {
    if (m_detectedMidi != detectedMidi || m_targetMidi != targetMidi || m_isVoiced != isVoiced) {
        m_detectedMidi = detectedMidi;
        m_targetMidi = targetMidi;
        m_isVoiced = isVoiced;
        repaint();
    }
}

bool PianoKeyboardComponent::isBlackKey(int midiNote) {
    int semitone = ((midiNote % 12) + 12) % 12;
    return (semitone == 1 || semitone == 3 || semitone == 6 || semitone == 8 || semitone == 10);
}

bool PianoKeyboardComponent::isNoteInScale(int midiNote) const {
    int noteInOctave = ((midiNote % 12) + 12) % 12;
    int relSemitone = ((noteInOctave - m_keyRoot) + 12) % 12;

    uint16_t mask = 0x0FFF;
    switch (m_scaleType) {
        case ScaleType::Chromatic: mask = 0x0FFF; break;
        case ScaleType::Major: mask = 0x0AB5; break; // 0,2,4,5,7,9,11
        case ScaleType::NaturalMinor: mask = 0x05AD; break; // 0,2,3,5,7,8,10
        case ScaleType::HarmonicMinor: mask = 0x09AD; break;
        case ScaleType::MelodicMinor: mask = 0x0AAD; break;
        case ScaleType::PentatonicMajor: mask = 0x02A5; break;
        case ScaleType::PentatonicMinor: mask = 0x0499; break;
        case ScaleType::Custom: mask = static_cast<uint16_t>(m_customMask); break;
        default: mask = 0x0FFF; break;
    }
    return (mask & (1 << relSemitone)) != 0;
}

void PianoKeyboardComponent::resized() {
}

void PianoKeyboardComponent::mouseDown(const juce::MouseEvent& e) {
    int midi = getMidiFromPosition(static_cast<float>(e.x), static_cast<float>(e.y));
    if (midi >= kStartMidi && midi <= kEndMidi) {
        int noteInOctave = ((midi % 12) + 12) % 12;
        int rel = ((noteInOctave - m_keyRoot) + 12) % 12;

        m_customMask ^= (1 << rel);
        if ((m_customMask & 0x0FFF) == 0) m_customMask = 0x0FFF;

        if (onCustomMaskChanged) {
            onCustomMaskChanged(m_customMask);
        }
        repaint();
    }
}

int PianoKeyboardComponent::getMidiFromPosition(float x, float y) const {
    int numWhiteKeys = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) numWhiteKeys++;
    }
    if (numWhiteKeys <= 0) return -1;

    float whiteKeyWidth = static_cast<float>(getWidth()) / static_cast<float>(numWhiteKeys);
    float blackKeyHeight = getHeight() * 0.62f;

    // Check black keys first
    if (y < blackKeyHeight) {
        int whiteIndex = 0;
        for (int m = kStartMidi; m <= kEndMidi; ++m) {
            if (isBlackKey(m)) {
                float blackCenterX = static_cast<float>(whiteIndex) * whiteKeyWidth;
                float blackWidth = whiteKeyWidth * 0.68f;
                if (x >= blackCenterX - blackWidth * 0.5f && x <= blackCenterX + blackWidth * 0.5f) {
                    return m;
                }
            } else {
                whiteIndex++;
            }
        }
    }

    // White keys
    int whiteIdx = static_cast<int>(x / whiteKeyWidth);
    int currentWhite = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) {
            if (currentWhite == whiteIdx) return m;
            currentWhite++;
        }
    }

    return -1;
}

void PianoKeyboardComponent::paint(juce::Graphics& g) {
    using namespace FreePitchColors;
    auto bounds = getLocalBounds().toFloat();

    // 1. Recessed keyboard trough (#101215)
    g.setColour(cavityRecessed);
    g.fillRect(bounds);

    // Beveled top inner shadow
    g.setColour(innerCavityDark);
    g.fillRect(bounds.removeFromTop(2.0f));

    bounds = getLocalBounds().toFloat().withTrimmedTop(2.0f);

    int minVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].minMidi;
    int maxVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].maxMidi;

    int numWhiteKeys = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) numWhiteKeys++;
    }
    float whiteKeyWidth = bounds.getWidth() / static_cast<float>(numWhiteKeys);
    float blackKeyHeight = bounds.getHeight() * 0.60f;
    float blackKeyWidth = whiteKeyWidth * 0.66f;

    // 2. Draw Crisp Soft-White Keys
    int whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) {
            float kx = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto keyRect = juce::Rectangle<float>(kx + 0.5f, bounds.getY(), whiteKeyWidth - 1.0f, bounds.getHeight() - 1.0f);

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            if (isDetected) {
                // Glowing Signal Red (#FF2E3B) with outer bloom
                g.setColour(signalRedBloom);
                g.fillRoundedRectangle(keyRect.expanded(2.0f), 2.0f);

                juce::ColourGradient grad(signalRed.brighter(0.15f), kx, bounds.getY(),
                                          signalRed, kx, bounds.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);
            } else if (isTarget) {
                juce::ColourGradient grad(signalRed.withAlpha(0.65f), kx, bounds.getY(),
                                          signalRed.withAlpha(0.40f), kx, bounds.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);
            } else {
                juce::Colour topCol = inVocalRange ? juce::Colour(0xFFF2F4F8) : juce::Colour(0xFFD4D8E0);
                juce::Colour botCol = inVocalRange ? juce::Colour(0xFFE0E5EC) : juce::Colour(0xFFBCC2CD);

                if (!inScale) {
                    topCol = topCol.darker(0.15f);
                    botCol = botCol.darker(0.15f);
                }

                juce::ColourGradient grad(topCol, kx, bounds.getY(), botCol, kx, bounds.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);
            }

            // Key outline
            g.setColour(cavityRecessed.withAlpha(0.5f));
            g.drawRoundedRectangle(keyRect, 2.0f, 0.8f);

            // Subtle scale-membership dot at bottom of in-scale white keys
            if (inScale && !isDetected && !isTarget) {
                float dotR = 2.0f;
                g.setColour(inVocalRange ? textSecondary : textDim);
                g.fillEllipse(kx + whiteKeyWidth * 0.5f - dotR, bounds.getBottom() - 10.0f, dotR * 2.0f, dotR * 2.0f);
            }

            whiteIdx++;
        }
    }

    // 3. Draw Matte Charcoal Black Keys
    whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (isBlackKey(m)) {
            float blackCenterX = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto keyRect = juce::Rectangle<float>(blackCenterX - blackKeyWidth * 0.5f, bounds.getY(),
                                                  blackKeyWidth, blackKeyHeight);

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            if (isDetected) {
                g.setColour(signalRedBloom);
                g.fillRoundedRectangle(keyRect.expanded(2.0f), 2.0f);

                juce::ColourGradient grad(signalRed.brighter(0.2f), blackCenterX, bounds.getY(),
                                          signalRed, blackCenterX, bounds.getY() + blackKeyHeight, false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);
            } else if (isTarget) {
                juce::ColourGradient grad(signalRed.withAlpha(0.70f), blackCenterX, bounds.getY(),
                                          signalRed.withAlpha(0.45f), blackCenterX, bounds.getY() + blackKeyHeight, false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);
            } else {
                juce::Colour topCol = inVocalRange ? juce::Colour(0xFF1E2127) : juce::Colour(0xFF14161A);
                juce::Colour botCol = inVocalRange ? juce::Colour(0xFF121418) : juce::Colour(0xFF0D0F12);

                if (!inScale) {
                    topCol = topCol.darker(0.12f);
                    botCol = botCol.darker(0.12f);
                }

                // Dual opposing shadow on black keys
                g.setColour(deepShadow.withAlpha(0.7f));
                g.fillRoundedRectangle(keyRect.translated(1.0f, 1.5f), 2.0f);

                juce::ColourGradient grad(topCol, blackCenterX, bounds.getY(), botCol, blackCenterX, bounds.getY() + blackKeyHeight, false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);

                // Top-left chamfer rim
                g.setColour(rimHighlight.withAlpha(0.5f));
                g.drawRoundedRectangle(keyRect, 2.0f, 0.8f);
            }

            // Subtle scale-membership dot on black keys
            if (inScale && !isDetected && !isTarget) {
                float dotR = 1.8f;
                g.setColour(inVocalRange ? textSecondary : textDim);
                g.fillEllipse(blackCenterX - dotR, bounds.getY() + blackKeyHeight - 8.0f, dotR * 2.0f, dotR * 2.0f);
            }
        } else {
            whiteIdx++;
        }
    }
}
