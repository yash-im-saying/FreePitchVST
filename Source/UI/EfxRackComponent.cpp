#include "EfxRackComponent.h"
#include <cmath>

// ============================================================================
// EfxCardComponent Implementation
// ============================================================================

EfxCardComponent::EfxCardComponent(const juce::String& title, const char* enabledParamID,
                                   juce::AudioProcessorValueTreeState& apvts)
    : m_title(title) {

    m_powerToggle.setButtonText("");
    m_powerToggle.setTooltip("Click to Bypass / Activate " + title);
    m_powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, enabledParamID, m_powerToggle);

    m_powerToggle.onStateChange = [this]() {
        repaint();
    };

    addAndMakeVisible(m_powerToggle);
    m_powerToggle.setAlpha(0.0f); // invisible hit-target over LED
}

void EfxCardComponent::addControl(std::unique_ptr<RotaryKnob> knob) {
    if (!knob) return;
    knob->setMini(true);
    addAndMakeVisible(knob.get());
    m_knobs.push_back(std::move(knob));
}

void EfxCardComponent::addComboBox(std::unique_ptr<juce::ComboBox> box) {
    if (!box) return;
    addAndMakeVisible(box.get());
    m_boxes.push_back(std::move(box));
}

void EfxCardComponent::mouseDown(const juce::MouseEvent& e) {
    // Clicking header bar (top 26px) toggles bypass
    if (e.y < 28) {
        m_powerToggle.setToggleState(!m_powerToggle.getToggleState(), juce::sendNotification);
    }
}

void EfxCardComponent::resized() {
    renderCachedCardBg();

    auto bounds = getLocalBounds();
    if (bounds.isEmpty()) return;

    // Hit-target for LED dot in top-left
    m_powerToggle.setBounds(8, 6, 20, 20);

    auto contentArea = bounds;
    contentArea.removeFromTop(28); // Header height
    contentArea.reduce(6, 6);

    int numBoxes = static_cast<int>(m_boxes.size());
    int numKnobs = static_cast<int>(m_knobs.size());

    // Layout combo box if present
    if (numBoxes > 0) {
        auto boxRow = contentArea.removeFromTop(18);
        for (int i = 0; i < numBoxes; ++i) {
            m_boxes[i]->setBounds(boxRow.reduced(4, 0));
        }
        contentArea.removeFromTop(4);
    }

    if (numKnobs <= 0) return;

    if (numKnobs == 3) {
        // Single row of 3 knobs
        int kw = contentArea.getWidth() / 3;
        for (int i = 0; i < 3; ++i) {
            m_knobs[i]->setBounds(contentArea.getX() + i * kw, contentArea.getY(), kw, contentArea.getHeight());
        }
    } else if (numKnobs == 4) {
        // 2x2 grid
        int rowH = contentArea.getHeight() / 2;
        int kw = contentArea.getWidth() / 2;
        m_knobs[0]->setBounds(contentArea.getX(), contentArea.getY(), kw, rowH);
        m_knobs[1]->setBounds(contentArea.getX() + kw, contentArea.getY(), kw, rowH);
        m_knobs[2]->setBounds(contentArea.getX(), contentArea.getY() + rowH, kw, rowH);
        m_knobs[3]->setBounds(contentArea.getX() + kw, contentArea.getY() + rowH, kw, rowH);
    } else if (numKnobs >= 5) {
        // 3 knobs on top row, remaining on bottom row
        int rowH = contentArea.getHeight() / 2;
        int topKw = contentArea.getWidth() / 3;
        m_knobs[0]->setBounds(contentArea.getX(), contentArea.getY(), topKw, rowH);
        m_knobs[1]->setBounds(contentArea.getX() + topKw, contentArea.getY(), topKw, rowH);
        m_knobs[2]->setBounds(contentArea.getX() + topKw * 2, contentArea.getY(), topKw, rowH);

        int botCount = numKnobs - 3;
        int botKw = contentArea.getWidth() / std::max(1, botCount);
        int botOffset = (contentArea.getWidth() - botKw * botCount) / 2;
        for (int i = 0; i < botCount; ++i) {
            m_knobs[3 + i]->setBounds(contentArea.getX() + botOffset + i * botKw,
                                      contentArea.getY() + rowH, botKw, rowH);
        }
    }
}

void EfxCardComponent::renderCachedCardBg() {
    int w = getWidth();
    int h = getHeight();
    if (w <= 0 || h <= 0) return;

    m_cachedCardBg = juce::Image(juce::Image::ARGB, w, h, true);
    juce::Graphics g(m_cachedCardBg);

    using namespace FreePitchColors;
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // Elevated card surface with dual opposing shadows (#1E2127, gradient #22262D -> #191C21)
    FreePitchLookAndFeel::drawNeumorphicRaised(g, bounds, 8.0f, 3.5f);

    // Subtle screw / rivet accent in top-right corner
    float screwX = bounds.getRight() - 14.0f;
    float screwY = bounds.getY() + 14.0f;
    float screwR = 3.5f;

    g.setColour(cavityRecessed);
    g.fillEllipse(screwX - screwR, screwY - screwR, screwR * 2.0f, screwR * 2.0f);
    g.setColour(rimHighlight.withAlpha(0.6f));
    g.drawEllipse(screwX - screwR, screwY - screwR, screwR * 2.0f, screwR * 2.0f, 0.8f);

    // Subtle screw slot line
    g.setColour(textDim);
    g.drawLine(screwX - 2.0f, screwY - 2.0f, screwX + 2.0f, screwY + 2.0f, 0.9f);
}

void EfxCardComponent::paint(juce::Graphics& g) {
    using namespace FreePitchColors;

    // 1. Draw pre-rendered heavy static card shell (Zero heap allocation in paint)
    if (m_cachedCardBg.isValid()) {
        g.drawImageAt(m_cachedCardBg, 0, 0);
    }

    bool isOn = m_powerToggle.getToggleState();

    // 2. Clickable Signal-Red LED bypass dot (#FF2E3B with bloom)
    float ledX = 18.0f;
    float ledY = 18.0f;
    FreePitchLookAndFeel::drawTactileLed(g, ledX, ledY, 3.5f, signalRed, isOn);

    // 3. Uppercase Module Header
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(isOn ? textPrimary : textDim);
    g.drawText(m_title.toUpperCase(), 28, 9, getWidth() - 48, 18, juce::Justification::centredLeft, true);
}

// ============================================================================
// EfxRackComponent Implementation
// ============================================================================

EfxRackComponent::EfxRackComponent(juce::AudioProcessorValueTreeState& apvts, EFXRack& efxRack)
    : m_apvts(apvts), m_efxRack(efxRack) {

    setupEqCard();
    setupCompCard();
    setupChorusCard();
    setupFlangerCard();
    setupDistCard();
    setupDelayCard();
}

void EfxRackComponent::setupEqCard() {
    m_eqCard = std::make_unique<EfxCardComponent>("EQ", FreePitchID::eqEnabled, m_apvts);

    auto addEqKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_eqCard->addControl(std::move(knob));
        }
    };

    addEqKnob(FreePitchID::eqHpfFreq,     "HPF", "Hz");
    addEqKnob(FreePitchID::eqLowMidGain,  "LOW", "dB");
    addEqKnob(FreePitchID::eqHighMidGain, "MID", "dB");
    addEqKnob(FreePitchID::eqAirGain,     "AIR", "dB");
    addEqKnob(FreePitchID::eqOutGain,     "OUT", "dB");

    addAndMakeVisible(m_eqCard.get());
}

void EfxRackComponent::setupCompCard() {
    m_compCard = std::make_unique<EfxCardComponent>("COMP", FreePitchID::compEnabled, m_apvts);

    auto addCompKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_compCard->addControl(std::move(knob));
        }
    };

    addCompKnob(FreePitchID::compThreshold, "THRESH",  "dB");
    addCompKnob(FreePitchID::compRatio,     "RATIO",   ":1");
    addCompKnob(FreePitchID::compMakeup,    "GAIN",    "dB");
    addCompKnob(FreePitchID::compAttack,    "ATTACK",  "ms");
    addCompKnob(FreePitchID::compRelease,   "RELEASE", "ms");

    addAndMakeVisible(m_compCard.get());
}

void EfxRackComponent::setupChorusCard() {
    m_chorusCard = std::make_unique<EfxCardComponent>("CHORUS", FreePitchID::chorusEnabled, m_apvts);

    auto modeBox = std::make_unique<juce::ComboBox>();
    modeBox->addItem("Chorus", 1);
    modeBox->addItem("Haas", 2);
    m_comboAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::chorusMode, *modeBox));
    m_chorusCard->addComboBox(std::move(modeBox));

    auto addChorusKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_chorusCard->addControl(std::move(knob));
        }
    };

    addChorusKnob(FreePitchID::chorusRate,  "RATE",  "Hz");
    addChorusKnob(FreePitchID::chorusDepth, "DEPTH", "%");
    addChorusKnob(FreePitchID::chorusMix,   "MIX",   "%");

    addAndMakeVisible(m_chorusCard.get());
}

void EfxRackComponent::setupFlangerCard() {
    m_flangerCard = std::make_unique<EfxCardComponent>("FLANGER", FreePitchID::flangerEnabled, m_apvts);

    auto addFlgKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_flangerCard->addControl(std::move(knob));
        }
    };

    addFlgKnob(FreePitchID::flangerRate,     "RATE",   "Hz");
    addFlgKnob(FreePitchID::flangerDepth,    "DEPTH",  "%");
    addFlgKnob(FreePitchID::flangerFeedback, "FEEDBK", "%");
    addFlgKnob(FreePitchID::flangerMix,      "MIX",    "%");

    addAndMakeVisible(m_flangerCard.get());
}

void EfxRackComponent::setupDistCard() {
    m_distCard = std::make_unique<EfxCardComponent>("DISTORTION", FreePitchID::distEnabled, m_apvts);

    auto modeBox = std::make_unique<juce::ComboBox>();
    modeBox->addItem("Tube", 1);
    modeBox->addItem("Tape", 2);
    modeBox->addItem("Clip", 3);
    m_comboAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::distMode, *modeBox));
    m_distCard->addComboBox(std::move(modeBox));

    auto addDistKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_distCard->addControl(std::move(knob));
        }
    };

    addDistKnob(FreePitchID::distDrive, "DRIVE", "%");
    addDistKnob(FreePitchID::distTone,  "TONE",  "Hz");
    addDistKnob(FreePitchID::distMix,   "MIX",   "%");

    addAndMakeVisible(m_distCard.get());
}

void EfxRackComponent::setupDelayCard() {
    m_delayCard = std::make_unique<EfxCardComponent>("DELAY", FreePitchID::delayEnabled, m_apvts);

    auto modeBox = std::make_unique<juce::ComboBox>();
    modeBox->addItem("Stereo", 1);
    modeBox->addItem("Ping-Pong", 2);
    m_comboAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(m_apvts, FreePitchID::delayMode, *modeBox));
    m_delayCard->addComboBox(std::move(modeBox));

    auto addDelayKnob = [this](const char* id, const juce::String& name, const juce::String& unit) {
        if (auto* p = m_apvts.getParameter(id)) {
            auto knob = std::make_unique<RotaryKnob>(*p, name, unit);
            m_sliderAttachments.push_back(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, id, knob->getSlider()));
            m_delayCard->addControl(std::move(knob));
        }
    };

    addDelayKnob(FreePitchID::delayTime,     "TIME", "ms");
    addDelayKnob(FreePitchID::delayFeedback, "FDBK", "%");
    addDelayKnob(FreePitchID::delayTone,     "TONE", "Hz");
    addDelayKnob(FreePitchID::delayMix,      "MIX",  "%");

    addAndMakeVisible(m_delayCard.get());
}

void EfxRackComponent::updateTelemetry(const PitchTelemetry& telemetry) {
    m_telemetry = telemetry;
}

void EfxRackComponent::updateThemeColors() {
    repaint();
}

void EfxRackComponent::paint(juce::Graphics& /*g*/) {
    // Rack chassis container
}

void EfxRackComponent::resized() {
    auto bounds = getLocalBounds();
    if (bounds.isEmpty()) return;

    constexpr int numCards = 6;
    constexpr int gutter = 12;

    int totalGutters = gutter * (numCards - 1);
    int availW = bounds.getWidth() - totalGutters;
    int cardW = availW / numCards;

    int startX = bounds.getX() + (bounds.getWidth() - (cardW * numCards + totalGutters)) / 2;

    EfxCardComponent* cards[numCards] = {
        m_eqCard.get(),
        m_compCard.get(),
        m_chorusCard.get(),
        m_flangerCard.get(),
        m_distCard.get(),
        m_delayCard.get()
    };

    for (int i = 0; i < numCards; ++i) {
        if (cards[i]) {
            cards[i]->setBounds(startX + i * (cardW + gutter), bounds.getY(), cardW, bounds.getHeight());
        }
    }
}
