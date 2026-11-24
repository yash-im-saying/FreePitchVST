#include "ScaleQuantizer.h"
#include <cmath>
#include <algorithm>

ScaleQuantizer::ScaleQuantizer() {
    for (int i = 0; i < kHistorySize; ++i) {
        m_pitchHistory[i] = 60.0f;
    }
}

void ScaleQuantizer::prepare(double sampleRate) {
    m_sampleRate = (sampleRate > 8000.0) ? sampleRate : 48000.0;
    reset();
}

void ScaleQuantizer::reset() {
    m_currentMidiPitch = 60.0f;
    m_currentTargetMidi = -1;
    m_timeOnCurrentNoteSec = 0.0f;
    m_wasVoiced = false;
    m_historyIndex = 0;
    m_historyCount = 0;
    m_sustainedTimeSec = 0.0f;
    for (int i = 0; i < kHistorySize; ++i) {
        m_pitchHistory[i] = 60.0f;
    }
}

void ScaleQuantizer::setKeyRoot(int rootIndex) {
    m_keyRoot = (rootIndex >= 0 && rootIndex < 12) ? rootIndex : 0;
}

void ScaleQuantizer::setScaleType(ScaleType scale) {
    if (scale < ScaleType::Chromatic || scale >= ScaleType::NumScales)
        scale = ScaleType::Chromatic;
    m_scaleType = scale;
}

void ScaleQuantizer::setCustomMask(uint32_t mask12Bit) {
    m_customMask = mask12Bit & 0x0FFF;
    if (m_customMask == 0) {
        m_customMask = 0x0FFF; // fallback to chromatic if empty
    }
}

void ScaleQuantizer::setRetuneSpeedMs(float speedMs) {
    m_retuneSpeedMs = std::clamp(speedMs, 0.0f, 400.0f);
}

void ScaleQuantizer::setHumanize(float humanizePercent) {
    m_humanize = std::clamp(humanizePercent, 0.0f, 100.0f);
}

void ScaleQuantizer::setFlexTune(float flexTunePercent) {
    m_flexTune = std::clamp(flexTunePercent, 0.0f, 100.0f);
}

void ScaleQuantizer::setPitchShift(float semitones) {
    m_pitchShift = std::clamp(semitones, -12.0f, 12.0f);
}

uint16_t ScaleQuantizer::getScaleBitmask() const {
    // Return a 12-bit mask where bit 0 is Root, bit 1 is Root+1, etc.
    switch (m_scaleType) {
        case ScaleType::Chromatic:
            return 0x0FFF; // All 12 notes
        case ScaleType::Major:
            // 0, 2, 4, 5, 7, 9, 11
            return (1 << 0) | (1 << 2) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11);
        case ScaleType::NaturalMinor:
            // 0, 2, 3, 5, 7, 8, 10
            return (1 << 0) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 10);
        case ScaleType::HarmonicMinor:
            // 0, 2, 3, 5, 7, 8, 11
            return (1 << 0) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 11);
        case ScaleType::MelodicMinor:
            // 0, 2, 3, 5, 7, 9, 11
            return (1 << 0) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 11);
        case ScaleType::PentatonicMajor:
            // 0, 2, 4, 7, 9
            return (1 << 0) | (1 << 2) | (1 << 4) | (1 << 7) | (1 << 9);
        case ScaleType::PentatonicMinor:
            // 0, 3, 5, 7, 10
            return (1 << 0) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 10);
        case ScaleType::Custom:
            return static_cast<uint16_t>(m_customMask);
        default:
            return 0x0FFF;
    }
}

bool ScaleQuantizer::isNoteInScale(int midiNote) const {
    int noteInOctave = ((midiNote % 12) + 12) % 12;
    // Semitone relative to KeyRoot
    int relSemitone = ((noteInOctave - m_keyRoot) + 12) % 12;
    uint16_t mask = getScaleBitmask();
    return (mask & (1 << relSemitone)) != 0;
}

int ScaleQuantizer::findNearestScaleMidi(float continuousMidi) const {
    int roundMidi = static_cast<int>(std::round(continuousMidi));
    if (isNoteInScale(roundMidi)) {
        return roundMidi;
    }

    // Search outward for the nearest note in scale
    for (int offset = 1; offset <= 6; ++offset) {
        int lower = roundMidi - offset;
        int upper = roundMidi + offset;

        float distLower = std::abs(continuousMidi - static_cast<float>(lower));
        float distUpper = std::abs(continuousMidi - static_cast<float>(upper));

        bool lowerValid = isNoteInScale(lower);
        bool upperValid = isNoteInScale(upper);

        if (lowerValid && upperValid) {
            return (distLower <= distUpper) ? lower : upper;
        } else if (lowerValid) {
            return lower;
        } else if (upperValid) {
            return upper;
        }
    }

    return roundMidi;
}

float ScaleQuantizer::processTuning(float detectedFreqHz, bool isVoiced, float dtSeconds,
                                    float& outCentsDev, float& outAppliedCorrectionCents,
                                    int& outDetectedMidi, int& outTargetMidi) {
    if (!isVoiced || detectedFreqHz < 20.0f) {
        m_wasVoiced = false;
        m_sustainedTimeSec = 0.0f;
        outCentsDev = 0.0f;
        outAppliedCorrectionCents = 0.0f;
        outDetectedMidi = -1;
        outTargetMidi = -1;
        return detectedFreqHz;
    }

    // 1. Continuous MIDI Pitch: 69 + 12 * log2(f / 440)
    float detectedMidi = 69.0f + 12.0f * (std::log(detectedFreqHz / 440.0f) / 0.69314718056f);
    outDetectedMidi = static_cast<int>(std::round(detectedMidi));

    // 2. Find Nearest Scale Target Note with Hysteresis
    int rawTargetMidi = findNearestScaleMidi(detectedMidi);
    int targetMidi = rawTargetMidi;

    // Apply note-boundary hysteresis if already voiced and on a scale note
    if (m_wasVoiced && m_currentTargetMidi >= 0 && isNoteInScale(m_currentTargetMidi)) {
        float distToCurrent = std::abs(detectedMidi - static_cast<float>(m_currentTargetMidi));
        float distToNew = std::abs(detectedMidi - static_cast<float>(rawTargetMidi));
        // Only switch away from current note if new note is definitively closer by > 12 cents
        // or current deviation exceeds 60 cents
        if (distToCurrent < 0.60f && (distToCurrent <= distToNew + 0.12f)) {
            targetMidi = m_currentTargetMidi;
        }
    }

    // Refresh onset timer on every new note
    bool isNewNote = (!m_wasVoiced || targetMidi != m_currentTargetMidi);
    if (isNewNote) {
        m_currentTargetMidi = targetMidi;
        m_timeOnCurrentNoteSec = 0.0f;
    } else {
        m_timeOnCurrentNoteSec += dtSeconds;
    }
    outTargetMidi = targetMidi;

    // Raw cents deviation between input and uncorrected target note center
    float rawCentsDelta = (detectedMidi - static_cast<float>(targetMidi)) * 100.0f;
    outCentsDev = rawCentsDelta;

    // 3. Track Sustained Stability for Humanize
    m_pitchHistory[m_historyIndex] = detectedMidi;
    m_historyIndex = (m_historyIndex + 1) % kHistorySize;
    if (m_historyCount < kHistorySize) m_historyCount++;

    m_sustainedTimeSec += dtSeconds;

    float meanPitch = 0.0f;
    for (int i = 0; i < m_historyCount; ++i) {
        meanPitch += m_pitchHistory[i];
    }
    meanPitch /= static_cast<float>(m_historyCount);

    float variance = 0.0f;
    for (int i = 0; i < m_historyCount; ++i) {
        float diff = m_pitchHistory[i] - meanPitch;
        variance += diff * diff;
    }
    variance /= static_cast<float>(m_historyCount);
    float stdDevCents = std::sqrt(variance) * 100.0f;

    // Stability factor S in [0, 1]: high when sustained note pitch is steady (vibrato window ~ 20-50 cents)
    // S ramps up after holding note for > 80ms
    float timeRamp = std::clamp((m_sustainedTimeSec - 0.08f) / 0.15f, 0.0f, 1.0f);
    float stability = std::clamp(1.0f - (stdDevCents / 80.0f), 0.0f, 1.0f) * timeRamp;

    // Humanize relaxation: relaxes full correction by up to 75% on sustained notes
    float humanizeRelaxation = (m_humanize / 100.0f) * stability * 0.70f;

    // 4. Flex-Tune: Elastic Dead-Zone around target note center
    // Dead-zone radius up to 35 cents at 100% Flex-Tune
    float deadZoneRadius = (m_flexTune / 100.0f) * 35.0f;
    float absDelta = std::abs(rawCentsDelta);
    float targetCentsDelta = 0.0f;

    if (deadZoneRadius > 0.5f) {
        if (absDelta <= deadZoneRadius) {
            // Inside dead-zone: intentional micro-inflection / scoop / slide untouched!
            targetCentsDelta = rawCentsDelta;
        } else {
            // Outside dead-zone: smoothly pull back towards center!
            float t = (absDelta - deadZoneRadius) / (50.0f - deadZoneRadius + 1e-4f);
            t = std::clamp(t, 0.0f, 1.0f);
            float smoothT = t * t * (3.0f - 2.0f * t); // Smooth Hermite curve
            targetCentsDelta = rawCentsDelta * (1.0f - smoothT);
        }
    } else {
        targetCentsDelta = 0.0f; // Perfect center tuning
    }

    // Blend in Humanize relaxation
    targetCentsDelta = targetCentsDelta * (1.0f - humanizeRelaxation) + rawCentsDelta * humanizeRelaxation;

    // Compute ideal target continuous MIDI pitch
    float desiredTargetMidi = static_cast<float>(targetMidi) + (targetCentsDelta / 100.0f);

    // 5. Retune Speed: Correction kicks in over retuneSpeedMs and refreshes on every new note
    float fullCorrectionDelta = desiredTargetMidi - detectedMidi;

    float correctionRamp = 1.0f;
    if (m_retuneSpeedMs > 0.5f) {
        float retuneSec = m_retuneSpeedMs * 0.001f;
        // Exponential ramp reaching >95% at retuneSpeedMs
        float tau = retuneSec * 0.33f;
        correctionRamp = 1.0f - std::exp(-m_timeOnCurrentNoteSec / tau);
        correctionRamp = std::clamp(correctionRamp, 0.0f, 1.0f);
    }

    float appliedDelta = correctionRamp * fullCorrectionDelta;
    m_currentMidiPitch = detectedMidi + appliedDelta;

    m_wasVoiced = true;

    // 6. Apply Continuous Pitch Shift
    float finalMidiPitch = m_currentMidiPitch + m_pitchShift;

    // Calculate applied correction relative to raw input
    outAppliedCorrectionCents = (finalMidiPitch - detectedMidi) * 100.0f;

    // Convert back to target frequency Hz: f = 440 * 2^((midi - 69) / 12)
    float targetFreqHz = 440.0f * std::exp2((finalMidiPitch - 69.0f) / 12.0f);
    return targetFreqHz;
}
