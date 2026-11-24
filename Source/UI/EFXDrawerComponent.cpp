#include "EFXDrawerComponent.h"
#include <cmath>

EFXDrawerComponent::EFXDrawerComponent(juce::AudioProcessorValueTreeState& apvts, EFXRack& efxRack)
    : m_apvts(apvts), m_efxRack(efxRack) {

    // Setup 6 EFX rack slots
    setupSlot(m_eqSlot, "4-BAND SCULPT EQ", FreePitchID::eqEnabled, 1);
    setupSlot(m_compSlot, "VOCAL FET/OPTO COMP", FreePitchID::compEnabled, 2);
    setupSlot(m_flangerSlot, "STEREO FLANGER", FreePitchID::flangerEnabled, 3);
    setupSlot(m_chorusSlot, "CHORUS / HAAS", FreePitchID::chorusEnabled, 4);
    setupSlot(m_distSlot, "WAVESHAPING SATURATION", FreePitchID::distEnabled, 5);
    setupSlot(m_delaySlot, "SPACE DELAY / FREQ ECHO", FreePitchID::delayEnabled, 6);

    // --- Instantiate EQ Knobs ---
    if (auto* p = m_apvts.getParameter(FreePitchID::eqHpfFreq)) {
        m_eqHpfKnob = std::make_unique<RotaryKnob>(*p, "HPF", "Hz");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqHpfFreq, m_eqHpfKnob->getSlider()));
        addChildComponent(m_eqHpfKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqLowMidGain)) {
        m_eqLowMidGainKnob = std::make_unique<RotaryKnob>(*p, "L-Mid", "dB");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqLowMidGain, m_eqLowMidGainKnob->getSlider()));
        addChildComponent(m_eqLowMidGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqHighMidGain)) {
        m_eqHighMidGainKnob = std::make_unique<RotaryKnob>(*p, "H-Mid", "dB");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqHighMidGain, m_eqHighMidGainKnob->getSlider()));
        addChildComponent(m_eqHighMidGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqAirGain)) {
        m_eqAirGainKnob = std::make_unique<RotaryKnob>(*p, "Air", "dB");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqAirGain, m_eqAirGainKnob->getSlider()));
        addChildComponent(m_eqAirGainKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::eqOutGain)) {
        m_eqOutGainKnob = std::make_unique<RotaryKnob>(*p, "Gain", "dB");
        m_eqAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::eqOutGain, m_eqOutGainKnob->getSlider()));
        addChildComponent(m_eqOutGainKnob.get());
    }

    // --- Instantiate Comp Knobs ---
    if (auto* p = m_apvts.getParameter(FreePitchID::compThreshold)) {
        m_compThreshKnob = std::make_unique<RotaryKnob>(*p, "Thresh", "dB");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compThreshold, m_compThreshKnob->getSlider()));
        addChildComponent(m_compThreshKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compRatio)) {
        m_compRatioKnob = std::make_unique<RotaryKnob>(*p, "Ratio", "");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compRatio, m_compRatioKnob->getSlider()));
        addChildComponent(m_compRatioKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compAttack)) {
        m_compAttackKnob = std::make_unique<RotaryKnob>(*p, "Attack", "ms");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compAttack, m_compAttackKnob->getSlider()));
        addChildComponent(m_compAttackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compRelease)) {
        m_compReleaseKnob = std::make_unique<RotaryKnob>(*p, "Release", "ms");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compRelease, m_compReleaseKnob->getSlider()));
        addChildComponent(m_compReleaseKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::compMakeup)) {
        m_compMakeupKnob = std::make_unique<RotaryKnob>(*p, "Makeup", "dB");
        m_compAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::compMakeup, m_compMakeupKnob->getSlider()));
        addChildComponent(m_compMakeupKnob.get());
    }

    // --- Instantiate Flanger Knobs ---
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerRate)) {
        m_flangerRateKnob = std::make_unique<RotaryKnob>(*p, "Rate", "Hz");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerRate, m_flangerRateKnob->getSlider()));
        addChildComponent(m_flangerRateKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerDepth)) {
        m_flangerDepthKnob = std::make_unique<RotaryKnob>(*p, "Depth", "%");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerDepth, m_flangerDepthKnob->getSlider()));
        addChildComponent(m_flangerDepthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerFeedback)) {
        m_flangerFeedbackKnob = std::make_unique<RotaryKnob>(*p, "Fdbk", "%");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerFeedback, m_flangerFeedbackKnob->getSlider()));
        addChildComponent(m_flangerFeedbackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::flangerMix)) {
        m_flangerMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_flangerAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::flangerMix, m_flangerMixKnob->getSlider()));
        addChildComponent(m_flangerMixKnob.get());
    }

    // --- Instantiate Chorus / Haas Knobs ---
    m_chorusModeBox.addItem("Chorus", 1);
    m_chorusModeBox.addItem("Haas Widener", 2);
    m_chorusModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::chorusMode, m_chorusModeBox);
    m_chorusModeBox.onChange = [this]() {
        updateChorusControlsVisibility();
        resized();
        repaint();
    };
    addChildComponent(m_chorusModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::chorusRate)) {
        m_chorusRateKnob = std::make_unique<RotaryKnob>(*p, "Rate", "Hz");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusRate, m_chorusRateKnob->getSlider()));
        addChildComponent(m_chorusRateKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::chorusDepth)) {
        m_chorusDepthKnob = std::make_unique<RotaryKnob>(*p, "Depth", "%");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusDepth, m_chorusDepthKnob->getSlider()));
        addChildComponent(m_chorusDepthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::haasDelay)) {
        m_haasDelayKnob = std::make_unique<RotaryKnob>(*p, "Delay", "ms");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::haasDelay, m_haasDelayKnob->getSlider()));
        addChildComponent(m_haasDelayKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::haasWidth)) {
        m_haasWidthKnob = std::make_unique<RotaryKnob>(*p, "Width", "%");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::haasWidth, m_haasWidthKnob->getSlider()));
        addChildComponent(m_haasWidthKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::chorusMix)) {
        m_chorusMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_chorusAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::chorusMix, m_chorusMixKnob->getSlider()));
        addChildComponent(m_chorusMixKnob.get());
    }

    // --- Instantiate Distortion Knobs ---
    m_distModeBox.addItem("Warm Tube", 1);
    m_distModeBox.addItem("Tape", 2);
    m_distModeBox.addItem("Hard Clip", 3);
    m_distModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::distMode, m_distModeBox);
    addChildComponent(m_distModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::distDrive)) {
        m_distDriveKnob = std::make_unique<RotaryKnob>(*p, "Drive", "%");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distDrive, m_distDriveKnob->getSlider()));
        addChildComponent(m_distDriveKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::distTone)) {
        m_distToneKnob = std::make_unique<RotaryKnob>(*p, "Tone", "Hz");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distTone, m_distToneKnob->getSlider()));
        addChildComponent(m_distToneKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::distMix)) {
        m_distMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_distAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::distMix, m_distMixKnob->getSlider()));
        addChildComponent(m_distMixKnob.get());
    }

    // --- Instantiate Delay Knobs (H-Delay, Supermassive, FreqEcho) ---
    m_delayModeBox.addItem("Stereo Echo", 1);
    m_delayModeBox.addItem("Ping-Pong Echo", 2);
    m_delayModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::delayMode, m_delayModeBox);
    addChildComponent(m_delayModeBox);

    if (auto* p = m_apvts.getParameter(FreePitchID::delayTime)) {
        m_delayTimeKnob = std::make_unique<RotaryKnob>(*p, "Time", "ms");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayTime, m_delayTimeKnob->getSlider()));
        addChildComponent(m_delayTimeKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayFeedback)) {
        m_delayFeedbackKnob = std::make_unique<RotaryKnob>(*p, "Feedback", "%");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayFeedback, m_delayFeedbackKnob->getSlider()));
        addChildComponent(m_delayFeedbackKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayFreqShift)) {
        m_delayFreqShiftKnob = std::make_unique<RotaryKnob>(*p, "Shift", "Hz");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayFreqShift, m_delayFreqShiftKnob->getSlider()));
        addChildComponent(m_delayFreqShiftKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayDiffusion)) {
        m_delayDiffusionKnob = std::make_unique<RotaryKnob>(*p, "Diffuse", "%");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayDiffusion, m_delayDiffusionKnob->getSlider()));
        addChildComponent(m_delayDiffusionKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayTone)) {
        m_delayToneKnob = std::make_unique<RotaryKnob>(*p, "Tone", "Hz");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayTone, m_delayToneKnob->getSlider()));
        addChildComponent(m_delayToneKnob.get());
    }
    if (auto* p = m_apvts.getParameter(FreePitchID::delayMix)) {
        m_delayMixKnob = std::make_unique<RotaryKnob>(*p, "Mix", "%");
        m_delayAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, FreePitchID::delayMix, m_delayMixKnob->getSlider()));
        addChildComponent(m_delayMixKnob.get());
    }

    setActiveDrawer(1); // EQ open initially
}

void EFXDrawerComponent::setupSlot(SlotButton& slot, const juce::String& name, const char* paramID, int slotIndex) {
    slot.powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(m_apvts, paramID, slot.powerBtn);
    addAndMakeVisible(slot.powerBtn);

    slot.editBtn.setButtonText(name);
    slot.editBtn.onClick = [this, &slot, slotIndex]() {
        // Auto-power ON when opening a drawer for an effect that was powered off
        if (m_activeDrawer != slotIndex && !slot.powerBtn.getToggleState()) {
            slot.powerBtn.setToggleState(true, juce::sendNotification);
        }
        setActiveDrawer(m_activeDrawer == slotIndex ? 0 : slotIndex); // toggle expand/collapse
    };
    addAndMakeVisible(slot.editBtn);
}

void EFXDrawerComponent::setActiveDrawer(int index) {
    m_activeDrawer = index;

    // Toggle visibility of inline components
    auto setVis = [](juce::Component* c, bool v) { if (c) c->setVisible(v); };

    setVis(m_eqHpfKnob.get(), m_activeDrawer == 1);
    setVis(m_eqLowMidGainKnob.get(), m_activeDrawer == 1);
    setVis(m_eqHighMidGainKnob.get(), m_activeDrawer == 1);
    setVis(m_eqAirGainKnob.get(), m_activeDrawer == 1);
    setVis(m_eqOutGainKnob.get(), m_activeDrawer == 1);

    setVis(m_compThreshKnob.get(), m_activeDrawer == 2);
    setVis(m_compRatioKnob.get(), m_activeDrawer == 2);
    setVis(m_compAttackKnob.get(), m_activeDrawer == 2);
    setVis(m_compReleaseKnob.get(), m_activeDrawer == 2);
    setVis(m_compMakeupKnob.get(), m_activeDrawer == 2);

    setVis(m_flangerRateKnob.get(), m_activeDrawer == 3);
    setVis(m_flangerDepthKnob.get(), m_activeDrawer == 3);
    setVis(m_flangerFeedbackKnob.get(), m_activeDrawer == 3);
    setVis(m_flangerMixKnob.get(), m_activeDrawer == 3);

    updateChorusControlsVisibility();

    m_distModeBox.setVisible(m_activeDrawer == 5);
    setVis(m_distDriveKnob.get(), m_activeDrawer == 5);
    setVis(m_distToneKnob.get(), m_activeDrawer == 5);
    setVis(m_distMixKnob.get(), m_activeDrawer == 5);

    bool isDelay = (m_activeDrawer == 6);
    m_delayModeBox.setVisible(isDelay);
    setVis(m_delayTimeKnob.get(), isDelay);
    setVis(m_delayFeedbackKnob.get(), isDelay);
    setVis(m_delayFreqShiftKnob.get(), isDelay);
    setVis(m_delayDiffusionKnob.get(), isDelay);
    setVis(m_delayToneKnob.get(), isDelay);
    setVis(m_delayMixKnob.get(), isDelay);

    resized();
    repaint();
}

void EFXDrawerComponent::updateChorusControlsVisibility() {
    bool isChorusDrawer = (m_activeDrawer == 4);
    m_chorusModeBox.setVisible(isChorusDrawer);
    int mode = m_chorusModeBox.getSelectedId(); // 1 = Chorus, 2 = Haas
    auto setVis = [](juce::Component* c, bool v) { if (c) c->setVisible(v); };

    setVis(m_chorusRateKnob.get(), isChorusDrawer && mode == 1);
    setVis(m_chorusDepthKnob.get(), isChorusDrawer && mode == 1);
    setVis(m_haasDelayKnob.get(), isChorusDrawer && mode == 2);
    setVis(m_haasWidthKnob.get(), isChorusDrawer && mode == 2);
    setVis(m_chorusMixKnob.get(), isChorusDrawer);
}

void EFXDrawerComponent::updateTelemetry(const PitchTelemetry& telemetry) {
    m_telemetry = telemetry;
    if (m_activeDrawer > 0) {
        repaint();
    }
}

void EFXDrawerComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    int slotHeight = 26;

    // Layout slots from top
    auto layoutSlot = [&](SlotButton& slot, int slotIndex) {
        auto row = bounds.removeFromTop(slotHeight);
        slot.powerBtn.setBounds(row.removeFromLeft(28));
        slot.editBtn.setBounds(row);
        bounds.removeFromTop(3);

        if (m_activeDrawer == slotIndex) {
            auto drawerArea = bounds.removeFromTop(130);
            bounds.removeFromTop(4);

            if (slotIndex == 1) { // EQ
                auto curveArea = drawerArea.removeFromTop(55);
                int kw = drawerArea.getWidth() / 5;
                if (m_eqHpfKnob) m_eqHpfKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_eqLowMidGainKnob) m_eqLowMidGainKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_eqHighMidGainKnob) m_eqHighMidGainKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_eqAirGainKnob) m_eqAirGainKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_eqOutGainKnob) m_eqOutGainKnob->setBounds(drawerArea);
            } else if (slotIndex == 2) { // Comp
                auto meterArea = drawerArea.removeFromTop(55);
                int kw = drawerArea.getWidth() / 5;
                if (m_compThreshKnob) m_compThreshKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_compRatioKnob) m_compRatioKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_compAttackKnob) m_compAttackKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_compReleaseKnob) m_compReleaseKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_compMakeupKnob) m_compMakeupKnob->setBounds(drawerArea);
            } else if (slotIndex == 3) { // Flanger
                int kw = drawerArea.getWidth() / 4;
                if (m_flangerRateKnob) m_flangerRateKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_flangerDepthKnob) m_flangerDepthKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_flangerFeedbackKnob) m_flangerFeedbackKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_flangerMixKnob) m_flangerMixKnob->setBounds(drawerArea);
            } else if (slotIndex == 4) { // Chorus/Haas
                auto topArea = drawerArea.removeFromTop(32);
                m_chorusModeBox.setBounds(topArea.reduced(2));
                int kw = drawerArea.getWidth() / 3;
                int mode = m_chorusModeBox.getSelectedId();
                if (mode == 1) { // Chorus mode: Rate, Depth, Mix
                    if (m_chorusRateKnob) m_chorusRateKnob->setBounds(drawerArea.removeFromLeft(kw));
                    if (m_chorusDepthKnob) m_chorusDepthKnob->setBounds(drawerArea.removeFromLeft(kw));
                    if (m_chorusMixKnob) m_chorusMixKnob->setBounds(drawerArea);
                } else { // Haas mode: Delay, Width, Mix
                    if (m_haasDelayKnob) m_haasDelayKnob->setBounds(drawerArea.removeFromLeft(kw));
                    if (m_haasWidthKnob) m_haasWidthKnob->setBounds(drawerArea.removeFromLeft(kw));
                    if (m_chorusMixKnob) m_chorusMixKnob->setBounds(drawerArea);
                }
            } else if (slotIndex == 5) { // Dist
                auto topArea = drawerArea.removeFromTop(32);
                m_distModeBox.setBounds(topArea.reduced(2));
                int kw = drawerArea.getWidth() / 3;
                if (m_distDriveKnob) m_distDriveKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_distToneKnob) m_distToneKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_distMixKnob) m_distMixKnob->setBounds(drawerArea);
            } else if (slotIndex == 6) { // Delay / Echo
                auto topArea = drawerArea.removeFromTop(32);
                m_delayModeBox.setBounds(topArea.removeFromLeft(140).reduced(2));
                if (m_delayToneKnob) m_delayToneKnob->setBounds(topArea.removeFromRight(80));

                int kw = drawerArea.getWidth() / 5;
                if (m_delayTimeKnob) m_delayTimeKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_delayFeedbackKnob) m_delayFeedbackKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_delayFreqShiftKnob) m_delayFreqShiftKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_delayDiffusionKnob) m_delayDiffusionKnob->setBounds(drawerArea.removeFromLeft(kw));
                if (m_delayMixKnob) m_delayMixKnob->setBounds(drawerArea);
            }
        }
    };

    layoutSlot(m_eqSlot, 1);
    layoutSlot(m_compSlot, 2);
    layoutSlot(m_flangerSlot, 3);
    layoutSlot(m_chorusSlot, 4);
    layoutSlot(m_distSlot, 5);
    layoutSlot(m_delaySlot, 6);
}

void EFXDrawerComponent::paintEqCurve(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(FreePitchLookAndFeel::kBackgroundDark);
    g.fillRoundedRectangle(area, 3.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(area, 3.0f, 1.0f);

    // Center 0 dB reference line
    float midY = area.getCentreY();
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawHorizontalLine(static_cast<int>(midY), area.getX(), area.getRight());

    // Update EFXRack directly from live APVTS atomics so curve reflects changes instantly even if transport is paused
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

    // Evaluate response at 40 logarithmic frequency points from 20 Hz to 20 kHz
    juce::Path curve;
    bool started = false;
    float minF = 20.0f;
    float maxF = 20000.0f;

    for (int i = 0; i < 40; ++i) {
        float frac = static_cast<float>(i) / 39.0f;
        float freq = minF * std::pow(maxF / minF, frac);
        float db = m_efxRack.getEqResponseAt(freq);

        float px = area.getX() + frac * area.getWidth();
        // +-12 dB mapped to full height
        float py = midY - (db / 12.0f) * (area.getHeight() * 0.45f);
        py = std::clamp(py, area.getY() + 2.0f, area.getBottom() - 2.0f);

        if (!started) {
            curve.startNewSubPath(px, py);
            started = true;
        } else {
            curve.lineTo(px, py);
        }
    }

    g.setColour(eqOn ? FreePitchLookAndFeel::kAccentAmber : FreePitchLookAndFeel::kTextDim);
    g.strokePath(curve, juce::PathStrokeType(1.5f));
}

void EFXDrawerComponent::paintCompMeter(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(FreePitchLookAndFeel::kBackgroundDark);
    g.fillRoundedRectangle(area, 3.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(area, 3.0f, 1.0f);

    // Live Gain Reduction meter
    float grDb = m_telemetry.compressorGR_dB;
    float grFrac = std::clamp(grDb / 20.0f, 0.0f, 1.0f); // 0 to 20 dB GR

    auto barArea = area.reduced(4.0f);
    barArea.setWidth(barArea.getWidth() * (1.0f - grFrac));

    g.setColour(FreePitchLookAndFeel::kAccentAmber);
    g.fillRect(barArea.removeFromRight(area.getWidth() * grFrac));

    g.setColour(FreePitchLookAndFeel::kTextWhite);
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));
    juce::String grText = "GR: -" + juce::String(grDb, 1) + " dB";
    g.drawText(grText, area, juce::Justification::centred);
}

void EFXDrawerComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // If active drawer is EQ, paint live EQ response curve
    if (m_activeDrawer == 1) {
        float curveY = 26.0f + 5.0f;
        auto curveArea = juce::Rectangle<float>(bounds.getX(), curveY, bounds.getWidth(), 50.0f);
        paintEqCurve(g, curveArea);
    } else if (m_activeDrawer == 2) {
        float meterY = 26.0f * 2.0f + 8.0f;
        auto meterArea = juce::Rectangle<float>(bounds.getX(), meterY, bounds.getWidth(), 50.0f);
        paintCompMeter(g, meterArea);
    } else if (m_activeDrawer == 4 && m_telemetry.haasPhaseWarning) {
        // Draw mono-compatibility caution indicator badge
        float warnY = 26.0f * 4.0f + 14.0f;
        auto badgeArea = juce::Rectangle<float>(bounds.getX() + 6.0f, warnY + 36.0f, bounds.getWidth() - 12.0f, 16.0f);
        g.setColour(FreePitchLookAndFeel::kAccentAmber);
        g.drawRoundedRectangle(badgeArea, 2.0f, 1.0f);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText("CAUTION: DELAY > 18ms (CHECK MONO)", badgeArea, juce::Justification::centred);
    }
}
