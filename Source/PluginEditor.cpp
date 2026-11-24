#include "PluginEditor.h"
#include <cmath>

FreePitchAudioProcessorEditor::FreePitchAudioProcessorEditor(FreePitchAudioProcessor& p)
    : AudioProcessorEditor(&p),
      m_processor(p),
      m_efxDrawer(p.getAPVTS(), p.getEFXRack()) {

    juce::LookAndFeel::setDefaultLookAndFeel(&m_lookAndFeel);

    // --- Top Bar ---
    m_titleLabel.setText("FREEPITCH", juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 17.0f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextWhite);
    addAndMakeVisible(m_titleLabel);

    m_versionLabel.setText("v1.0.0", juce::dontSendNotification);
    m_versionLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    m_versionLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    addAndMakeVisible(m_versionLabel);

    // Presets
    m_prevPresetBtn.onClick = [this]() {
        m_processor.getPresetManager().previousPreset();
        updatePresetBox();
    };
    addAndMakeVisible(m_prevPresetBtn);

    m_nextPresetBtn.onClick = [this]() {
        m_processor.getPresetManager().nextPreset();
        updatePresetBox();
    };
    addAndMakeVisible(m_nextPresetBtn);

    updatePresetBox();
    m_presetBox.onChange = [this]() {
        int idx = m_presetBox.getSelectedId() - 1;
        if (idx >= 0) {
            m_processor.getPresetManager().loadPreset(idx);
        }
    };
    addAndMakeVisible(m_presetBox);

    m_savePresetBtn.onClick = [this]() {
        int nextNum = m_processor.getPresetManager().getNumPresets() + 1;
        m_processor.getPresetManager().saveUserPreset("User Preset " + juce::String(nextNum));
        updatePresetBox();
    };
    addAndMakeVisible(m_savePresetBtn);

    m_abCompareBtn.onClick = [this]() {
        m_processor.getPresetManager().toggleAB();
        bool isB = m_processor.getPresetManager().isStateB();
        m_abCompareBtn.setButtonText(isB ? "STATE B" : "STATE A");
    };
    addAndMakeVisible(m_abCompareBtn);

    // Latency / Lookahead Mode
    m_lookaheadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::lookaheadMode, m_lookaheadToggle);
    addAndMakeVisible(m_lookaheadToggle);

    m_latencyLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::plain));
    m_latencyLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_latencyLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(m_latencyLabel);

    // Key & Scale Pickers
    const char* roots[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i) {
        m_keyRootBox.addItem(roots[i], i + 1);
    }
    m_keyRootAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::keyRoot, m_keyRootBox);
    addAndMakeVisible(m_keyRootBox);

    const char* scales[] = { "Chromatic", "Major", "Natural Minor", "Harmonic Minor",
                             "Melodic Minor", "Pentatonic Major", "Pentatonic Minor", "Custom" };
    for (int i = 0; i < 8; ++i) {
        m_scaleTypeBox.addItem(scales[i], i + 1);
    }
    m_scaleTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::scaleType, m_scaleTypeBox);
    addAndMakeVisible(m_scaleTypeBox);

    // Master EFX Toggle
    m_masterEfxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::masterEfxActive, m_masterEfxToggle);
    addAndMakeVisible(m_masterEfxToggle);

    // --- Left Control Bank ---
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::retuneSpeed)) {
        m_retuneSpeedKnob = std::make_unique<RotaryKnob>(*param, "Retune Speed", "ms");
        m_retuneSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::retuneSpeed, m_retuneSpeedKnob->getSlider());
        addAndMakeVisible(m_retuneSpeedKnob.get());
    }
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::humanize)) {
        m_humanizeKnob = std::make_unique<RotaryKnob>(*param, "Humanize", "%");
        m_humanizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::humanize, m_humanizeKnob->getSlider());
        addAndMakeVisible(m_humanizeKnob.get());
    }
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::flexTune)) {
        m_flexTuneKnob = std::make_unique<RotaryKnob>(*param, "Flex-Tune", "%");
        m_flexTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::flexTune, m_flexTuneKnob->getSlider());
        addAndMakeVisible(m_flexTuneKnob.get());
    }

    // --- Center Focal Point ---
    addAndMakeVisible(m_circularMeter);

    for (int i = 0; i < static_cast<int>(VocalMode::NumModes); ++i) {
        m_vocalModeBox.addItem(g_vocalModeBounds[i].name, i + 1);
    }
    m_vocalModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::vocalMode, m_vocalModeBox);
    addAndMakeVisible(m_vocalModeBox);

    m_choosyTrackingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::choosyTracking, m_choosyTrackingToggle);
    addAndMakeVisible(m_choosyTrackingToggle);

    // --- Right Control Bank ---
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::pitchShift)) {
        m_pitchShiftKnob = std::make_unique<RotaryKnob>(*param, "Pitch Shift", "st");
        m_pitchShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::pitchShift, m_pitchShiftKnob->getSlider());
        addAndMakeVisible(m_pitchShiftKnob.get());
    }

    m_formantsTitleLabel.setText("FORMANTS", juce::dontSendNotification);
    m_formantsTitleLabel.setFont(juce::FontOptions(10.0f));
    m_formantsTitleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_formantsTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_formantsTitleLabel);

    m_formantsToggle.setButtonText("PRESERVE");
    m_formantsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::formantsOn, m_formantsToggle);
    addAndMakeVisible(m_formantsToggle);

    m_formantsSubLabel.setText("VOCAL TRACT", juce::dontSendNotification);
    m_formantsSubLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
    m_formantsSubLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextDim);
    m_formantsSubLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_formantsSubLabel);

    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::formantShift)) {
        m_formantShiftKnob = std::make_unique<RotaryKnob>(*param, "Formant Shift", "st");
        m_formantShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::formantShift, m_formantShiftKnob->getSlider());
        addAndMakeVisible(m_formantShiftKnob.get());
    }

    // EFX Rack Stack
    addAndMakeVisible(m_efxDrawer);

    // --- Bottom Piano Keyboard ---
    addAndMakeVisible(m_pianoKeyboard);
    m_pianoKeyboard.onCustomMaskChanged = [this](uint32_t newMask) {
        m_processor.setCustomScaleMask(newMask);
    };

    setSize(1060, 660);
    setResizable(true, true);
    setResizeLimits(850, 550, 1600, 1000);

    startTimerHz(60); // 60 FPS UI Ballistics
}

FreePitchAudioProcessorEditor::~FreePitchAudioProcessorEditor() {
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void FreePitchAudioProcessorEditor::updatePresetBox() {
    m_presetBox.clear(juce::dontSendNotification);
    int num = m_processor.getPresetManager().getNumPresets();
    for (int i = 0; i < num; ++i) {
        m_presetBox.addItem(m_processor.getPresetManager().getPresetName(i), i + 1);
    }
    m_presetBox.setSelectedId(m_processor.getPresetManager().getCurrentPresetIndex() + 1, juce::dontSendNotification);
}

void FreePitchAudioProcessorEditor::timerCallback() {
    auto telemetry = m_processor.getLatestTelemetry();

    // 1. Update Central Meter
    m_circularMeter.updateTelemetry(telemetry);

    // 2. Update EFX Telemetry
    m_efxDrawer.updateTelemetry(telemetry);

    // 3. Update Bottom Piano Keyboard
    int vocalModeIdx = m_vocalModeBox.getSelectedId() - 1;
    if (vocalModeIdx < 0) vocalModeIdx = 3;
    m_pianoKeyboard.setVocalMode(static_cast<VocalMode>(vocalModeIdx));

    int root = m_keyRootBox.getSelectedId() - 1;
    int scale = m_scaleTypeBox.getSelectedId() - 1;
    m_pianoKeyboard.setScale(root >= 0 ? root : 0,
                             scale >= 0 ? static_cast<ScaleType>(scale) : ScaleType::Chromatic,
                             telemetry.customScaleMask);

    m_pianoKeyboard.updateLiveNotes(telemetry.detectedMidi, telemetry.targetMidi, telemetry.isVoiced);

    // 4. Update Latency Readout
    int latencySamples = m_processor.getLatencySamples();
    float latencyMs = (static_cast<float>(latencySamples) / static_cast<float>(m_processor.getSampleRate())) * 1000.0f;
    juce::String latText = "LATENCY: " + juce::String(latencyMs, 1) + " ms (" + juce::String(latencySamples) + " smp)";
    m_latencyLabel.setText(latText, juce::dontSendNotification);

    // Smooth confidence meter
    m_smoothConfidence += 0.2f * (telemetry.voicedConfidence - m_smoothConfidence);
    m_smoothInputRms += 0.2f * (telemetry.inputRmsDb - m_smoothInputRms);
}

void FreePitchAudioProcessorEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // 1. Matte anodized charcoal background
    g.fillAll(FreePitchLookAndFeel::kBackgroundDark);

    // 2. Top Bar Panel
    auto topBarArea = bounds.removeFromTop(48.0f);
    g.setColour(FreePitchLookAndFeel::kPanelGraphite);
    g.fillRect(topBarArea);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawHorizontalLine(static_cast<int>(topBarArea.getBottom()), 0.0f, bounds.getWidth());

    // 3. Sub-Panel Outlines
    float bottomPianoHeight = 84.0f;
    auto contentArea = bounds.removeFromTop(bounds.getHeight() - bottomPianoHeight);

    // Left Bank Panel
    float leftWidth = 220.0f;
    auto leftPanel = contentArea.removeFromLeft(leftWidth).reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelGraphite);
    g.fillRoundedRectangle(leftPanel, 4.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(leftPanel, 4.0f, 1.0f);

    // Right Bank Panel
    float rightWidth = 430.0f;
    auto rightPanel = contentArea.removeFromRight(rightWidth).reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelGraphite);
    g.fillRoundedRectangle(rightPanel, 4.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(rightPanel, 4.0f, 1.0f);

    // Center Focal Zone Panel
    auto centerPanel = contentArea.reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelGraphite);
    g.fillRoundedRectangle(centerPanel, 4.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(centerPanel, 4.0f, 1.0f);

    // Section Titles
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(FreePitchLookAndFeel::kTextMuted);
    g.drawText("TUNING DYNAMICS", juce::Rectangle<float>(leftPanel.getX() + 10.0f, leftPanel.getY() + 8.0f, 140.0f, 16.0f), juce::Justification::centredLeft);
    g.drawText("PITCH TRACKER & SCALE QUANTIZER", juce::Rectangle<float>(centerPanel.getX() + 14.0f, centerPanel.getY() + 8.0f, 240.0f, 16.0f), juce::Justification::centredLeft);
    g.drawText("POST-PITCH PROCESSING & EFX", juce::Rectangle<float>(rightPanel.getX() + 12.0f, rightPanel.getY() + 8.0f, 200.0f, 16.0f), juce::Justification::centredLeft);

    // Input Gate / Confidence Meter in Right Panel
    if (!m_confidenceMeterBounds.isEmpty()) {
        float meterX = m_confidenceMeterBounds.getX();
        float meterY = m_confidenceMeterBounds.getY();
        float meterW = m_confidenceMeterBounds.getWidth();

        // Top line: label on left, status readout on right
        g.setFont(juce::FontOptions(9.5f));
        g.setColour(FreePitchLookAndFeel::kTextMuted);
        g.drawText("CONFIDENCE / VOICED GATE", juce::Rectangle<float>(meterX, meterY, 180.0f, 12.0f), juce::Justification::centredLeft);

        int confPct = static_cast<int>(std::round(std::clamp(m_smoothConfidence, 0.0f, 1.0f) * 100.0f));
        juce::String statusText = (m_smoothConfidence >= 0.58f ? "VOICED " : "UNVOICED ") + juce::String(confPct) + "%";
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
        g.setColour(m_smoothConfidence >= 0.58f ? FreePitchLookAndFeel::kAccentAmber : FreePitchLookAndFeel::kTextDim);
        g.drawText(statusText, juce::Rectangle<float>(meterX + meterW - 130.0f, meterY, 130.0f, 12.0f), juce::Justification::centredRight);

        // Bar at bottom of meter area
        float barY = meterY + 14.0f;
        float barH = 8.0f;
        juce::Rectangle<float> meterBg(meterX, barY, meterW, barH);
        g.setColour(FreePitchLookAndFeel::kBackgroundDark);
        g.fillRoundedRectangle(meterBg, 2.0f);
        g.setColour(FreePitchLookAndFeel::kPanelBorder);
        g.drawRoundedRectangle(meterBg, 2.0f, 1.0f);

        float activeW = meterW * std::clamp(m_smoothConfidence, 0.0f, 1.0f);
        if (activeW > 1.0f) {
            g.setColour(m_smoothConfidence >= 0.58f ? FreePitchLookAndFeel::kAccentAmber : FreePitchLookAndFeel::kTextDim);
            g.fillRoundedRectangle(meterX + 1.0f, barY + 1.0f, std::max(0.0f, activeW - 2.0f), barH - 2.0f, 1.5f);
        }

        // Voiced threshold marker at 58%
        float threshX = meterX + meterW * 0.58f;
        g.setColour(FreePitchLookAndFeel::kPanelBorderLight);
        g.drawVerticalLine(static_cast<int>(threshX), barY - 1.0f, barY + barH + 1.0f);
    }
}

void FreePitchAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    // 1. Top Bar Layout (height 48)
    auto topBar = bounds.removeFromTop(48).reduced(8, 6);
    m_titleLabel.setBounds(topBar.removeFromLeft(100));
    m_versionLabel.setBounds(topBar.removeFromLeft(45).withTrimmedTop(4));

    topBar.removeFromLeft(12);
    // Presets
    m_prevPresetBtn.setBounds(topBar.removeFromLeft(24));
    m_presetBox.setBounds(topBar.removeFromLeft(160));
    m_nextPresetBtn.setBounds(topBar.removeFromLeft(24));
    topBar.removeFromLeft(6);
    m_savePresetBtn.setBounds(topBar.removeFromLeft(48));
    topBar.removeFromLeft(6);
    m_abCompareBtn.setBounds(topBar.removeFromLeft(56));

    // Master EFX Toggle & Latency Readout on far right
    m_masterEfxToggle.setBounds(topBar.removeFromRight(56));
    topBar.removeFromRight(10);
    m_latencyLabel.setBounds(topBar.removeFromRight(190));
    topBar.removeFromRight(10);
    m_lookaheadToggle.setBounds(topBar.removeFromRight(105));

    // Key & Scale in Top Bar
    topBar.removeFromLeft(16);
    m_keyRootBox.setBounds(topBar.removeFromLeft(54));
    topBar.removeFromLeft(6);
    m_scaleTypeBox.setBounds(topBar.removeFromLeft(120));

    // 2. Bottom Piano Keyboard (height 84)
    auto bottomArea = bounds.removeFromBottom(84);
    m_pianoKeyboard.setBounds(bottomArea.reduced(6, 2));

    // 3. Main Center Workspace
    float leftWidth = 220.0f;
    float rightWidth = 430.0f;

    auto leftArea = bounds.removeFromLeft(static_cast<int>(leftWidth)).reduced(12, 10);
    auto rightArea = bounds.removeFromRight(static_cast<int>(rightWidth)).reduced(12, 10);
    auto centerArea = bounds.reduced(10, 10);

    // Left Bank Layout (Retune Speed, Humanize, Flex-Tune)
    leftArea.removeFromTop(24); // header spacing
    int knobH = leftArea.getHeight() / 3;
    if (m_retuneSpeedKnob) m_retuneSpeedKnob->setBounds(leftArea.removeFromTop(knobH).reduced(8, 4));
    if (m_humanizeKnob)    m_humanizeKnob->setBounds(leftArea.removeFromTop(knobH).reduced(8, 4));
    if (m_flexTuneKnob)    m_flexTuneKnob->setBounds(leftArea.reduced(8, 4));

    // Center Focal Point Layout
    centerArea.removeFromTop(24);
    auto bottomControlRow = centerArea.removeFromBottom(34).reduced(12, 3);
    m_vocalModeBox.setBounds(bottomControlRow.removeFromLeft(bottomControlRow.getWidth() - 145));
    m_choosyTrackingToggle.setBounds(bottomControlRow.reduced(6, 1));
    m_circularMeter.setBounds(centerArea.reduced(4));

    // Right Bank Layout
    rightArea.removeFromTop(24); // header spacing

    // 1. Top Controls row: Pitch Shift knob, Formants center column, Formant Shift knob
    auto topControls = rightArea.removeFromTop(96);
    int colWidth = topControls.getWidth() / 3;

    if (m_pitchShiftKnob) {
        m_pitchShiftKnob->setBounds(topControls.removeFromLeft(colWidth).reduced(2));
    }

    auto formantsCol = topControls.removeFromLeft(colWidth);
    m_formantsTitleLabel.setBounds(formantsCol.removeFromTop(18));
    m_formantsSubLabel.setBounds(formantsCol.removeFromBottom(18));
    m_formantsToggle.setBounds(formantsCol.reduced(10, 8));

    if (m_formantShiftKnob) {
        m_formantShiftKnob->setBounds(topControls.reduced(2));
    }

    // 2. Confidence / Voiced Gate meter area (height 26)
    rightArea.removeFromTop(8); // separator spacing
    m_confidenceMeterBounds = rightArea.removeFromTop(26).toFloat();

    // 3. EFX Drawer Stack
    rightArea.removeFromTop(8); // separator spacing
    m_efxDrawer.setBounds(rightArea);
}
