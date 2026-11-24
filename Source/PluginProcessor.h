#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "PresetManager.h"
#include "DSP/PitchDetector.h"
#include "DSP/VoicedClassifier.h"
#include "DSP/ScaleQuantizer.h"
#include "DSP/PitchShifter.h"
#include "DSP/EFXRack.h"

class FreePitchAudioProcessor : public juce::AudioProcessor {
public:
    FreePitchAudioProcessor();
    ~FreePitchAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "FreePitch"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return m_apvts; }
    PresetManager& getPresetManager() { return m_presetManager; }
    EFXRack& getEFXRack() { return m_efxRack; }

    // Lock-free telemetry access for GUI (called on 60fps UI timer)
    PitchTelemetry getLatestTelemetry();

    void setCustomScaleMask(uint32_t mask12Bit);
    uint32_t getCustomScaleMask() const { return m_customScaleMask.load(); }

    void setDarkTheme(bool isDark) { m_isDarkTheme.store(isDark); }
    bool isDarkTheme() const { return m_isDarkTheme.load(); }

private:
    juce::AudioProcessorValueTreeState m_apvts;
    PresetManager m_presetManager;

    // DSP Subsystems
    PitchDetector m_pitchDetector;
    VoicedClassifier m_voicedClassifier;
    ScaleQuantizer m_scaleQuantizer;
    PitchShifter m_pitchShifter;
    EFXRack m_efxRack;

    // Scratch buffers pre-allocated in prepareToPlay
    std::vector<float> m_monoInputBuffer;
    std::vector<float> m_shiftedMonoBuffer;

    // Atomic / Lock-Free Telemetry
    std::atomic<float> m_atomicDetectedFreq { 0.0f };
    std::atomic<float> m_atomicTargetFreq { 0.0f };
    std::atomic<int>   m_atomicDetectedMidi { -1 };
    std::atomic<int>   m_atomicTargetMidi { -1 };
    std::atomic<float> m_atomicCentsDev { 0.0f };
    std::atomic<float> m_atomicAppliedCorr { 0.0f };
    std::atomic<float> m_atomicVoicedConfidence { 0.0f };
    std::atomic<bool>  m_atomicIsVoiced { false };
    std::atomic<float> m_atomicCompGR { 0.0f };
    std::atomic<bool>  m_atomicHaasPhaseWarn { false };
    std::atomic<float> m_atomicInputRmsDb { -100.0f };
    std::atomic<float> m_atomicOutputRmsDb { -100.0f };
    std::atomic<uint32_t> m_customScaleMask { 0x0FFF };
    std::atomic<bool>     m_isDarkTheme { true };

    // Cached parameter pointers for zero-overhead audio thread lookup
    std::atomic<float>* m_prmVocalMode = nullptr;
    std::atomic<float>* m_prmKeyRoot = nullptr;
    std::atomic<float>* m_prmScaleType = nullptr;
    std::atomic<float>* m_prmRetuneSpeed = nullptr;
    std::atomic<float>* m_prmHumanize = nullptr;
    std::atomic<float>* m_prmFlexTune = nullptr;
    std::atomic<float>* m_prmChoosyTracking = nullptr;
    std::atomic<float>* m_prmPitchShift = nullptr;
    std::atomic<float>* m_prmFormantsOn = nullptr;
    std::atomic<float>* m_prmFormantShift = nullptr;
    std::atomic<float>* m_prmLookaheadMode = nullptr;
    std::atomic<float>* m_prmMasterEfxActive = nullptr;

    // EQ
    std::atomic<float>* m_prmEqEnabled = nullptr;
    std::atomic<float>* m_prmEqHpfFreq = nullptr;
    std::atomic<float>* m_prmEqLowMidGain = nullptr;
    std::atomic<float>* m_prmEqLowMidFreq = nullptr;
    std::atomic<float>* m_prmEqHighMidGain = nullptr;
    std::atomic<float>* m_prmEqHighMidFreq = nullptr;
    std::atomic<float>* m_prmEqAirGain = nullptr;
    std::atomic<float>* m_prmEqAirFreq = nullptr;
    std::atomic<float>* m_prmEqOutGain = nullptr;

    // Comp
    std::atomic<float>* m_prmCompEnabled = nullptr;
    std::atomic<float>* m_prmCompThreshold = nullptr;
    std::atomic<float>* m_prmCompRatio = nullptr;
    std::atomic<float>* m_prmCompAttack = nullptr;
    std::atomic<float>* m_prmCompRelease = nullptr;
    std::atomic<float>* m_prmCompMakeup = nullptr;

    // Flanger
    std::atomic<float>* m_prmFlangerEnabled = nullptr;
    std::atomic<float>* m_prmFlangerRate = nullptr;
    std::atomic<float>* m_prmFlangerDepth = nullptr;
    std::atomic<float>* m_prmFlangerFeedback = nullptr;
    std::atomic<float>* m_prmFlangerMix = nullptr;

    // Chorus
    std::atomic<float>* m_prmChorusEnabled = nullptr;
    std::atomic<float>* m_prmChorusMode = nullptr;
    std::atomic<float>* m_prmChorusRate = nullptr;
    std::atomic<float>* m_prmChorusDepth = nullptr;
    std::atomic<float>* m_prmHaasDelay = nullptr;
    std::atomic<float>* m_prmHaasWidth = nullptr;
    std::atomic<float>* m_prmChorusMix = nullptr;

    // Dist
    std::atomic<float>* m_prmDistEnabled = nullptr;
    std::atomic<float>* m_prmDistMode = nullptr;
    std::atomic<float>* m_prmDistDrive = nullptr;
    std::atomic<float>* m_prmDistTone = nullptr;
    std::atomic<float>* m_prmDistMix = nullptr;

    // Delay / Echo
    std::atomic<float>* m_prmDelayEnabled = nullptr;
    std::atomic<float>* m_prmDelayMode = nullptr;
    std::atomic<float>* m_prmDelayTime = nullptr;
    std::atomic<float>* m_prmDelayFeedback = nullptr;
    std::atomic<float>* m_prmDelayFreqShift = nullptr;
    std::atomic<float>* m_prmDelayDiffusion = nullptr;
    std::atomic<float>* m_prmDelayTone = nullptr;
    std::atomic<float>* m_prmDelayMix = nullptr;

    void cacheParameterPointers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FreePitchAudioProcessor)
};
