#include "PianoKeyboardComponent.h"

PianoKeyboardComponent::PianoKeyboardComponent() {
    setTooltip("Piano Keyboard: Displays active scale notes and tracks sung pitch in real time. Click any key to toggle it in or out of the scale.");
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
        case ScaleType::PentatonicMajor: mask = 0x02A5; break; // 0,2,4,7,9
        case ScaleType::PentatonicMinor: mask = 0x0499; break; // 0,3,5,7,10
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

        // Toggle in custom mask
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

    // Check black keys first if y is in top 62%
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

    // Otherwise check white keys
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
    auto bounds = getLocalBounds().toFloat();
    const auto& p = FreePitchLookAndFeel::getPalette();
    bool isDarkTheme = FreePitchLookAndFeel::isDark();

    int minVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].minMidi;
    int maxVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].maxMidi;

    int numWhiteKeys = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) numWhiteKeys++;
    }
    float whiteKeyWidth = bounds.getWidth() / static_cast<float>(numWhiteKeys);
    float blackKeyHeight = bounds.getHeight() * 0.62f;
    float blackKeyWidth = whiteKeyWidth * 0.68f;

    // 1. Draw Ivory White Keys
    int whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) {
            float kx = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto keyRect = juce::Rectangle<float>(kx, 0.0f, whiteKeyWidth, bounds.getHeight());

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            juce::Colour topCol;
            juce::Colour botCol;

            if (isDarkTheme) {
                topCol = juce::Colour(0xFF282D38);
                botCol = juce::Colour(0xFF1D2128);
                if (!inVocalRange) {
                    topCol = juce::Colour(0xFF16191F);
                    botCol = juce::Colour(0xFF101216);
                } else if (!inScale) {
                    topCol = juce::Colour(0xFF20242D);
                    botCol = juce::Colour(0xFF171A20);
                }
            } else {
                topCol = juce::Colour(0xFFFFFFFF);
                botCol = juce::Colour(0xFFE8EDF4);
                if (!inVocalRange) {
                    topCol = juce::Colour(0xFFD3D9E2);
                    botCol = juce::Colour(0xFFC4CBD6);
                } else if (!inScale) {
                    topCol = juce::Colour(0xFFBAC5D3);
                    botCol = juce::Colour(0xFFAAB6C6);
                }
            }

            if (isTarget) {
                topCol = p.accentAmber;
                botCol = p.accentAmber.darker(0.25f);
            } else if (isDetected) {
                topCol = p.accentCyan;
                botCol = p.accentCyan.darker(0.25f);
            }

            juce::ColourGradient grad(topCol, kx, 0.0f, botCol, kx, bounds.getHeight(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(keyRect, 2.0f);

            // Active scale note indicator dot at bottom of in-scale white keys
            if (inScale && inVocalRange && !isTarget && !isDetected) {
                float dotRadius = 2.2f;
                g.setColour(p.accentCyan.withAlpha(0.75f));
                g.fillEllipse(kx + whiteKeyWidth * 0.5f - dotRadius, bounds.getHeight() - 16.0f,
                              dotRadius * 2.0f, dotRadius * 2.0f);
            }

            // Hairline separator between keys
            g.setColour(p.bgBase);
            g.drawRect(keyRect, 0.8f);

            // Print Note Name (e.g. C1, C2, C3, C4, C5, C6) on C keys
            if (m % 12 == 0) {
                int oct = (m / 12) - 1;
                g.setColour(isTarget || isDetected ? juce::Colours::black : p.textDim);
                g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
                g.drawText("C" + juce::String(oct), keyRect.removeFromBottom(16.0f), juce::Justification::centred);
            }

            whiteIdx++;
        }
    }

    // 2. Draw Ebony Black Keys
    whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (isBlackKey(m)) {
            float blackCenterX = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto blackRect = juce::Rectangle<float>(blackCenterX - blackKeyWidth * 0.5f, 0.0f, blackKeyWidth, blackKeyHeight);

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            // Skeuomorphic drop shadow cast by raised black key onto white keys below
            g.setColour(p.shadowDark);
            g.fillRoundedRectangle(blackRect.translated(1.5f, 2.5f), 2.5f);

            juce::Colour topCol = isDarkTheme ? juce::Colour(0xFF262C37) : juce::Colour(0xFF333D4B);
            juce::Colour botCol = isDarkTheme ? juce::Colour(0xFF11141A) : juce::Colour(0xFF1E242C);

            if (!inVocalRange) {
                topCol = isDarkTheme ? juce::Colour(0xFF14171E) : juce::Colour(0xFF202630);
                botCol = isDarkTheme ? juce::Colour(0xFF0A0C0F) : juce::Colour(0xFF14181E);
            } else if (!inScale) {
                topCol = isDarkTheme ? juce::Colour(0xFF1D222B) : juce::Colour(0xFF272F3A);
                botCol = isDarkTheme ? juce::Colour(0xFF0F1116) : juce::Colour(0xFF181C22);
            }

            if (isTarget) {
                topCol = p.accentAmber;
                botCol = p.accentAmber.darker(0.35f);
            } else if (isDetected) {
                topCol = p.accentCyan;
                botCol = p.accentCyan.darker(0.35f);
            }

            juce::ColourGradient blackGrad(topCol, blackRect.getX(), 0.0f, botCol, blackRect.getX(), blackKeyHeight, false);
            g.setGradientFill(blackGrad);
            g.fillRoundedRectangle(blackRect.reduced(0.5f, 0.0f), 2.5f);

            // Active scale note dot on black keys
            if (inScale && inVocalRange && !isTarget && !isDetected) {
                float dotRadius = 2.0f;
                g.setColour(p.accentCyan.withAlpha(0.75f));
                g.fillEllipse(blackCenterX - dotRadius, blackKeyHeight - 10.0f,
                              dotRadius * 2.0f, dotRadius * 2.0f);
            }

            g.setColour(p.bgBase);
            g.drawRoundedRectangle(blackRect.reduced(0.5f, 0.0f), 2.0f, 1.0f);
        } else {
            whiteIdx++;
        }
    }

    // 3. Crisp Vocal Range Bounds Bracket across top of keyboard
    float rangeX1 = 0.0f;
    float rangeX2 = bounds.getWidth();
    whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) {
            if (m == minVocalMidi || (m > minVocalMidi && rangeX1 == 0.0f)) {
                rangeX1 = static_cast<float>(whiteIdx) * whiteKeyWidth;
            }
            if (m == maxVocalMidi || m < maxVocalMidi) {
                rangeX2 = static_cast<float>(whiteIdx + 1) * whiteKeyWidth;
            }
            whiteIdx++;
        }
    }

    g.setColour(p.accentAmber);
    g.drawHorizontalLine(1, rangeX1, rangeX2);
    g.drawVerticalLine(static_cast<int>(rangeX1), 0.0f, 7.0f);
    g.drawVerticalLine(static_cast<int>(rangeX2), 0.0f, 7.0f);
}
