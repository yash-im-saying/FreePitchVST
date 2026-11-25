#include "PluginEditor.h"
#include <cmath>

FreePitchAudioProcessorEditor::FreePitchAudioProcessorEditor(FreePitchAudioProcessor& p)
    : AudioProcessorEditor(&p),
      m_processor(p),
      m_efxRack(p.getAPVTS(), p.getEFXRack()) {

    juce::LookAndFeel::setDefaultLookAndFeel(&m_lookAndFeel);

    // ========================================================================
    // 1. Header Bar Components
    // ========================================================================
    // Left: Wordmark & VST3 Badge
    m_titleLabel.setText("FREEPITCH", juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 21.0f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchColors::textPrimary);
    addAndMakeVisible(m_titleLabel);

    m_vst3Badge.setText("VST3", juce::dontSendNotification);
    m_vst3Badge.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 9.5f, juce::Font::bold));
    m_vst3Badge.setColour(juce::Label::textColourId, FreePitchColors::textSecondary);
    m_vst3Badge.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_vst3Badge);

    // Center: Preset Selector Pill
    m_presetSelector = std::make_unique<HeaderPillSelector>("");
    m_presetSelector->setTooltip("Preset Library: Audition factory vocal styles or manage your user presets.");
    updatePresetBox();
    m_presetSelector->getComboBox().onChange = [this]() {
        int idx = m_presetSelector->getComboBox().getSelectedId() - 1;
        if (idx >= 0) {
            m_processor.getPresetManager().loadPreset(idx);
            m_presetSelector->repaint();
        }
    };
    addAndMakeVisible(m_presetSelector.get());

    // Center: Key & Scale Selector Pill (e.g. "C MINOR" with subtitle "KEY/SCALE")
    m_keyScaleSelector = std::make_unique<HeaderPillSelector>("KEY/SCALE");
    m_keyScaleSelector->setTooltip("Key & Scale: Root note and musical scale for real-time pitch quantization.");

    auto& keyBox = m_keyScaleSelector->getComboBox();
    const char* roots[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const char* scales[] = { "Chromatic", "Major", "Minor", "Harmonic Min", "Melodic Min", "Pentatonic Maj", "Pentatonic Min", "Custom" };

    // Fill combo with common key/scale pairs for quick dropdown
    for (int k = 0; k < 12; ++k) {
        for (int s = 0; s < 8; ++s) {
            keyBox.addItem(juce::String(roots[k]) + " " + juce::String(scales[s]).toUpperCase(), (k * 8 + s) + 1);
        }
    }

    auto updateKeyScaleDisplay = [this, roots, scales]() {
        auto* keyParam = dynamic_cast<juce::AudioParameterChoice*>(m_processor.getAPVTS().getParameter(FreePitchID::keyRoot));
        auto* scaleParam = dynamic_cast<juce::AudioParameterChoice*>(m_processor.getAPVTS().getParameter(FreePitchID::scaleType));
        int k = keyParam ? keyParam->getIndex() : 0;
        int s = scaleParam ? scaleParam->getIndex() : 0;
        k = std::clamp(k, 0, 11);
        s = std::clamp(s, 0, 7);
        m_keyScaleSelector->getComboBox().setSelectedId(k * 8 + s + 1, juce::dontSendNotification);
        m_keyScaleSelector->repaint();
    };

    keyBox.onChange = [this, updateKeyScaleDisplay]() {
        int id = m_keyScaleSelector->getComboBox().getSelectedId() - 1;
        if (id >= 0) {
            int k = id / 8;
            int s = id % 8;
            if (auto* keyParam = m_processor.getAPVTS().getParameter(FreePitchID::keyRoot))
                keyParam->setValueNotifyingHost(keyParam->convertTo0to1(static_cast<float>(k)));
            if (auto* scaleParam = m_processor.getAPVTS().getParameter(FreePitchID::scaleType))
                scaleParam->setValueNotifyingHost(scaleParam->convertTo0to1(static_cast<float>(s)));
        }
    };
    updateKeyScaleDisplay();
    addAndMakeVisible(m_keyScaleSelector.get());

    // Right: Latency Mode Toggle (TRACK vs LOOKAHEAD)
    m_latencyToggle = std::make_unique<SegmentedLatencyToggle>();
    m_lookaheadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::lookaheadMode, m_latencyToggle->getButton());
    addAndMakeVisible(m_latencyToggle.get());

    // Right: Vocal Mode Selector Pill
    m_vocalModeSelector = std::make_unique<HeaderPillSelector>("VOCAL MODE");
    m_vocalModeSelector->setTooltip("Vocal Range Mode: Limits pitch tracking search to eliminate octave errors.");
    for (int i = 0; i < static_cast<int>(VocalMode::NumModes); ++i) {
        juce::String name = g_vocalModeBounds[i].name;
        int spaceIdx = name.indexOfChar(' ');
        if (spaceIdx > 0) name = name.substring(0, spaceIdx);
        m_vocalModeSelector->getComboBox().addItem(name.toUpperCase(), i + 1);
    }
    m_vocalModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::vocalMode, m_vocalModeSelector->getComboBox());
    addAndMakeVisible(m_vocalModeSelector.get());

    // Right: Master Bypass Button
    m_bypassButton = std::make_unique<CircularBypassButton>();
    m_masterEfxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::masterEfxActive, m_bypassButton->getButton());
    addAndMakeVisible(m_bypassButton.get());

    // ========================================================================
    // 2. Main Pitch Engine (Left 30% / Center 40% / Right 30%)
    // ========================================================================
    // Left Column: RETUNE SPEED, HUMANIZE, FLEX-TUNE + CHOOSY TRACKING
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::retuneSpeed)) {
        m_retuneSpeedKnob = std::make_unique<RotaryKnob>(*param, "Retune Speed", "ms");
        m_retuneSpeedKnob->setHero(true);
        m_retuneSpeedKnob->setTooltipText("Retune Speed: Correction response time (0 ms = snap, 20-40 ms = modern pop, 80+ ms = natural).");
        m_retuneSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::retuneSpeed, m_retuneSpeedKnob->getSlider());
        addAndMakeVisible(m_retuneSpeedKnob.get());
    }

    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::humanize)) {
        m_humanizeKnob = std::make_unique<RotaryKnob>(*param, "Humanize", "%");
        m_humanizeKnob->setHero(true);
        m_humanizeKnob->setTooltipText("Humanize: Preserves natural vocal vibrato on sustained notes.");
        m_humanizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::humanize, m_humanizeKnob->getSlider());
        addAndMakeVisible(m_humanizeKnob.get());
    }

    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::flexTune)) {
        m_flexTuneKnob = std::make_unique<RotaryKnob>(*param, "Flex-Tune", "%");
        m_flexTuneKnob->setHero(true);
        m_flexTuneKnob->setTooltipText("Flex-Tune: Elastic dead-band allowing expressive vocal scoops and slides.");
        m_flexTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::flexTune, m_flexTuneKnob->getSlider());
        addAndMakeVisible(m_flexTuneKnob.get());
    }

    m_choosyTrackingPill = std::make_unique<NeumorphicTogglePill>(
        "Choosy Tracking", "Choosy Tracking: Tightens tracking thresholds so unvoiced breaths pass naturally.");
    m_choosyTrackingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::choosyTracking, m_choosyTrackingPill->getToggleButton());
    addAndMakeVisible(m_choosyTrackingPill.get());

    // Center Focal Point: Circular Pitch Meter
    addAndMakeVisible(m_pitchMeter);

    // Right Column: FORMANT, TRANSPOSE + FORMANT LOCK
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::formantShift)) {
        m_formantKnob = std::make_unique<RotaryKnob>(*param, "Formant", "st");
        m_formantKnob->setHero(true);
        m_formantKnob->setTooltipText("Formant: Morphs vocal tract resonances independently (-12 to +12 st).");
        m_formantAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::formantShift, m_formantKnob->getSlider());
        addAndMakeVisible(m_formantKnob.get());
    }

    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::pitchShift)) {
        m_transposeKnob = std::make_unique<RotaryKnob>(*param, "Transpose", "st");
        m_transposeKnob->setHero(true);
        m_transposeKnob->setTooltipText("Transpose: Transposes corrected pitch in semitones (-12 to +12 st).");
        m_transposeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::pitchShift, m_transposeKnob->getSlider());
        addAndMakeVisible(m_transposeKnob.get());
    }

    m_formantLockPill = std::make_unique<NeumorphicTogglePill>(
        "Formant Lock", "Formant Lock: Preserves natural vocal resonances during pitch transposition.");
    m_formantsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::formantsOn, m_formantLockPill->getToggleButton());
    addAndMakeVisible(m_formantLockPill.get());

    // ========================================================================
    // 3. Modular Post-Pitch EFX Rack
    // ========================================================================
    addAndMakeVisible(m_efxRack);

    // ========================================================================
    // 4. Interactive Piano Keyboard
    // ========================================================================
    addAndMakeVisible(m_pianoKeyboard);
    m_pianoKeyboard.onCustomMaskChanged = [this](uint32_t newMask) {
        m_processor.setCustomScaleMask(newMask);
    };

    // ========================================================================
    // Window Architecture & Scaling (1120 x 630 default, 16:9 Aspect Ratio)
    // ========================================================================
    setSize(1120, 630);
    setResizable(true, true);
    setResizeLimits(920, 518, 1920, 1080);
    getConstrainer()->setFixedAspectRatio(1120.0 / 630.0);

    startTimerHz(60); // 60 FPS visual telemetry
}

FreePitchAudioProcessorEditor::~FreePitchAudioProcessorEditor() {
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void FreePitchAudioProcessorEditor::updatePresetBox() {
    auto& box = m_presetSelector->getComboBox();
    box.clear(juce::dontSendNotification);
    int num = m_processor.getPresetManager().getNumPresets();
    for (int i = 0; i < num; ++i) {
        box.addItem(m_processor.getPresetManager().getPresetName(i), i + 1);
    }
    box.setSelectedId(m_processor.getPresetManager().getCurrentPresetIndex() + 1, juce::dontSendNotification);
}

void FreePitchAudioProcessorEditor::timerCallback() {
    auto telemetry = m_processor.getLatestTelemetry();

    // 1. Update Center Pitch Radar HUD
    m_pitchMeter.updateTelemetry(telemetry);

    // 2. Update EFX Rack Telemetry
    m_efxRack.updateTelemetry(telemetry);

    // 3. Update Piano Keyboard
    int vocalModeIdx = m_vocalModeSelector->getComboBox().getSelectedId() - 1;
    if (vocalModeIdx < 0) vocalModeIdx = 3; // Tenor default
    m_pianoKeyboard.setVocalMode(static_cast<VocalMode>(vocalModeIdx));

    auto* keyParam = dynamic_cast<juce::AudioParameterChoice*>(m_processor.getAPVTS().getParameter(FreePitchID::keyRoot));
    auto* scaleParam = dynamic_cast<juce::AudioParameterChoice*>(m_processor.getAPVTS().getParameter(FreePitchID::scaleType));
    int root = keyParam ? keyParam->getIndex() : 0;
    int scale = scaleParam ? scaleParam->getIndex() : 0;

    m_pianoKeyboard.setScale(root, static_cast<ScaleType>(scale), telemetry.customScaleMask);
    m_pianoKeyboard.updateLiveNotes(telemetry.detectedMidi, telemetry.targetMidi, telemetry.isVoiced);
}

void FreePitchAudioProcessorEditor::paint(juce::Graphics& g) {
    using namespace FreePitchColors;
    auto bounds = getLocalBounds().toFloat();

    // 1. Studio Chassis Background (#16181C)
    g.fillAll(bgChassis);

    // 2. Top Header Bar Area
    float headerH = bounds.getHeight() * 0.10f;
    auto headerArea = bounds.removeFromTop(headerH);

    // Subtle 1px separation hairline under header
    g.setColour(rimHighlight.withAlpha(0.40f));
    g.drawLine(0.0f, headerArea.getBottom(), bounds.getWidth(), headerArea.getBottom(), 1.0f);

    // VST3 badge rounded border
    if (m_vst3Badge.isVisible()) {
        auto badgeBounds = m_vst3Badge.getBounds().toFloat();
        g.setColour(cavityRecessed);
        g.fillRoundedRectangle(badgeBounds, 4.0f);
        g.setColour(rimHighlight.withAlpha(0.6f));
        g.drawRoundedRectangle(badgeBounds.reduced(0.5f), 4.0f, 1.0f);
    }
}

void FreePitchAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    if (bounds.isEmpty()) return;

    int totalW = bounds.getWidth();
    int totalH = bounds.getHeight();

    // ========================================================================
    // 1. Top Header Bar (~10% height: ~63px at 630h)
    // ========================================================================
    int headerH = static_cast<int>(totalH * 0.10f);
    auto headerArea = bounds.removeFromTop(headerH).reduced(16, 8);

    // Left: Wordmark & VST3 Badge
    int titleW = 125;
    m_titleLabel.setBounds(headerArea.removeFromLeft(titleW).withTrimmedBottom(4));
    m_vst3Badge.setBounds(headerArea.removeFromLeft(38).reduced(0, 8));

    // Right side: Master Bypass Button, Vocal Mode Selector, Latency Toggle
    int bypassSize = std::clamp(headerArea.getHeight(), 26, 36);
    m_bypassButton->setBounds(headerArea.removeFromRight(bypassSize).reduced(0, 1));
    headerArea.removeFromRight(14); // gutter

    int vocalW = std::clamp(totalW / 9, 90, 115);
    m_vocalModeSelector->setBounds(headerArea.removeFromRight(vocalW));
    headerArea.removeFromRight(14); // gutter

    int latencyW = std::clamp(totalW / 8, 120, 150);
    m_latencyToggle->setBounds(headerArea.removeFromRight(latencyW));
    headerArea.removeFromRight(20); // gutter

    // Center: Preset Selector Pill and Key/Scale Selector Pill
    int centerAvailW = headerArea.getWidth();
    int pillW = std::clamp(centerAvailW / 2 - 12, 100, 160);
    int centerStartX = headerArea.getX() + (centerAvailW - (pillW * 2 + 16)) / 2;

    m_presetSelector->setBounds(centerStartX, headerArea.getY(), pillW, headerArea.getHeight());
    m_keyScaleSelector->setBounds(centerStartX + pillW + 16, headerArea.getY(), pillW, headerArea.getHeight());

    // ========================================================================
    // 4. Bottom Piano Keyboard (~18% height: ~114px at 630h)
    // ========================================================================
    int pianoH = static_cast<int>(totalH * 0.18f);
    m_pianoKeyboard.setBounds(bounds.removeFromBottom(pianoH));

    // ========================================================================
    // 3. Modular Post-Pitch EFX Rack (~24% height: ~151px at 630h)
    // ========================================================================
    int rackH = static_cast<int>(totalH * 0.24f);
    m_efxRack.setBounds(bounds.removeFromBottom(rackH).reduced(16, 4));

    // ========================================================================
    // 2. Main Pitch Engine (~48% height: ~302px at 630h) - 30% / 40% / 30% Split
    // ========================================================================
    auto engineArea = bounds.reduced(16, 8);

    int colLeftW = static_cast<int>(engineArea.getWidth() * 0.30f);
    int colRightW = static_cast<int>(engineArea.getWidth() * 0.30f);

    auto leftArea = engineArea.removeFromLeft(colLeftW).reduced(8, 6);
    auto rightArea = engineArea.removeFromRight(colRightW).reduced(8, 6);
    auto centerArea = engineArea.reduced(8, 4);

    // Left Column: Row of 3 primary knobs + Choosy Tracking pill below
    int pillHeight = 36;
    auto leftBottom = leftArea.removeFromBottom(pillHeight);
    m_choosyTrackingPill->setBounds(leftBottom.getCentreX() - 40, leftBottom.getY(), 80, pillHeight);

    int leftKnobW = leftArea.getWidth() / 3;
    if (m_retuneSpeedKnob) m_retuneSpeedKnob->setBounds(leftArea.getX(), leftArea.getY(), leftKnobW, leftArea.getHeight());
    if (m_humanizeKnob)    m_humanizeKnob->setBounds(leftArea.getX() + leftKnobW, leftArea.getY(), leftKnobW, leftArea.getHeight());
    if (m_flexTuneKnob)    m_flexTuneKnob->setBounds(leftArea.getX() + leftKnobW * 2, leftArea.getY(), leftKnobW, leftArea.getHeight());

    // Center Focal Point: Circular Pitch Meter
    m_pitchMeter.setBounds(centerArea);

    // Right Column: Row of 2 secondary knobs + Formant Lock pill below
    auto rightBottom = rightArea.removeFromBottom(pillHeight);
    m_formantLockPill->setBounds(rightBottom.getCentreX() - 40, rightBottom.getY(), 80, pillHeight);

    int rightKnobW = rightArea.getWidth() / 2;
    if (m_formantKnob)   m_formantKnob->setBounds(rightArea.getX(), rightArea.getY(), rightKnobW, rightArea.getHeight());
    if (m_transposeKnob) m_transposeKnob->setBounds(rightArea.getX() + rightKnobW, rightArea.getY(), rightKnobW, rightArea.getHeight());
}
