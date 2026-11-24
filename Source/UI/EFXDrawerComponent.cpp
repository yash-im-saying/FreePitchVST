#include "EFXDrawerComponent.h"
#include <cmath>

EFXDrawerComponent::EFXDrawerComponent(juce::AudioProcessorValueTreeState& apvts, EFXRack& efxRack)
    : m_apvts(apvts), m_efxRack(efxRack) {

    // 1. Setup 6 Module Tabs
    setupTab(m_eqTab,      "EQ",      FreePitchID::eqEnabled,      1, "4-Band Sculpt EQ: Tailor vocal body, presence, and air.");
    setupTab(m_compTab,    "COMP",    FreePitchID::compEnabled,    2, "Vocal FET/Opto Compressor: Smooth vocal dynamics and level consistency.");
    setupTab(m_flangerTab, "FLANGE",  FreePitchID::flangerEnabled, 3, "Stereo Flanger: Swirling spatial harmonic modulation.");
    setupTab(m_chorusTab,  "CHORUS",  FreePitchID::chorusEnabled,  4, "Chorus / Haas Widener: Thick vocal doubles and wide stereo imagery.");
    setupTab(m_distTab,    "DRIVE",   FreePitchID::distEnabled,    5, "Waveshaping Saturation: Warm Tube, Tape, and Hard Clip harmonics.");
    setupTab(m_delayTab,   "DELAY",   FreePitchID::delayEnabled,   6, "Space Delay / FreqEcho: Tape echo, frequency shifter & Supermassive diffusion.");

    // --- EQ Controls ---
    if (auto* p = m_apvts.getParameter(FreePitchID::eqHpfFreq)) {
        m_eqHpfKnob = std::make_unique<RotaryKnob>(*p, "HPF", "Hz");
        m_eqHpfKnob->setTooltipText("High-Pass Filter: Cuts sub-bass rumble and low-end plosives (20 - 300 Hz).");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqHpfFreq, m_eqHpfKnob->getSlider()));
        addChildComponent(m_eqHpfKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqLowMidGain)) {
        m_eqLowMidGainKnob = std::make_unique<RotaryKnob>(*p, "Low-Mid", "dB");
        m_eqLowMidGainKnob->setTooltipText("Low-Mid Bell (500 Hz): Controls vocal chest resonance and muddiness.");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqLowMidGain, m_eqLowMidGainKnob->getSlider()));
        addChildComponent(m_eqLowMidGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqHighMidGain)) {
        m_eqHighMidGainKnob = std::make_unique<RotaryKnob>(*p, "Presence", "dB");
        m_eqHighMidGainKnob->setTooltipText("High-Mid Bell (3 kHz): Enhances vocal clarity, bite, and articulation.");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqHighMidGain, m_eqHighMidGainKnob->getSlider()));
        addChildComponent(m_eqHighMidGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqAirGain)) {
        m_eqAirGainKnob = std::make_unique<RotaryKnob>(*p, "Air", "dB");
        m_eqAirGainKnob->setTooltipText("Air Shelf (12 kHz): Adds modern high-frequency sheen and breath shimmer.");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqAirGain, m_eqAirGainKnob->getSlider()));
        addChildComponent(m_eqAirGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqOutGain)) {
        m_eqOutGainKnob = std::make_unique<RotaryKnob>(*p, "Output", "dB");
        m_eqOutGainKnob->setTooltipText("EQ Output Level: Compensates for overall EQ boost or cut.");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqOutGain, m_eqOutGainKnob->getSlider()));
        addChildComponent(m_eqOutGainKnob.get());
    }

    // --- Compressor Controls ---
    if (auto* p = m_apvts.getParameter(FreePitchID::compThreshold)) {
        m_compThreshKnob = std::make_unique<RotaryKnob>(*p, "Threshold", "dB");
        m_compThreshKnob->setTooltipText("Compressor Threshold: Level above which gain reduction begins.");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compThreshold, m_compThreshKnob->getSlider()));
        addChildComponent(m_compThreshKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compRatio)) {
        m_compRatioKnob = std::make_unique<RotaryKnob>(*p, "Ratio", ":1");
        m_compRatioKnob->setTooltipText("Compression Ratio: Compression intensity (e.g. 4:1 for vocals).");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compRatio, m_compRatioKnob->getSlider()));
        addChildComponent(m_compRatioKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compAttack)) {
        m_compAttackKnob = std::make_unique<RotaryKnob>(*p, "Attack", "ms");
        m_compAttackKnob->setTooltipText("Attack Time: Speed at which compression engages on vocal transients.");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compAttack, m_compAttackKnob->getSlider()));
        addChildComponent(m_compAttackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compRelease)) {
        m_compReleaseKnob = std::make_unique<RotaryKnob>(*p, "Release", "ms");
        m_compReleaseKnob->setTooltipText("Release Time: Speed at which compressor recovers after peaks.");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compRelease, m_compReleaseKnob->getSlider()));
        addChildComponent(m_compReleaseKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compMakeup)) {
        m_compMakeupKnob = std::make_unique<RotaryKnob>(*p, "Makeup", "dB");
        m_compMakeupKnob->setTooltipText("Makeup Gain: Restores signal level lost during compression.");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compMakeup, m_compMakeupKnob->getSlider()));
        addChildComponent(m_compMakeupKnob.get());
    }

    // --- Flanger Controls ---
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerRate)) {
        m_flangerRateKnob = std::make_unique<RotaryKnob>(*p, "Rate", "Hz");
        m_flangerRateKnob->setTooltipText("Flanger Modulation Rate: LFO cycling frequency (0.05 to 5 Hz).");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerRate, m_flangerRateKnob->getSlider()));
        addChildComponent(m_flangerRateKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerDepth)) {
        m_flangerDepthKnob = std::make_unique<RotaryKnob>(*p, "Depth", "%");
        m_flangerDepthKnob->setTooltipText("Flanger Depth: Intensity of delay sweep modulation.");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerDepth, m_flangerDepthKnob->getSlider()));
        addChildComponent(m_flangerDepthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerFeedback)) {
        m_flangerFeedbackKnob = std::make_unique<RotaryKnob>(*p, "Feedback", "%");
        m_flangerFeedbackKnob->setTooltipText("Flanger Feedback: Resonance and comb-filtering metallic edge.");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerFeedback, m_flangerFeedbackKnob->getSlider()));
        addChildComponent(m_flangerFeedbackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerMix)) {
        m_flangerMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_flangerMixKnob->setTooltipText("Flanger Dry/Wet Mix: Blends original vocal with flanged effect.");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerMix, m_flangerMixKnob->getSlider()));
        addChildComponent(m_flangerMixKnob.get());
    }

    // --- Chorus / Haas Controls ---
    m_chorusModeBox.addItem("Modulated Chorus", 1);
    m_chorusModeBox.addItem("Haas 3D Widener", 2);
    m_chorusModeBox.setTooltip("Stereo Mode: Modulated multi-voice Chorus vs psychoacoustic Haas delay widening.");
    m_chorusModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::chorusMode, m_chorusModeBox);
    m_chorusModeBox.onChange = [this]() {
        updateChorusControlsVisibility();
        resized();
        repaint();
    };
    addChildComponent(m_chorusModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::chorusRate)) {
        m_chorusRateKnob = std::make_unique<RotaryKnob>(*p, "Rate", "Hz");
        m_chorusRateKnob->setTooltipText("Chorus Modulation Rate: Speed of stereo delay modulation.");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusRate, m_chorusRateKnob->getSlider()));
        addChildComponent(m_chorusRateKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::chorusDepth)) {
        m_chorusDepthKnob = std::make_unique<RotaryKnob>(*p, "Depth", "%");
        m_chorusDepthKnob->setTooltipText("Chorus Modulation Depth: Width of pitch-modulated detuning.");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusDepth, m_chorusDepthKnob->getSlider()));
        addChildComponent(m_chorusDepthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::haasDelay)) {
        m_haasDelayKnob = std::make_unique<RotaryKnob>(*p, "Delay", "ms");
        m_haasDelayKnob->setTooltipText("Haas Micro-Delay (1 - 25 ms): Inter-aural time difference for 3D width.");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::haasDelay, m_haasDelayKnob->getSlider()));
        addChildComponent(m_haasDelayKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::haasWidth)) {
        m_haasWidthKnob = std::make_unique<RotaryKnob>(*p, "Width", "%");
        m_haasWidthKnob->setTooltipText("Haas Stereo Width: Expands spatial separation between left and right.");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::haasWidth, m_haasWidthKnob->getSlider()));
        addChildComponent(m_haasWidthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::chorusMix)) {
        m_chorusMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_chorusMixKnob->setTooltipText("Chorus/Haas Dry/Wet Mix: Blends direct vocal with stereo effect.");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusMix, m_chorusMixKnob->getSlider()));
        addChildComponent(m_chorusMixKnob.get());
    }

    // --- Saturation Controls ---
    m_distModeBox.addItem("Warm Tube", 1);
    m_distModeBox.addItem("Tape Saturation", 2);
    m_distModeBox.addItem("Hard Clip", 3);
    m_distModeBox.setTooltip("Saturation Flavor: Warm Tube (2nd harmonics), Tape (punchy odd harmonics), or Hard Clip.");
    m_distModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::distMode, m_distModeBox);
    addChildComponent(m_distModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::distDrive)) {
        m_distDriveKnob = std::make_unique<RotaryKnob>(*p, "Drive", "%");
        m_distDriveKnob->setTooltipText("Drive Amount: Pushes vocal into analog harmonic saturation with auto-makeup.");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distDrive, m_distDriveKnob->getSlider()));
        addChildComponent(m_distDriveKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::distTone)) {
        m_distToneKnob = std::make_unique<RotaryKnob>(*p, "Tone", "Hz");
        m_distToneKnob->setTooltipText("Post-Drive Tone: Damps harsh top-end fizz (1 kHz to 16 kHz).");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distTone, m_distToneKnob->getSlider()));
        addChildComponent(m_distToneKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::distMix)) {
        m_distMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_distMixKnob->setTooltipText("Saturation Dry/Wet Mix: Parallel saturation blending.");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distMix, m_distMixKnob->getSlider()));
        addChildComponent(m_distMixKnob.get());
    }

    // --- Space Delay / FreqEcho Controls ---
    m_delayModeBox.addItem("Stereo Echo", 1);
    m_delayModeBox.addItem("Ping-Pong", 2);
    m_delayModeBox.setTooltip("Delay Mode: Balanced Stereo repeats vs dynamic Ping-Pong alternating ears.");
    m_delayModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::delayMode, m_delayModeBox);
    addChildComponent(m_delayModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::delayTime)) {
        m_delayTimeKnob = std::make_unique<RotaryKnob>(*p, "Time", "ms");
        m_delayTimeKnob->setTooltipText("Delay Time (10 to 2000 ms): Tape pitch glide on adjustment.");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayTime, m_delayTimeKnob->getSlider()));
        addChildComponent(m_delayTimeKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayFeedback)) {
        m_delayFeedbackKnob = std::make_unique<RotaryKnob>(*p, "Feedback", "%");
        m_delayFeedbackKnob->setTooltipText("Delay Feedback: Repeat decay sustained with analog tape saturation.");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayFeedback, m_delayFeedbackKnob->getSlider()));
        addChildComponent(m_delayFeedbackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayFreqShift)) {
        m_delayFreqShiftKnob = std::make_unique<RotaryKnob>(*p, "Shift", "Hz");
        m_delayFreqShiftKnob->setTooltipText("Valhalla FreqEcho: Frequency shifts repeats by +/-100 Hz for spiraling echoes.");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayFreqShift, m_delayFreqShiftKnob->getSlider()));
        addChildComponent(m_delayFreqShiftKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayDiffusion)) {
        m_delayDiffusionKnob = std::make_unique<RotaryKnob>(*p, "Diffuse", "%");
        m_delayDiffusionKnob->setTooltipText("Valhalla Supermassive Diffusion: Smears echoes into lush ambient reverb clouds.");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayDiffusion, m_delayDiffusionKnob->getSlider()));
        addChildComponent(m_delayDiffusionKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayTone)) {
        m_delayToneKnob = std::make_unique<RotaryKnob>(*p, "Damping", "Hz");
        m_delayToneKnob->setTooltipText("Delay Damping Filter: Analog tape HF roll-off (1 kHz to 16 kHz).");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayTone, m_delayToneKnob->getSlider()));
        addChildComponent(m_delayToneKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayMix)) {
        m_delayMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_delayMixKnob->setTooltipText("Delay Dry/Wet Mix: Blends direct voice with spatial repeats.");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayMix, m_delayMixKnob->getSlider()));
        addChildComponent(m_delayMixKnob.get());
    }

    setActiveTab(1); // EQ active by default
}

void EFXDrawerComponent::setupTab(ModuleTab& tab, const juce::String& name, const char* paramID, int tabIndex, const juce::String& tip) {
    tab.selectBtn.setButtonText(name);
    tab.selectBtn.setTooltip(tip);
    tab.selectBtn.onClick = [this, tabIndex]() {
        setActiveTab(tabIndex);
    };
    addAndMakeVisible(tab.selectBtn);

    tab.powerBtn.setButtonText("");
    tab.powerBtn.setTooltip("Toggle " + name + " Power ON/BYPASS");
    tab.powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(m_apvts, paramID, tab.powerBtn);
    addAndMakeVisible(tab.powerBtn);
}

void EFXDrawerComponent::setActiveTab(int index) {
    m_activeTab = index;

    bool isEq   = (index == 1);
    bool isComp = (index == 2);
    bool isFlg  = (index == 3);
    bool isCho  = (index == 4);
    bool isDist = (index == 5);
    bool isDly  = (index == 6);

    auto ensurePower = [this](ModuleTab& tab) {
        if (!tab.powerBtn.getToggleState()) {
            tab.powerBtn.setToggleState(true, juce::sendNotification);
        }
    };

    if (isEq)   ensurePower(m_eqTab);
    if (isComp) ensurePower(m_compTab);
    if (isFlg)  ensurePower(m_flangerTab);
    if (isCho)  ensurePower(m_chorusTab);
    if (isDist) ensurePower(m_distTab);
    if (isDly)  ensurePower(m_delayTab);

    // EQ controls
    if (m_eqHpfKnob)         m_eqHpfKnob->setVisible(isEq);
    if (m_eqLowMidGainKnob)  m_eqLowMidGainKnob->setVisible(isEq);
    if (m_eqHighMidGainKnob) m_eqHighMidGainKnob->setVisible(isEq);
    if (m_eqAirGainKnob)     m_eqAirGainKnob->setVisible(isEq);
    if (m_eqOutGainKnob)     m_eqOutGainKnob->setVisible(isEq);

    // Comp controls
    if (m_compThreshKnob)    m_compThreshKnob->setVisible(isComp);
    if (m_compRatioKnob)     m_compRatioKnob->setVisible(isComp);
    if (m_compAttackKnob)    m_compAttackKnob->setVisible(isComp);
    if (m_compReleaseKnob)   m_compReleaseKnob->setVisible(isComp);
    if (m_compMakeupKnob)    m_compMakeupKnob->setVisible(isComp);

    // Flanger controls
    if (m_flangerRateKnob)     m_flangerRateKnob->setVisible(isFlg);
    if (m_flangerDepthKnob)    m_flangerDepthKnob->setVisible(isFlg);
    if (m_flangerFeedbackKnob) m_flangerFeedbackKnob->setVisible(isFlg);
    if (m_flangerMixKnob)      m_flangerMixKnob->setVisible(isFlg);

    // Chorus controls
    m_chorusModeBox.setVisible(isCho);
    if (isCho) updateChorusControlsVisibility();
    else {
        if (m_chorusRateKnob)  m_chorusRateKnob->setVisible(false);
        if (m_chorusDepthKnob) m_chorusDepthKnob->setVisible(false);
        if (m_haasDelayKnob)   m_haasDelayKnob->setVisible(false);
        if (m_haasWidthKnob)   m_haasWidthKnob->setVisible(false);
        if (m_chorusMixKnob)   m_chorusMixKnob->setVisible(false);
    }

    // Saturation controls
    m_distModeBox.setVisible(isDist);
    if (m_distDriveKnob) m_distDriveKnob->setVisible(isDist);
    if (m_distToneKnob)  m_distToneKnob->setVisible(isDist);
    if (m_distMixKnob)   m_distMixKnob->setVisible(isDist);

    // Delay controls
    m_delayModeBox.setVisible(isDly);
    if (m_delayTimeKnob)      m_delayTimeKnob->setVisible(isDly);
    if (m_delayFeedbackKnob)  m_delayFeedbackKnob->setVisible(isDly);
    if (m_delayFreqShiftKnob) m_delayFreqShiftKnob->setVisible(isDly);
    if (m_delayDiffusionKnob) m_delayDiffusionKnob->setVisible(isDly);
    if (m_delayToneKnob)      m_delayToneKnob->setVisible(isDly);
    if (m_delayMixKnob)       m_delayMixKnob->setVisible(isDly);

    resized();
    repaint();
}

void EFXDrawerComponent::updateChorusControlsVisibility() {
    int mode = m_chorusModeBox.getSelectedId();
    bool isChorus = (mode == 1);
    if (m_chorusRateKnob)  m_chorusRateKnob->setVisible(isChorus);
    if (m_chorusDepthKnob) m_chorusDepthKnob->setVisible(isChorus);
    if (m_haasDelayKnob)   m_haasDelayKnob->setVisible(!isChorus);
    if (m_haasWidthKnob)   m_haasWidthKnob->setVisible(!isChorus);
    if (m_chorusMixKnob)   m_chorusMixKnob->setVisible(true);
}

void EFXDrawerComponent::updateTelemetry(const PitchTelemetry& telemetry) {
    m_telemetry = telemetry;
    repaint();
}

void EFXDrawerComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);

    // 1. Top Tab Selector Bar
    auto tabBar = bounds.removeFromTop(32);
    int numTabs = 6;
    float tabWidth = static_cast<float>(tabBar.getWidth()) / static_cast<float>(numTabs);

    auto layoutTab = [tabBar, tabWidth](ModuleTab& tab, int idx) {
        float tx = static_cast<float>(tabBar.getX()) + static_cast<float>(idx) * tabWidth;
        auto tabArea = juce::Rectangle<float>(tx, static_cast<float>(tabBar.getY()), tabWidth, static_cast<float>(tabBar.getHeight())).toNearestInt();

        tab.powerBtn.setBounds(tabArea.removeFromLeft(18).reduced(1));
        tab.selectBtn.setBounds(tabArea.reduced(1));
    };

    layoutTab(m_eqTab,      0);
    layoutTab(m_compTab,    1);
    layoutTab(m_flangerTab, 2);
    layoutTab(m_chorusTab,  3);
    layoutTab(m_distTab,    4);
    layoutTab(m_delayTab,   5);

    // 2. Main Studio Rack Bay
    bounds.removeFromTop(4); // separator
    auto bayArea = bounds;
    m_activeBayArea = bayArea.toFloat();

    if (m_activeTab == 1) { // EQ
        // Responsive curve height: scales with vertical height of bay
        int curveH = std::clamp(static_cast<int>(bayArea.getHeight() * 0.46f), 70, 160);
        m_eqCurveArea = bayArea.removeFromTop(curveH).toFloat().reduced(6.0f, 4.0f);
        bayArea.removeFromTop(4);

        int kw = bayArea.getWidth() / 5;
        if (m_eqHpfKnob)         m_eqHpfKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_eqLowMidGainKnob)  m_eqLowMidGainKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_eqHighMidGainKnob) m_eqHighMidGainKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_eqAirGainKnob)     m_eqAirGainKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_eqOutGainKnob)     m_eqOutGainKnob->setBounds(bayArea);
    } else if (m_activeTab == 2) { // Compressor
        // Responsive GR meter height
        int meterH = std::clamp(static_cast<int>(bayArea.getHeight() * 0.38f), 55, 120);
        m_compMeterArea = bayArea.removeFromTop(meterH).toFloat().reduced(6.0f, 4.0f);
        bayArea.removeFromTop(4);

        int kw = bayArea.getWidth() / 5;
        if (m_compThreshKnob)  m_compThreshKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_compRatioKnob)   m_compRatioKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_compAttackKnob)  m_compAttackKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_compReleaseKnob) m_compReleaseKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_compMakeupKnob)  m_compMakeupKnob->setBounds(bayArea);
    } else if (m_activeTab == 3) { // Flanger
        bayArea.removeFromTop(16);
        int kw = bayArea.getWidth() / 4;
        if (m_flangerRateKnob)     m_flangerRateKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_flangerDepthKnob)    m_flangerDepthKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_flangerFeedbackKnob) m_flangerFeedbackKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_flangerMixKnob)      m_flangerMixKnob->setBounds(bayArea);
    } else if (m_activeTab == 4) { // Chorus / Haas
        auto topArea = bayArea.removeFromTop(32);
        m_chorusModeBox.setBounds(topArea.removeFromLeft(160).reduced(2));
        bayArea.removeFromTop(8);

        int mode = m_chorusModeBox.getSelectedId();
        int kw = bayArea.getWidth() / 3;
        if (mode == 1) { // Chorus: Rate, Depth, Mix
            if (m_chorusRateKnob)  m_chorusRateKnob->setBounds(bayArea.removeFromLeft(kw));
            if (m_chorusDepthKnob) m_chorusDepthKnob->setBounds(bayArea.removeFromLeft(kw));
            if (m_chorusMixKnob)   m_chorusMixKnob->setBounds(bayArea);
        } else { // Haas: Delay, Width, Mix
            if (m_haasDelayKnob) m_haasDelayKnob->setBounds(bayArea.removeFromLeft(kw));
            if (m_haasWidthKnob) m_haasWidthKnob->setBounds(bayArea.removeFromLeft(kw));
            if (m_chorusMixKnob) m_chorusMixKnob->setBounds(bayArea);
        }
    } else if (m_activeTab == 5) { // Saturation
        auto topArea = bayArea.removeFromTop(32);
        m_distModeBox.setBounds(topArea.removeFromLeft(160).reduced(2));
        bayArea.removeFromTop(8);

        int kw = bayArea.getWidth() / 3;
        if (m_distDriveKnob) m_distDriveKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_distToneKnob)  m_distToneKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_distMixKnob)   m_distMixKnob->setBounds(bayArea);
    } else if (m_activeTab == 6) { // Space Delay
        auto topArea = bayArea.removeFromTop(32);
        m_delayModeBox.setBounds(topArea.removeFromLeft(140).reduced(2));
        if (m_delayToneKnob) m_delayToneKnob->setBounds(topArea.removeFromRight(100));
        bayArea.removeFromTop(8);

        int kw = bayArea.getWidth() / 5;
        if (m_delayTimeKnob)      m_delayTimeKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_delayFeedbackKnob)  m_delayFeedbackKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_delayFreqShiftKnob) m_delayFreqShiftKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_delayDiffusionKnob) m_delayDiffusionKnob->setBounds(bayArea.removeFromLeft(kw));
        if (m_delayMixKnob)       m_delayMixKnob->setBounds(bayArea);
    }
}

void EFXDrawerComponent::paintEqCurve(juce::Graphics& g, juce::Rectangle<float> area) {
    if (area.isEmpty()) return;

    // Skeuomorphic CRT / LCD monitor screen bezel
    FreePitchLookAndFeel::drawNeumorphicSunken(g, area, 5.0f, 3.0f);

    float midY = area.getCentreY();

    // Frequency Grid lines (100 Hz, 1 kHz, 10 kHz)
    float minF = 20.0f;
    float maxF = 20000.0f;
    float logRange = std::log10(maxF / minF);

    auto drawFreqGrid = [&](float freq, const juce::String& label) {
        float frac = std::log10(freq / minF) / logRange;
        float gx = area.getX() + frac * area.getWidth();
        g.setColour(FreePitchLookAndFeel::kPanelBorder.withAlpha(0.6f));
        g.drawVerticalLine(static_cast<int>(gx), area.getY(), area.getBottom());

        g.setColour(FreePitchLookAndFeel::kTextDim);
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
        g.drawText(label, static_cast<int>(gx + 2), static_cast<int>(area.getBottom() - 13), 32, 11, juce::Justification::centredLeft);
    };

    drawFreqGrid(100.0f, "100");
    drawFreqGrid(1000.0f, "1k");
    drawFreqGrid(10000.0f, "10k");

    // Center 0 dB reference line
    g.setColour(FreePitchLookAndFeel::kPanelBorderLight);
    g.drawHorizontalLine(static_cast<int>(midY), area.getX(), area.getRight());

    // Update EFXRack from live parameters
    auto getVal = [this](const char* id, float def) {
        if (auto* p = m_apvts.getRawParameterValue(id)) return p->load();
        return def;
    };

    bool eqOn = (getVal(FreePitchID::eqEnabled, 1.0f) > 0.5f);
    float hpf = getVal(FreePitchID::eqHpfFreq, 80.0f);
    float lmF = getVal(FreePitchID::eqLowMidFreq, 500.0f);
    float lmG = getVal(FreePitchID::eqLowMidGain, 0.0f);
    float hmF = getVal(FreePitchID::eqHighMidFreq, 3000.0f);
    float hmG = getVal(FreePitchID::eqHighMidGain, 0.0f);
    float airF = getVal(FreePitchID::eqAirFreq, 12000.0f);
    float airG = getVal(FreePitchID::eqAirGain, 0.0f);
    float outG = getVal(FreePitchID::eqOutGain, 0.0f);

    m_efxRack.setEqParams(eqOn, hpf, lmF, lmG, hmF, hmG, airF, airG, outG);

    // Build curve path and fill area
    juce::Path curve;
    juce::Path fillArea;
    bool started = false;

    for (int i = 0; i <= 60; ++i) {
        float frac = static_cast<float>(i) / 60.0f;
        float freq = minF * std::pow(maxF / minF, frac);
        float db = m_efxRack.getEqResponseAt(freq);

        float px = area.getX() + frac * area.getWidth();
        float py = midY - (db / 15.0f) * (area.getHeight() * 0.44f);
        py = std::clamp(py, area.getY() + 2.0f, area.getBottom() - 2.0f);

        if (!started) {
            curve.startNewSubPath(px, py);
            fillArea.startNewSubPath(px, midY);
            fillArea.lineTo(px, py);
            started = true;
        } else {
            curve.lineTo(px, py);
            fillArea.lineTo(px, py);
        }
    }
    fillArea.lineTo(area.getRight(), midY);
    fillArea.closeSubPath();

    if (eqOn) {
        // Gradient fill under curve
        juce::ColourGradient fillGrad(FreePitchLookAndFeel::kAccentCyan.withAlpha(0.25f), 0.0f, area.getY(),
                                      FreePitchLookAndFeel::kAccentCyan.withAlpha(0.02f), 0.0f, midY, false);
        g.setGradientFill(fillGrad);
        g.fillPath(fillArea);

        // Luminous curve stroke
        g.setColour(FreePitchLookAndFeel::kAccentCyan.withAlpha(0.4f));
        g.strokePath(curve, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(FreePitchLookAndFeel::kAccentCyan);
        g.strokePath(curve, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    } else {
        g.setColour(FreePitchLookAndFeel::kTextDim);
        g.strokePath(curve, juce::PathStrokeType(1.2f));
    }
}

void EFXDrawerComponent::paintCompMeter(juce::Graphics& g, juce::Rectangle<float> area) {
    if (area.isEmpty()) return;

    // Skeuomorphic analog meter bezel screen
    FreePitchLookAndFeel::drawNeumorphicSunken(g, area, 5.0f, 3.0f);

    float grDb = m_telemetry.compressorGR_dB;
    float grFrac = std::clamp(grDb / 20.0f, 0.0f, 1.0f); // 0 to 20 dB GR

    // Meter track
    auto meterTrack = area.reduced(10.0f, 16.0f);
    g.setColour(FreePitchLookAndFeel::kPanelDark);
    g.fillRoundedRectangle(meterTrack, 3.0f);

    if (grFrac > 0.005f) {
        float activeWidth = meterTrack.getWidth() * grFrac;
        auto activeBar = juce::Rectangle<float>(meterTrack.getRight() - activeWidth, meterTrack.getY(),
                                                activeWidth, meterTrack.getHeight());

        juce::ColourGradient grGrad(FreePitchLookAndFeel::kIndicatorSharp, activeBar.getX(), 0.0f,
                                    FreePitchLookAndFeel::kIndicatorWarning, activeBar.getRight(), 0.0f, false);
        g.setGradientFill(grGrad);
        g.fillRoundedRectangle(activeBar, 3.0f);
    }

    // dB scale labels (-20, -12, -6, -3, 0)
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.setColour(FreePitchLookAndFeel::kTextMuted);

    int dBs[] = { 20, 12, 6, 3, 0 };
    for (int db : dBs) {
        float frac = static_cast<float>(db) / 20.0f;
        float x = meterTrack.getRight() - frac * meterTrack.getWidth();
        g.drawText("-" + juce::String(db), static_cast<int>(x - 12), static_cast<int>(meterTrack.getY() - 13), 24, 12, juce::Justification::centred);
    }

    // Live GR numeric badge
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
    g.setColour(grDb > 0.5f ? FreePitchLookAndFeel::kAccentAmber : FreePitchLookAndFeel::kTextDim);
    juce::String grText = "GAIN REDUCTION: -" + juce::String(grDb, 1) + " dB";
    g.drawText(grText, static_cast<int>(area.getX()), static_cast<int>(meterTrack.getBottom() + 4),
               static_cast<int>(area.getWidth()), 16, juce::Justification::centred);
}

void EFXDrawerComponent::updateThemeColors() {
    auto updateKnob = [](std::unique_ptr<RotaryKnob>& k) {
        if (k) k->updateThemeColors();
    };

    updateKnob(m_eqHpfKnob);
    updateKnob(m_eqLowMidGainKnob);
    updateKnob(m_eqHighMidGainKnob);
    updateKnob(m_eqAirGainKnob);
    updateKnob(m_eqOutGainKnob);

    updateKnob(m_compThreshKnob);
    updateKnob(m_compRatioKnob);
    updateKnob(m_compAttackKnob);
    updateKnob(m_compReleaseKnob);
    updateKnob(m_compMakeupKnob);

    updateKnob(m_flangerRateKnob);
    updateKnob(m_flangerDepthKnob);
    updateKnob(m_flangerFeedbackKnob);
    updateKnob(m_flangerMixKnob);

    updateKnob(m_chorusRateKnob);
    updateKnob(m_chorusDepthKnob);
    updateKnob(m_haasDelayKnob);
    updateKnob(m_haasWidthKnob);
    updateKnob(m_chorusMixKnob);

    updateKnob(m_distDriveKnob);
    updateKnob(m_distToneKnob);
    updateKnob(m_distMixKnob);

    updateKnob(m_delayTimeKnob);
    updateKnob(m_delayFeedbackKnob);
    updateKnob(m_delayFreqShiftKnob);
    updateKnob(m_delayDiffusionKnob);
    updateKnob(m_delayMixKnob);
    updateKnob(m_delayToneKnob);

    repaint();
}

void EFXDrawerComponent::paint(juce::Graphics& g) {
    if (m_activeBayArea.isEmpty()) return;

    const auto& p = FreePitchLookAndFeel::getPalette();

    // 1. Skeuomorphic Rack Bay Container (Sunken Channel Bay)
    FreePitchLookAndFeel::drawNeumorphicSunken(g, m_activeBayArea, 6.0f, 3.0f);

    // 2. Highlight active tab with glowing cyan neon status bar
    float tabWidth = static_cast<float>(getWidth() - 4) / 6.0f;
    float activeTabX = 2.0f + static_cast<float>(m_activeTab - 1) * tabWidth;
    g.setColour(p.accentCyan.withAlpha(0.35f));
    g.fillRect(activeTabX + 2.0f, 31.0f, tabWidth - 4.0f, 4.0f);
    g.setColour(p.accentCyan);
    g.fillRect(activeTabX + 4.0f, 32.0f, tabWidth - 8.0f, 2.5f);

    // 3. Specific Module Visualizations using stored responsive bounds
    if (m_activeTab == 1) { // EQ
        paintEqCurve(g, m_eqCurveArea);
    } else if (m_activeTab == 2) { // Compressor
        paintCompMeter(g, m_compMeterArea);
    } else if (m_activeTab == 4 && m_telemetry.haasPhaseWarning) {
        // Mono-compatibility caution badge
        auto badgeArea = juce::Rectangle<float>(m_activeBayArea.getX() + 180.0f, m_activeBayArea.getY() + 6.0f,
                                                m_activeBayArea.getWidth() - 190.0f, 24.0f);
        g.setColour(p.indicatorWarning.withAlpha(0.2f));
        g.fillRoundedRectangle(badgeArea, 3.0f);
        g.setColour(p.indicatorWarning);
        g.drawRoundedRectangle(badgeArea, 3.0f, 1.0f);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("MONO CAUTION: DELAY > 18ms", badgeArea, juce::Justification::centred);
    }
}
