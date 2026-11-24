#include "PianoKeyboardComponent.h"

PianoKeyboardComponent::PianoKeyboardComponent() {
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
    float blackKeyHeight = getHeight() * 0.60f;

    // Check black keys first if y is in top 60%
    if (y < blackKeyHeight) {
        int whiteIndex = 0;
        for (int m = kStartMidi; m <= kEndMidi; ++m) {
            if (isBlackKey(m)) {
                float blackCenterX = static_cast<float>(whiteIndex) * whiteKeyWidth;
                float blackWidth = whiteKeyWidth * 0.65f;
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
    int minVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].minMidi;
    int maxVocalMidi = g_vocalModeBounds[static_cast<int>(m_vocalMode)].maxMidi;

    int numWhiteKeys = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) numWhiteKeys++;
    }
    float whiteKeyWidth = bounds.getWidth() / static_cast<float>(numWhiteKeys);
    float blackKeyHeight = bounds.getHeight() * 0.60f;
    float blackKeyWidth = whiteKeyWidth * 0.65f;

    // 1. Draw White Keys
    int whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (!isBlackKey(m)) {
            float kx = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto keyRect = juce::Rectangle<float>(kx, 0.0f, whiteKeyWidth, bounds.getHeight());

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            juce::Colour keyColor = juce::Colour(0xFFDCE0E6); // Clean ivory off-white
            if (!inVocalRange) {
                keyColor = juce::Colour(0xFF4A4E57); // Dimmed outside range
            } else if (!inScale) {
                keyColor = juce::Colour(0xFF8B929E); // Muted if excluded from scale
            }

            if (isTarget) {
                keyColor = FreePitchLookAndFeel::kAccentAmber; // Glowing target note
            } else if (isDetected) {
                keyColor = FreePitchLookAndFeel::kIndicatorFlat; // Detected note
            }

            g.setColour(keyColor);
            g.fillRect(keyRect);

            // 1px hairline border between keys
            g.setColour(FreePitchLookAndFeel::kBackgroundDark);
            g.drawRect(keyRect, 1.0f);

            // Print Note Name (e.g. C2, C3, C4) on C notes
            if (m % 12 == 0) {
                int oct = (m / 12) - 1;
                g.setColour(FreePitchLookAndFeel::kBackgroundDark);
                g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
                g.drawText("C" + juce::String(oct), keyRect.removeFromBottom(12.0f), juce::Justification::centred);
            }

            whiteIdx++;
        }
    }

    // 2. Draw Black Keys
    whiteIdx = 0;
    for (int m = kStartMidi; m <= kEndMidi; ++m) {
        if (isBlackKey(m)) {
            float blackCenterX = static_cast<float>(whiteIdx) * whiteKeyWidth;
            auto blackRect = juce::Rectangle<float>(blackCenterX - blackKeyWidth * 0.5f, 0.0f, blackKeyWidth, blackKeyHeight);

            bool inVocalRange = (m >= minVocalMidi && m <= maxVocalMidi);
            bool inScale = isNoteInScale(m);
            bool isDetected = (m_isVoiced && m == m_detectedMidi);
            bool isTarget = (m_isVoiced && m == m_targetMidi);

            juce::Colour blackColor = juce::Colour(0xFF1D2025); // Matte graphite
            if (!inVocalRange) {
                blackColor = juce::Colour(0xFF141619);
            } else if (!inScale) {
                blackColor = juce::Colour(0xFF2C313A);
            }

            if (isTarget) {
                blackColor = FreePitchLookAndFeel::kAccentAmber;
            } else if (isDetected) {
                blackColor = FreePitchLookAndFeel::kIndicatorFlat;
            }

            g.setColour(blackColor);
            g.fillRect(blackRect);

            g.setColour(FreePitchLookAndFeel::kBackgroundDark);
            g.drawRect(blackRect, 1.0f);
        } else {
            whiteIdx++;
        }
    }

    // 3. Crisp Range Brackets for active Vocal Range Mode
    // Find left coordinate of minVocalMidi and right coordinate of maxVocalMidi
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

    // Hairline range bracket across top of keyboard
    g.setColour(FreePitchLookAndFeel::kAccentAmber);
    g.drawHorizontalLine(1, rangeX1, rangeX2);
    g.drawVerticalLine(static_cast<int>(rangeX1), 0.0f, 6.0f);
    g.drawVerticalLine(static_cast<int>(rangeX2), 0.0f, 6.0f);
}
