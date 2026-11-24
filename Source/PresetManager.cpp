#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : m_apvts(apvts) {
    initPresets();
}

void PresetManager::initPresets() {
    m_presets.clear();

    // 1. Init / Natural Male: Tenor, Retune 65 ms, Humanize 70%, Flex-Tune 50%, Formants ON (0 st), EFX bypassed.
    {
        Preset p;
        p.name = "01: Init / Natural Male";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f; // Tenor
        p.values[FreePitchID::retuneSpeed] = 65.0f;
        p.values[FreePitchID::humanize] = 70.0f;
        p.values[FreePitchID::flexTune] = 50.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 0.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        m_presets.push_back(p);
    }

    // 2. Silky Smooth: Tenor, Retune 35 ms, Humanize 60%, Flex-Tune 40%, Formants ON (0 st), gentle EQ (HPF 80 Hz + Air +1.5 dB), smooth 2.5:1 Compressor.
    {
        Preset p;
        p.name = "02: Silky Smooth";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f; // Tenor
        p.values[FreePitchID::retuneSpeed] = 35.0f;
        p.values[FreePitchID::humanize] = 60.0f;
        p.values[FreePitchID::flexTune] = 40.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 0.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHpfFreq] = 80.0f;
        p.values[FreePitchID::eqAirGain] = 1.5f;
        p.values[FreePitchID::compEnabled] = 1.0f;
        p.values[FreePitchID::compRatio] = 2.5f;
        p.values[FreePitchID::compThreshold] = -16.0f;
        m_presets.push_back(p);
    }

    // 3. Hard Rock: Tenor/Baritone, Retune 16 ms, Humanize 25%, Flex-Tune 20%, Formants ON (0 st), mid-forward EQ, punchy 4:1 Compressor, subtle Tube Saturation.
    {
        Preset p;
        p.name = "03: Hard Rock";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 4.0f; // Baritone
        p.values[FreePitchID::retuneSpeed] = 16.0f;
        p.values[FreePitchID::humanize] = 25.0f;
        p.values[FreePitchID::flexTune] = 20.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 0.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHighMidGain] = 3.0f;
        p.values[FreePitchID::eqHighMidFreq] = 2800.0f;
        p.values[FreePitchID::compEnabled] = 1.0f;
        p.values[FreePitchID::compRatio] = 4.0f;
        p.values[FreePitchID::compThreshold] = -20.0f;
        p.values[FreePitchID::distEnabled] = 1.0f;
        p.values[FreePitchID::distMode] = 0.0f; // Warm Tube
        p.values[FreePitchID::distDrive] = 18.0f;
        p.values[FreePitchID::distMix] = 25.0f;
        m_presets.push_back(p);
    }

    // 4. Backing Vocals: Tenor, Retune 20 ms, Humanize 30%, Flex-Tune 15%, Formants ON (+1.0 st), HPF 120 Hz EQ + Haas/Chorus stereo spread.
    {
        Preset p;
        p.name = "04: Backing Vocals";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f;
        p.values[FreePitchID::retuneSpeed] = 20.0f;
        p.values[FreePitchID::humanize] = 30.0f;
        p.values[FreePitchID::flexTune] = 15.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 1.0f; // +1.0 st formant lift
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHpfFreq] = 120.0f;
        p.values[FreePitchID::chorusEnabled] = 1.0f;
        p.values[FreePitchID::chorusMode] = 1.0f; // Haas Widener
        p.values[FreePitchID::haasDelay] = 14.0f;
        p.values[FreePitchID::haasWidth] = 85.0f;
        p.values[FreePitchID::chorusMix] = 60.0f;
        p.values[FreePitchID::delayEnabled] = 1.0f;
        p.values[FreePitchID::delayMode] = 1.0f; // Ping-Pong
        p.values[FreePitchID::delayTime] = 300.0f;
        p.values[FreePitchID::delayFeedback] = 30.0f;
        p.values[FreePitchID::delayDiffusion] = 40.0f; // Supermassive wash
        p.values[FreePitchID::delayMix] = 25.0f;
        m_presets.push_back(p);
    }

    // 5. Ad Lib: Tenor, Retune 6 ms, Humanize 15%, Flex-Tune 10%, Formants OFF (+2.0 st), bandpass/telephone EQ + slow Flanger.
    {
        Preset p;
        p.name = "05: Ad Lib";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f;
        p.values[FreePitchID::retuneSpeed] = 6.0f;
        p.values[FreePitchID::humanize] = 15.0f;
        p.values[FreePitchID::flexTune] = 10.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 0.0f; // Formants OFF
        p.values[FreePitchID::formantShift] = 2.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHpfFreq] = 250.0f;
        p.values[FreePitchID::flangerEnabled] = 1.0f;
        p.values[FreePitchID::flangerRate] = 0.35f;
        p.values[FreePitchID::flangerDepth] = 60.0f;
        p.values[FreePitchID::flangerMix] = 45.0f;
        p.values[FreePitchID::delayEnabled] = 1.0f;
        p.values[FreePitchID::delayMode] = 0.0f; // Stereo
        p.values[FreePitchID::delayTime] = 350.0f;
        p.values[FreePitchID::delayFeedback] = 45.0f;
        p.values[FreePitchID::delayFreqShift] = 6.0f; // +6 Hz Valhalla FreqEcho rise!
        p.values[FreePitchID::delayDiffusion] = 25.0f;
        p.values[FreePitchID::delayMix] = 30.0f;
        m_presets.push_back(p);
    }

    // 6. Trap: Tenor, Retune 3 ms, Humanize 8%, Flex-Tune 5%, Formants ON (-1.5 st), HPF + tight Compressor + subtle Saturation.
    {
        Preset p;
        p.name = "06: Trap";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f;
        p.values[FreePitchID::retuneSpeed] = 3.0f;
        p.values[FreePitchID::humanize] = 8.0f;
        p.values[FreePitchID::flexTune] = 5.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = -1.5f; // -1.5 st
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHpfFreq] = 100.0f;
        p.values[FreePitchID::compEnabled] = 1.0f;
        p.values[FreePitchID::compRatio] = 5.0f;
        p.values[FreePitchID::compThreshold] = -22.0f;
        p.values[FreePitchID::distEnabled] = 1.0f;
        p.values[FreePitchID::distMode] = 1.0f; // Tape
        p.values[FreePitchID::distDrive] = 25.0f;
        p.values[FreePitchID::distMix] = 20.0f;
        m_presets.push_back(p);
    }

    // 7. Melodic Rap: Tenor, Retune 14 ms, Humanize 30%, Flex-Tune 20%, Formants ON (0 st), presence EQ + fast Compressor + light wide Chorus.
    {
        Preset p;
        p.name = "07: Melodic Rap";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f;
        p.values[FreePitchID::retuneSpeed] = 14.0f;
        p.values[FreePitchID::humanize] = 30.0f;
        p.values[FreePitchID::flexTune] = 20.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 0.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqHighMidGain] = 2.5f;
        p.values[FreePitchID::compEnabled] = 1.0f;
        p.values[FreePitchID::compRatio] = 3.5f;
        p.values[FreePitchID::chorusEnabled] = 1.0f;
        p.values[FreePitchID::chorusMode] = 0.0f; // Chorus
        p.values[FreePitchID::chorusRate] = 0.8f;
        p.values[FreePitchID::chorusDepth] = 35.0f;
        p.values[FreePitchID::chorusMix] = 30.0f;
        m_presets.push_back(p);
    }

    // 8. Hard Tune: Tenor, Retune 0 ms, Humanize 0%, Flex-Tune 0%, Formants ON (0 st), crisp top-end EQ + firm Compressor.
    {
        Preset p;
        p.name = "08: Hard Tune";
        p.isFactory = true;
        p.values[FreePitchID::vocalMode] = 3.0f;
        p.values[FreePitchID::retuneSpeed] = 0.0f; // 0 ms instant hard snap
        p.values[FreePitchID::humanize] = 0.0f;
        p.values[FreePitchID::flexTune] = 0.0f;
        p.values[FreePitchID::pitchShift] = 0.0f;
        p.values[FreePitchID::formantsOn] = 1.0f;
        p.values[FreePitchID::formantShift] = 0.0f;
        p.values[FreePitchID::masterEfxActive] = 1.0f;
        p.values[FreePitchID::eqEnabled] = 1.0f;
        p.values[FreePitchID::eqAirGain] = 3.5f;
        p.values[FreePitchID::compEnabled] = 1.0f;
        p.values[FreePitchID::compRatio] = 4.5f;
        p.values[FreePitchID::compThreshold] = -24.0f;
        m_presets.push_back(p);
    }

    m_currentPresetIndex = 0;
    m_stateA = m_apvts.copyState();
    m_stateB = m_apvts.copyState();
}

juce::String PresetManager::getPresetName(int index) const {
    if (index >= 0 && index < static_cast<int>(m_presets.size())) {
        return m_presets[index].name;
    }
    return {};
}

void PresetManager::loadPreset(int index) {
    if (index >= 0 && index < static_cast<int>(m_presets.size())) {
        m_currentPresetIndex = index;
        applyPresetValues(m_presets[index]);
    }
}

void PresetManager::nextPreset() {
    int next = (m_currentPresetIndex + 1) % static_cast<int>(m_presets.size());
    loadPreset(next);
}

void PresetManager::previousPreset() {
    int prev = (m_currentPresetIndex - 1 + static_cast<int>(m_presets.size())) % static_cast<int>(m_presets.size());
    loadPreset(prev);
}

void PresetManager::toggleAB() {
    if (!m_isStateB) {
        // Save current to A, load B
        m_stateA = m_apvts.copyState();
        m_apvts.replaceState(m_stateB);
        m_isStateB = true;
    } else {
        // Save current to B, load A
        m_stateB = m_apvts.copyState();
        m_apvts.replaceState(m_stateA);
        m_isStateB = false;
    }
}

void PresetManager::saveUserPreset(const juce::String& name) {
    Preset p;
    p.name = name.isNotEmpty() ? name : juce::String("User Preset ") + juce::String(m_presets.size() + 1);
    p.isFactory = false;

    for (auto* param : m_apvts.processor.getParameters()) {
        if (auto* withID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param)) {
            p.values[withID->paramID] = withID->getValue();
        }
    }

    m_presets.push_back(p);
    m_currentPresetIndex = static_cast<int>(m_presets.size()) - 1;
}

void PresetManager::resetToDefault() {
    loadPreset(0);
}

void PresetManager::applyPresetValues(const Preset& p) {
    for (const auto& [paramID, val] : p.values) {
        if (auto* param = m_apvts.getParameter(paramID)) {
            if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(param)) {
                *choice = static_cast<int>(val);
            } else if (auto* boolParam = dynamic_cast<juce::AudioParameterBool*>(param)) {
                *boolParam = (val > 0.5f);
            } else if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
                *floatParam = val;
            }
        }
    }
}
