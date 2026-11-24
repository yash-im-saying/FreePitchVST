#include "PluginEditor.h"
#include <cmath>

FreePitchAudioProcessorEditor::FreePitchAudioProcessorEditor(FreePitchAudioProcessor& p)
    : AudioProcessorEditor(&p),
      m_processor(p),
      m_efxDrawer(p.getAPVTS(), p.getEFXRack()) {

    juce::LookAndFeel::setDefaultLookAndFeel(&m_lookAndFeel);

    // --- Top Bar Branding ---
    m_titleLabel.setText("FREEPITCH", juce::dontSendNotification);
    m_titleLabel.setFont(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::bold));
    m_titleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextWhite);
    addAndMakeVisible(m_titleLabel);

    m_subtitleLabel.setText("PRO REAL-TIME TUNING", juce::dontSendNotification);
    m_subtitleLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    m_subtitleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kAccentCyan);
    addAndMakeVisible(m_subtitleLabel);

    m_versionLabel.setText("v1.0.0", juce::dontSendNotification);
    m_versionLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
    m_versionLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextDim);
    addAndMakeVisible(m_versionLabel);

    // --- Presets Section ---
    m_prevPresetBtn.setTooltip("Previous Preset");
    m_prevPresetBtn.onClick = [this]() {
        m_processor.getPresetManager().previousPreset();
        updatePresetBox();
    };
    addAndMakeVisible(m_prevPresetBtn);

    m_nextPresetBtn.setTooltip("Next Preset");
    m_nextPresetBtn.onClick = [this]() {
        m_processor.getPresetManager().nextPreset();
        updatePresetBox();
    };
    addAndMakeVisible(m_nextPresetBtn);

    updatePresetBox();
    m_presetBox.setTooltip("Preset Library: Choose from curated vocal starting points or your saved user presets.");
    m_presetBox.onChange = [this]() {
        int idx = m_presetBox.getSelectedId() - 1;
        if (idx >= 0) {
            m_processor.getPresetManager().loadPreset(idx);
        }
    };
    addAndMakeVisible(m_presetBox);

    m_savePresetBtn.setTooltip("Save Preset: Stores current tuning and effect settings as a new user preset.");
    m_savePresetBtn.onClick = [this]() {
        int nextNum = m_processor.getPresetManager().getNumPresets() + 1;
        m_processor.getPresetManager().saveUserPreset("User Preset " + juce::String(nextNum));
        updatePresetBox();
    };
    addAndMakeVisible(m_savePresetBtn);

    m_abCompareBtn.setTooltip("A / B Compare: Instantly compare current settings against an alternate audition state.");
    m_abCompareBtn.onClick = [this]() {
        m_processor.getPresetManager().toggleAB();
        bool isB = m_processor.getPresetManager().isStateB();
        m_abCompareBtn.setButtonText(isB ? "STATE B" : "STATE A");
    };
    addAndMakeVisible(m_abCompareBtn);

    // --- Key & Scale Pickers ---
    const char* roots[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i) {
        m_keyRootBox.addItem(roots[i], i + 1);
    }
    m_keyRootBox.setTooltip("Musical Key: Sets the root note of the correction scale (C through B).");
    m_keyRootAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::keyRoot, m_keyRootBox);
    addAndMakeVisible(m_keyRootBox);

    const char* scales[] = { "Chromatic", "Major", "Natural Minor", "Harmonic Minor",
                             "Melodic Minor", "Pentatonic Major", "Pentatonic Minor", "Custom" };
    for (int i = 0; i < 8; ++i) {
        m_scaleTypeBox.addItem(scales[i], i + 1);
    }
    m_scaleTypeBox.setTooltip("Scale Type: Selects the musical scale mode. Choose Custom to edit notes directly on the keyboard.");
    m_scaleTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::scaleType, m_scaleTypeBox);
    addAndMakeVisible(m_scaleTypeBox);

    // --- Lookahead / Latency Mode ---
    m_lookaheadToggle.setButtonText("LOOKAHEAD");
    m_lookaheadToggle.setTooltip("Lookahead Mode: Engages a 20 ms lookahead buffer for refined pitch transitions during mixing.\nToggle off for ultra-low latency live tracking.");
    m_lookaheadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::lookaheadMode, m_lookaheadToggle);
    addAndMakeVisible(m_lookaheadToggle);

    m_latencyLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
    m_latencyLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_latencyLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(m_latencyLabel);

    // --- Master EFX Toggle ---
    m_masterEfxToggle.setButtonText("MASTER EFX");
    m_masterEfxToggle.setTooltip("Master EFX: Master bypass for the entire post-pitch EFX rack.");
    m_masterEfxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::masterEfxActive, m_masterEfxToggle);
    addAndMakeVisible(m_masterEfxToggle);

    // --- Left Control Bank (Tuning Dynamics) ---
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::retuneSpeed)) {
        m_retuneSpeedKnob = std::make_unique<RotaryKnob>(*param, "Retune Speed", "ms");
        m_retuneSpeedKnob->setHero(true);
        m_retuneSpeedKnob->setTooltipText("Retune Speed (0 - 400 ms): Governs how fast FreePitch pulls vocals to the target note.\n- 0 ms: Instant Cher / T-Pain hard snap\n- 15-35 ms: Tight modern pop & trap tuning\n- 60-150 ms: Transparent natural vocal correction.");
        m_retuneSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::retuneSpeed, m_retuneSpeedKnob->getSlider());
        addAndMakeVisible(m_retuneSpeedKnob.get());
    }
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::humanize)) {
        m_humanizeKnob = std::make_unique<RotaryKnob>(*param, "Humanize", "%");
        m_humanizeKnob->setTooltipText("Humanize (0 - 100%): Relaxes pitch correction on sustained steady notes,\nallowing natural singer vibrato and micro-inflections through while correcting transitions.");
        m_humanizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::humanize, m_humanizeKnob->getSlider());
        addAndMakeVisible(m_humanizeKnob.get());
    }
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::flexTune)) {
        m_flexTuneKnob = std::make_unique<RotaryKnob>(*param, "Flex-Tune", "%");
        m_flexTuneKnob->setTooltipText("Flex-Tune (0 - 100%): Elastic dead-band tolerance around target scale notes.\nIntentional vocal scoops and expressive bends near note center remain untouched;\nout-of-tune notes are pulled cleanly into center.");
        m_flexTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::flexTune, m_flexTuneKnob->getSlider());
        addAndMakeVisible(m_flexTuneKnob.get());
    }

    // --- Center Focal Point (Pitch Radar HUD) ---
    addAndMakeVisible(m_circularMeter);

    for (int i = 0; i < static_cast<int>(VocalMode::NumModes); ++i) {
        m_vocalModeBox.addItem(g_vocalModeBounds[i].name, i + 1);
    }
    m_vocalModeBox.setTooltip("Vocal Range Mode: Constrains pitch detection search bounds to eliminate octave-jumping errors.\nSelect the mode matching the singer's vocal register.");
    m_vocalModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_processor.getAPVTS(), FreePitchID::vocalMode, m_vocalModeBox);
    addAndMakeVisible(m_vocalModeBox);

    m_choosyTrackingToggle.setButtonText("CHOOSY TRACKING");
    m_choosyTrackingToggle.setTooltip("Choosy Tracking: Tightens clarity and noise thresholds so breaths, plosives, and sibilants\npass through transparently without unwanted pitch warble.");
    m_choosyTrackingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::choosyTracking, m_choosyTrackingToggle);
    addAndMakeVisible(m_choosyTrackingToggle);

    // --- Right Control Bank (Tone & Modular EFX) ---
    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::pitchShift)) {
        m_pitchShiftKnob = std::make_unique<RotaryKnob>(*param, "Pitch Shift", "st");
        m_pitchShiftKnob->setAccentAmber(true);
        m_pitchShiftKnob->setTooltipText("Pitch Shift (-12 to +12 st): Transposes corrected pitch in continuous semitones.");
        m_pitchShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::pitchShift, m_pitchShiftKnob->getSlider());
        addAndMakeVisible(m_pitchShiftKnob.get());
    }

    m_formantsTitleLabel.setText("FORMANTS", juce::dontSendNotification);
    m_formantsTitleLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    m_formantsTitleLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextMuted);
    m_formantsTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_formantsTitleLabel);

    m_formantsToggle.setButtonText("PRESERVE");
    m_formantsToggle.setTooltip("Formant Preserve: Locks vocal tract resonant frequencies in place during pitch transposition,\npreventing chipmunk / monster distortion.");
    m_formantsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_processor.getAPVTS(), FreePitchID::formantsOn, m_formantsToggle);
    addAndMakeVisible(m_formantsToggle);

    m_formantsSubLabel.setText("VOCAL TRACT", juce::dontSendNotification);
    m_formantsSubLabel.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::bold));
    m_formantsSubLabel.setColour(juce::Label::textColourId, FreePitchLookAndFeel::kTextDim);
    m_formantsSubLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_formantsSubLabel);

    if (auto* param = m_processor.getAPVTS().getParameter(FreePitchID::formantShift)) {
        m_formantShiftKnob = std::make_unique<RotaryKnob>(*param, "Formant Shift", "st");
        m_formantShiftKnob->setAccentAmber(true);
        m_formantShiftKnob->setTooltipText("Formant Shift (-12 to +12 st): Morphs vocal tract resonances independently.\nLower = deeper, darker chest resonance; Higher = brighter, smaller head voice.");
        m_formantShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_processor.getAPVTS(), FreePitchID::formantShift, m_formantShiftKnob->getSlider());
        addAndMakeVisible(m_formantShiftKnob.get());
    }

    // Modular Post-Pitch EFX Rack Bay
    addAndMakeVisible(m_efxDrawer);

    // --- Bottom Piano Keyboard ---
    addAndMakeVisible(m_pianoKeyboard);
    m_pianoKeyboard.onCustomMaskChanged = [this](uint32_t newMask) {
        m_processor.setCustomScaleMask(newMask);
    };

    setSize(1080, 680);
    setResizable(true, true);
    setResizeLimits(920, 580, 1600, 1050);

    startTimerHz(60); // 60 FPS Visual Telemetry
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

    // 1. Update Central Pitch Radar
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

    // 4. Update Host PDC Latency Readout
    int latencySamples = m_processor.getLatencySamples();
    float latencyMs = (static_cast<float>(latencySamples) / static_cast<float>(m_processor.getSampleRate())) * 1000.0f;
    juce::String latText = "LATENCY: " + juce::String(latencyMs, 1) + " ms (" + juce::String(latencySamples) + " smp)";
    m_latencyLabel.setText(latText, juce::dontSendNotification);

    // 5. Smooth Confidence / Gate Telemetry
    m_smoothConfidence += 0.25f * (telemetry.voicedConfidence - m_smoothConfidence);
    m_smoothInputRms += 0.25f * (telemetry.inputRmsDb - m_smoothInputRms);

    repaint();
}

void FreePitchAudioProcessorEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // 1. Deep Obsidian Studio Chassis
    g.fillAll(FreePitchLookAndFeel::kBackgroundDark);

    // 2. Top Bar Panel (50px)
    auto topBarArea = bounds.removeFromTop(50.0f);
    g.setColour(FreePitchLookAndFeel::kPanelDark);
    g.fillRect(topBarArea);

    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawHorizontalLine(static_cast<int>(topBarArea.getBottom()), 0.0f, bounds.getWidth());

    // Cyan branding dot beside title
    g.setColour(FreePitchLookAndFeel::kAccentCyan);
    g.fillEllipse(14.0f, 16.0f, 6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kAccentCyanGlow);
    g.fillEllipse(12.0f, 14.0f, 10.0f, 10.0f);

    // 3. Responsive Proportional Grid
    float bottomPianoHeight = std::clamp(bounds.getHeight() * 0.135f, 76.0f, 120.0f);
    auto contentArea = bounds.removeFromTop(bounds.getHeight() - bottomPianoHeight);

    float totalW = contentArea.getWidth();
    float leftWidth = std::clamp(totalW * 0.22f, 210.0f, 290.0f);
    float rightWidth = std::clamp(totalW * 0.46f, 450.0f, 740.0f);

    // Left Bank Panel (Tuning Dynamics)
    auto leftPanel = contentArea.removeFromLeft(leftWidth).reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelCard);
    g.fillRoundedRectangle(leftPanel, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(leftPanel, 6.0f, 1.0f);

    // Right Bank Panel (Voice Tone & Modular EFX)
    auto rightPanel = contentArea.removeFromRight(rightWidth).reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelCard);
    g.fillRoundedRectangle(rightPanel, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(rightPanel, 6.0f, 1.0f);

    // Center Focal Zone Panel (Pitch Radar & Scale Quantizer)
    auto centerPanel = contentArea.reduced(6.0f, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelCard);
    g.fillRoundedRectangle(centerPanel, 6.0f);
    g.setColour(FreePitchLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle(centerPanel, 6.0f, 1.0f);

    // Section Header Badges
    auto drawHeader = [&](const juce::String& text, juce::Rectangle<float> panel, const juce::Colour& dotCol) {
        g.setColour(dotCol);
        g.fillEllipse(panel.getX() + 12.0f, panel.getY() + 11.0f, 5.0f, 5.0f);
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.setColour(FreePitchLookAndFeel::kTextLight);
        g.drawText(text, juce::Rectangle<float>(panel.getX() + 22.0f, panel.getY() + 6.0f, panel.getWidth() - 30.0f, 16.0f),
                   juce::Justification::centredLeft);
    };

    drawHeader("TUNING DYNAMICS", leftPanel, FreePitchLookAndFeel::kAccentCyan);
    drawHeader("PITCH RADAR & SCALE QUANTIZER", centerPanel, FreePitchLookAndFeel::kAccentAmber);
    drawHeader("VOICE TONE & MODULAR EFX", rightPanel, FreePitchLookAndFeel::kAccentCyan);

    // Live Confidence / Voiced Gate Bar in Center Panel
    if (!m_confidenceMeterBounds.isEmpty()) {
        float cx = m_confidenceMeterBounds.getX();
        float cy = m_confidenceMeterBounds.getY();
        float cw = m_confidenceMeterBounds.getWidth();
        float ch = m_confidenceMeterBounds.getHeight();

        g.setColour(FreePitchLookAndFeel::kPanelDark);
        g.fillRoundedRectangle(cx, cy, cw, ch, 3.0f);
        g.setColour(FreePitchLookAndFeel::kPanelBorder);
        g.drawRoundedRectangle(cx, cy, cw, ch, 3.0f, 0.8f);

        float confFrac = std::clamp(m_smoothConfidence, 0.0f, 1.0f);
        if (confFrac > 0.02f) {
            auto fillRect = juce::Rectangle<float>(cx, cy, cw * confFrac, ch);
            juce::Colour fillCol = (confFrac >= 0.58f) ? FreePitchLookAndFeel::kIndicatorInTune
                                                      : FreePitchLookAndFeel::kIndicatorSharp;
            g.setColour(fillCol.withAlpha(0.85f));
            g.fillRoundedRectangle(fillRect, 3.0f);
        }

        // Percentage readout
        int pct = static_cast<int>(std::round(confFrac * 100.0f));
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
        g.setColour(FreePitchLookAndFeel::kTextWhite);
        juce::String txt = (confFrac >= 0.58f ? "GATE: VOICED " : "GATE: UNVOICED ") + juce::String(pct) + "%";
        g.drawText(txt, static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(cw), static_cast<int>(ch),
                   juce::Justification::centred);
    }
}

void FreePitchAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    int totalW = bounds.getWidth();
    int leftW = std::clamp(static_cast<int>(totalW * 0.22f), 210, 290);
    int rightW = std::clamp(static_cast<int>(totalW * 0.46f), 450, 740);

    // 1. Top Header Bar (50px)
    auto topBar = bounds.removeFromTop(50).reduced(8, 6);

    // Left Zone (width = leftW - 8): Logo, Subtitle, Version
    auto leftTopArea = topBar.removeFromLeft(leftW - 8);
    int logoW = std::clamp(leftTopArea.getWidth() - 44, 110, 150);
    auto logoArea = leftTopArea.removeFromLeft(logoW);
    m_titleLabel.setBounds(logoArea.removeFromTop(20).withTrimmedLeft(18));
    m_subtitleLabel.setBounds(logoArea.withTrimmedLeft(18));
    m_versionLabel.setBounds(leftTopArea.reduced(2, 4));

    // Right Zone (width = rightW - 8): Key, Scale, Lookahead, Latency, Master EFX
    auto rightTopArea = topBar.removeFromRight(rightW - 8);
    m_masterEfxToggle.setBounds(rightTopArea.removeFromRight(std::min(95, rightTopArea.getWidth() / 5)).reduced(2, 2));
    m_latencyLabel.setBounds(rightTopArea.removeFromRight(std::min(130, rightTopArea.getWidth() / 4)).reduced(2, 2));
    m_lookaheadToggle.setBounds(rightTopArea.removeFromRight(std::min(92, rightTopArea.getWidth() / 3)).reduced(2, 2));
    m_scaleTypeBox.setBounds(rightTopArea.removeFromRight(std::min(115, rightTopArea.getWidth() / 2)).reduced(2, 3));
    m_keyRootBox.setBounds(rightTopArea.reduced(2, 3));

    // Center Zone (Preset Controls): Center-aligned in whatever remains
    auto centerTopArea = topBar;
    int presetStripW = std::min(centerTopArea.getWidth() - 8, 290);
    int presetStartX = centerTopArea.getX() + (centerTopArea.getWidth() - presetStripW) / 2;
    auto presetStrip = juce::Rectangle<int>(presetStartX, centerTopArea.getY(), presetStripW, centerTopArea.getHeight());

    m_prevPresetBtn.setBounds(presetStrip.removeFromLeft(22).reduced(0, 3));
    m_nextPresetBtn.setBounds(presetStrip.removeFromRight(22).reduced(0, 3));
    m_abCompareBtn.setBounds(presetStrip.removeFromRight(46).reduced(2, 3));
    m_savePresetBtn.setBounds(presetStrip.removeFromRight(46).reduced(2, 3));
    m_presetBox.setBounds(presetStrip.reduced(2, 3));

    // 2. Bottom Piano Keyboard (Responsive Height: ~13.5% of window height)
    int pianoH = std::clamp(static_cast<int>(bounds.getHeight() * 0.135f), 76, 120);
    m_pianoKeyboard.setBounds(bounds.removeFromBottom(pianoH));

    // 3. Main 3-Column Content Zone
    auto contentArea = bounds;
    auto leftArea = contentArea.removeFromLeft(leftW).reduced(10, 10);
    leftArea.removeFromTop(26); // Header title margin

    // Left Knobs (Hero Retune Speed on top, Humanize, Flex-Tune)
    int heroHeight = static_cast<int>(leftArea.getHeight() * 0.42f);
    if (m_retuneSpeedKnob) m_retuneSpeedKnob->setBounds(leftArea.removeFromTop(heroHeight));

    int medHeight = leftArea.getHeight() / 2;
    if (m_humanizeKnob) m_humanizeKnob->setBounds(leftArea.removeFromTop(medHeight));
    if (m_flexTuneKnob) m_flexTuneKnob->setBounds(leftArea);

    // Right Area (Tone Knobs + Modular EFX Rack)
    auto rightArea = contentArea.removeFromRight(rightW).reduced(10, 10);
    rightArea.removeFromTop(26); // Header title margin

    // Top Tone Row (Pitch Shift, Preserve Toggle, Formant Shift)
    int toneRowH = std::clamp(static_cast<int>(rightArea.getHeight() * 0.24f), 86, 120);
    auto toneRow = rightArea.removeFromTop(toneRowH);
    int toneKnobWidth = toneRow.getWidth() / 3;

    if (m_pitchShiftKnob) m_pitchShiftKnob->setBounds(toneRow.removeFromLeft(toneKnobWidth));

    auto formantsCenter = toneRow.removeFromLeft(toneKnobWidth);
    int fLabelH = 15;
    int fToggleH = 26;
    int fSubH = 14;
    int fTotal = fLabelH + fToggleH + fSubH + 4;
    int fStartY = std::max(0, (formantsCenter.getHeight() - fTotal) / 2);
    m_formantsTitleLabel.setBounds(formantsCenter.getX(), formantsCenter.getY() + fStartY, formantsCenter.getWidth(), fLabelH);

    int toggleW = std::min(96, formantsCenter.getWidth() - 8);
    int toggleX = formantsCenter.getX() + (formantsCenter.getWidth() - toggleW) / 2;
    m_formantsToggle.setBounds(toggleX, formantsCenter.getY() + fStartY + fLabelH + 2, toggleW, fToggleH);

    m_formantsSubLabel.setBounds(formantsCenter.getX(), formantsCenter.getY() + fStartY + fLabelH + fToggleH + 4, formantsCenter.getWidth(), fSubH);

    if (m_formantShiftKnob) m_formantShiftKnob->setBounds(toneRow);

    // Modular EFX Rack Bay takes the remaining right area
    rightArea.removeFromTop(6);
    m_efxDrawer.setBounds(rightArea);

    // Center Area (Pitch Radar HUD + Vocal Mode + Tracking)
    auto centerArea = contentArea.reduced(10, 10);
    centerArea.removeFromTop(26); // Header title margin

    // Bottom controls in center area
    auto centerBottom = centerArea.removeFromBottom(70);
    auto vocalRow = centerBottom.removeFromTop(28);
    m_vocalModeBox.setBounds(vocalRow.removeFromLeft(vocalRow.getWidth() / 2).reduced(4, 1));
    m_choosyTrackingToggle.setBounds(vocalRow.reduced(4, 1));

    centerBottom.removeFromTop(4);
    m_confidenceMeterBounds = centerBottom.removeFromTop(16).toFloat().reduced(6.0f, 0.0f);

    // Pitch Radar HUD fills remaining center area
    m_circularMeter.setBounds(centerArea.reduced(4));
}
