#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

namespace FreePitchID {
    // Tuning & Vocal Parameters
    inline constexpr const char* vocalMode     = "vocalMode";
    inline constexpr const char* keyRoot       = "keyRoot";
    inline constexpr const char* scaleType     = "scaleType";
    inline constexpr const char* retuneSpeed   = "retuneSpeed";
    inline constexpr const char* humanize      = "humanize";
    inline constexpr const char* flexTune      = "flexTune";
    inline constexpr const char* choosyTracking = "choosyTracking";
    inline constexpr const char* pitchShift    = "pitchShift";
    inline constexpr const char* formantsOn    = "formantsOn";
    inline constexpr const char* formantShift  = "formantShift";
    inline constexpr const char* lookaheadMode = "lookaheadMode";
    inline constexpr const char* masterEfxActive = "masterEfxActive";

    // EQ
    inline constexpr const char* eqEnabled     = "eqEnabled";
    inline constexpr const char* eqHpfFreq     = "eqHpfFreq";
    inline constexpr const char* eqLowMidGain  = "eqLowMidGain";
    inline constexpr const char* eqLowMidFreq  = "eqLowMidFreq";
    inline constexpr const char* eqHighMidGain = "eqHighMidGain";
    inline constexpr const char* eqHighMidFreq = "eqHighMidFreq";
    inline constexpr const char* eqAirGain     = "eqAirGain";
    inline constexpr const char* eqAirFreq     = "eqAirFreq";
    inline constexpr const char* eqOutGain     = "eqOutGain";

    // Compressor
    inline constexpr const char* compEnabled   = "compEnabled";
    inline constexpr const char* compThreshold = "compThreshold";
    inline constexpr const char* compRatio     = "compRatio";
    inline constexpr const char* compAttack    = "compAttack";
    inline constexpr const char* compRelease   = "compRelease";
    inline constexpr const char* compMakeup    = "compMakeup";

    // Flanger
    inline constexpr const char* flangerEnabled  = "flangerEnabled";
    inline constexpr const char* flangerRate     = "flangerRate";
    inline constexpr const char* flangerDepth    = "flangerDepth";
    inline constexpr const char* flangerFeedback = "flangerFeedback";
    inline constexpr const char* flangerMix      = "flangerMix";

    // Chorus / Haas
    inline constexpr const char* chorusEnabled   = "chorusEnabled";
    inline constexpr const char* chorusMode      = "chorusMode";
    inline constexpr const char* chorusRate      = "chorusRate";
    inline constexpr const char* chorusDepth     = "chorusDepth";
    inline constexpr const char* haasDelay       = "haasDelay";
    inline constexpr const char* haasWidth       = "haasWidth";
    inline constexpr const char* chorusMix       = "chorusMix";

    // Distortion
    inline constexpr const char* distEnabled     = "distEnabled";
    inline constexpr const char* distMode        = "distMode";
    inline constexpr const char* distDrive       = "distDrive";
    inline constexpr const char* distTone        = "distTone";
    inline constexpr const char* distMix         = "distMix";

    // Delay / Echo (H-Delay, Supermassive, FreqEcho)
    inline constexpr const char* delayEnabled    = "delayEnabled";
    inline constexpr const char* delayMode       = "delayMode";
    inline constexpr const char* delayTime       = "delayTime";
    inline constexpr const char* delayFeedback   = "delayFeedback";
    inline constexpr const char* delayFreqShift  = "delayFreqShift";
    inline constexpr const char* delayDiffusion  = "delayDiffusion";
    inline constexpr const char* delayTone       = "delayTone";
    inline constexpr const char* delayMix        = "delayMix";
}

enum class VocalMode {
    Soprano = 0,
    MezzoSoprano,
    Alto,
    Tenor,
    Baritone,
    LowMale,
    Bass,
    FullRange,
    NumModes
};

struct VocalModeBounds {
    float minHz;
    float maxHz;
    int minMidi;
    int maxMidi;
    const char* name;
};

inline const VocalModeBounds g_vocalModeBounds[] = {
    { 261.63f, 1046.50f, 60, 84, "Soprano (C4-C6)" },
    { 220.00f,  880.00f, 57, 81, "Mezzo-Soprano (A3-A5)" },
    { 174.61f,  698.46f, 53, 77, "Alto (F3-F5)" },
    { 130.81f,  523.25f, 48, 72, "Tenor (C3-C5)" },
    {  98.00f,  392.00f, 43, 67, "Baritone (G2-G4)" },
    {  82.41f,  329.63f, 40, 64, "Low Male (E2-E4)" },
    {  41.20f,  164.81f, 28, 52, "Bass / Instrument (E1-E3)" },
    {  32.70f, 1046.50f, 24, 84, "Chromatic / Full Range (C1-C6)" }
};

enum class ScaleType {
    Chromatic = 0,
    Major,
    NaturalMinor,
    HarmonicMinor,
    MelodicMinor,
    PentatonicMajor,
    PentatonicMinor,
    Custom,
    NumScales
};

struct PitchTelemetry {
    float detectedFreqHz = 0.0f;
    float targetFreqHz = 0.0f;
    int detectedMidi = -1;
    int targetMidi = -1;
    float centsDeviation = 0.0f;
    float appliedCorrectionCents = 0.0f;
    float voicedConfidence = 0.0f;
    bool isVoiced = false;
    float compressorGR_dB = 0.0f;
    bool haasPhaseWarning = false;
    float inputRmsDb = -100.0f;
    float outputRmsDb = -100.0f;
    uint32_t customScaleMask = 0xFFF; // 12-bit chromatic mask
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Vocal Mode
    juce::StringArray vocalModes;
    for (const auto& b : g_vocalModeBounds) {
        vocalModes.add(b.name);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::vocalMode, 1), "Vocal Mode", vocalModes, 3)); // Tenor default

    // Key Root
    juce::StringArray roots = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::keyRoot, 1), "Key", roots, 0));

    // Scale Type
    juce::StringArray scales = { "Chromatic", "Major", "Natural Minor", "Harmonic Minor",
                                "Melodic Minor", "Pentatonic Major", "Pentatonic Minor", "Custom" };
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::scaleType, 1), "Scale", scales, 0));

    // Tuning knobs
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::retuneSpeed, 1), "Retune Speed",
        juce::NormalisableRange<float>(0.0f, 400.0f, 0.1f, 0.35f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::humanize, 1), "Humanize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::flexTune, 1), "Flex-Tune",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::choosyTracking, 1), "Choosy Tracking", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::pitchShift, 1), "Pitch Shift",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.05f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::formantsOn, 1), "Formants", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::formantShift, 1), "Formant Shift",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.05f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::lookaheadMode, 1), "Lookahead / Mixing", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::masterEfxActive, 1), "Master EFX", true));

    // EQ
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::eqEnabled, 1), "EQ Active", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqHpfFreq, 1), "EQ HPF Freq",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.4f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqLowMidGain, 1), "EQ Low-Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqLowMidFreq, 1), "EQ Low-Mid Freq",
        juce::NormalisableRange<float>(200.0f, 1500.0f, 1.0f, 0.4f), 500.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqHighMidGain, 1), "EQ High-Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqHighMidFreq, 1), "EQ High-Mid Freq",
        juce::NormalisableRange<float>(1500.0f, 6000.0f, 1.0f, 0.4f), 3000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqAirGain, 1), "EQ Air Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqAirFreq, 1), "EQ Air Freq",
        juce::NormalisableRange<float>(8000.0f, 16000.0f, 1.0f, 0.4f), 12000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::eqOutGain, 1), "EQ Out Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Compressor
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::compEnabled, 1), "Comp Active", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::compThreshold, 1), "Comp Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -18.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::compRatio, 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 3.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::compAttack, 1), "Comp Attack",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.5f, 0.4f), 15.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::compRelease, 1), "Comp Release",
        juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.4f), 120.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::compMakeup, 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 3.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Flanger
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::flangerEnabled, 1), "Flanger Active", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::flangerRate, 1), "Flanger Rate",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 0.4f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::flangerDepth, 1), "Flanger Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::flangerFeedback, 1), "Flanger Feedback",
        juce::NormalisableRange<float>(-90.0f, 90.0f, 0.5f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::flangerMix, 1), "Flanger Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 40.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Chorus / Haas
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::chorusEnabled, 1), "Chorus/Haas Active", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::chorusMode, 1), "Chorus Mode",
        juce::StringArray { "Chorus", "Haas Widener" }, 0)); // Chorus default
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::chorusRate, 1), "Chorus Rate",
        juce::NormalisableRange<float>(0.1f, 8.0f, 0.05f, 0.4f), 1.2f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::chorusDepth, 1), "Chorus Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 40.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::haasDelay, 1), "Haas Delay",
        juce::NormalisableRange<float>(1.0f, 25.0f, 0.1f), 12.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::haasWidth, 1), "Haas Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::chorusMix, 1), "Chorus/Haas Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Distortion
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::distEnabled, 1), "Dist Active", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::distMode, 1), "Dist Mode",
        juce::StringArray { "Warm Tube", "Tape", "Hard Clip" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::distDrive, 1), "Dist Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::distTone, 1), "Dist Tone",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 10.0f, 0.4f), 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::distMix, 1), "Dist Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Delay / Echo (H-Delay, Supermassive, FreqEcho)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(FreePitchID::delayEnabled, 1), "Delay Active", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(FreePitchID::delayMode, 1), "Delay Mode",
        juce::StringArray { "Stereo", "Ping-Pong" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayTime, 1), "Delay Time",
        juce::NormalisableRange<float>(1.0f, 1500.0f, 1.0f, 0.4f), 250.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayFeedback, 1), "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.5f), 35.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayFreqShift, 1), "Delay Freq Shift",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayDiffusion, 1), "Delay Diffusion",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayTone, 1), "Delay Tone",
        juce::NormalisableRange<float>(1000.0f, 16000.0f, 10.0f, 0.4f), 6000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FreePitchID::delayMix, 1), "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    return layout;
}
