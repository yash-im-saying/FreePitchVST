#include "PluginProcessor.h"
#include "PluginEditor.h"

FreePitchAudioProcessor::FreePitchAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      m_apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
      m_presetManager(m_apvts) {

    cacheParameterPointers();
}

void FreePitchAudioProcessor::cacheParameterPointers() {
    auto getRaw = [this](const char* id) {
        return m_apvts.getRawParameterValue(id);
    };

    m_prmVocalMode     = getRaw(FreePitchID::vocalMode);
    m_prmKeyRoot       = getRaw(FreePitchID::keyRoot);
    m_prmScaleType     = getRaw(FreePitchID::scaleType);
    m_prmRetuneSpeed   = getRaw(FreePitchID::retuneSpeed);
    m_prmHumanize      = getRaw(FreePitchID::humanize);
    m_prmFlexTune      = getRaw(FreePitchID::flexTune);
    m_prmChoosyTracking = getRaw(FreePitchID::choosyTracking);
    m_prmPitchShift    = getRaw(FreePitchID::pitchShift);
    m_prmFormantsOn    = getRaw(FreePitchID::formantsOn);
    m_prmFormantShift  = getRaw(FreePitchID::formantShift);
    m_prmLookaheadMode = getRaw(FreePitchID::lookaheadMode);
    m_prmMasterEfxActive = getRaw(FreePitchID::masterEfxActive);

    m_prmEqEnabled     = getRaw(FreePitchID::eqEnabled);
    m_prmEqHpfFreq     = getRaw(FreePitchID::eqHpfFreq);
    m_prmEqLowMidGain  = getRaw(FreePitchID::eqLowMidGain);
    m_prmEqLowMidFreq  = getRaw(FreePitchID::eqLowMidFreq);
    m_prmEqHighMidGain = getRaw(FreePitchID::eqHighMidGain);
    m_prmEqHighMidFreq = getRaw(FreePitchID::eqHighMidFreq);
    m_prmEqAirGain     = getRaw(FreePitchID::eqAirGain);
    m_prmEqAirFreq     = getRaw(FreePitchID::eqAirFreq);
    m_prmEqOutGain     = getRaw(FreePitchID::eqOutGain);

    m_prmCompEnabled   = getRaw(FreePitchID::compEnabled);
    m_prmCompThreshold = getRaw(FreePitchID::compThreshold);
    m_prmCompRatio     = getRaw(FreePitchID::compRatio);
    m_prmCompAttack    = getRaw(FreePitchID::compAttack);
    m_prmCompRelease   = getRaw(FreePitchID::compRelease);
    m_prmCompMakeup    = getRaw(FreePitchID::compMakeup);

    m_prmFlangerEnabled  = getRaw(FreePitchID::flangerEnabled);
    m_prmFlangerRate     = getRaw(FreePitchID::flangerRate);
    m_prmFlangerDepth    = getRaw(FreePitchID::flangerDepth);
    m_prmFlangerFeedback = getRaw(FreePitchID::flangerFeedback);
    m_prmFlangerMix      = getRaw(FreePitchID::flangerMix);

    m_prmChorusEnabled   = getRaw(FreePitchID::chorusEnabled);
    m_prmChorusMode      = getRaw(FreePitchID::chorusMode);
    m_prmChorusRate      = getRaw(FreePitchID::chorusRate);
    m_prmChorusDepth     = getRaw(FreePitchID::chorusDepth);
    m_prmHaasDelay       = getRaw(FreePitchID::haasDelay);
    m_prmHaasWidth       = getRaw(FreePitchID::haasWidth);
    m_prmChorusMix       = getRaw(FreePitchID::chorusMix);

    m_prmDistEnabled     = getRaw(FreePitchID::distEnabled);
    m_prmDistMode        = getRaw(FreePitchID::distMode);
    m_prmDistDrive       = getRaw(FreePitchID::distDrive);
    m_prmDistTone        = getRaw(FreePitchID::distTone);
    m_prmDistMix         = getRaw(FreePitchID::distMix);

    m_prmDelayEnabled    = getRaw(FreePitchID::delayEnabled);
    m_prmDelayMode       = getRaw(FreePitchID::delayMode);
    m_prmDelaySyncMode   = getRaw(FreePitchID::delaySyncMode);
    m_prmDelaySubdiv     = getRaw(FreePitchID::delaySubdiv);
    m_prmDelayBpm        = getRaw(FreePitchID::delayBpm);
    m_prmDelayTime       = getRaw(FreePitchID::delayTime);
    m_prmDelayFeedback   = getRaw(FreePitchID::delayFeedback);
    m_prmDelayFreqShift  = getRaw(FreePitchID::delayFreqShift);
    m_prmDelayDiffusion  = getRaw(FreePitchID::delayDiffusion);
    m_prmDelayTone       = getRaw(FreePitchID::delayTone);
    m_prmDelayMix        = getRaw(FreePitchID::delayMix);
}

void FreePitchAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    m_pitchDetector.prepare(sampleRate, samplesPerBlock);
    m_voicedClassifier.prepare(sampleRate);
    m_scaleQuantizer.prepare(sampleRate);
    m_pitchShifter.prepare(sampleRate, samplesPerBlock);
    m_efxRack.prepare(sampleRate, samplesPerBlock);

    m_monoInputBuffer.assign(std::max(samplesPerBlock, 8192), 0.0f);
    m_shiftedMonoBuffer.assign(std::max(samplesPerBlock, 8192), 0.0f);

    setLatencySamples(m_pitchShifter.getLatencySamples());
}

void FreePitchAudioProcessor::releaseResources() {
    m_pitchDetector.reset();
    m_voicedClassifier.reset();
    m_scaleQuantizer.reset();
    m_pitchShifter.reset();
    m_efxRack.reset();
}

bool FreePitchAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void FreePitchAudioProcessor::setCustomScaleMask(uint32_t mask12Bit) {
    m_customScaleMask.store(mask12Bit);
    m_scaleQuantizer.setCustomMask(mask12Bit);
}

PitchTelemetry FreePitchAudioProcessor::getLatestTelemetry() {
    PitchTelemetry t;
    t.detectedFreqHz = m_atomicDetectedFreq.load();
    t.targetFreqHz = m_atomicTargetFreq.load();
    t.detectedMidi = m_atomicDetectedMidi.load();
    t.targetMidi = m_atomicTargetMidi.load();
    t.centsDeviation = m_atomicCentsDev.load();
    t.appliedCorrectionCents = m_atomicAppliedCorr.load();
    t.voicedConfidence = m_atomicVoicedConfidence.load();
    t.isVoiced = m_atomicIsVoiced.load();
    t.compressorGR_dB = m_atomicCompGR.load();
    t.haasPhaseWarning = m_atomicHaasPhaseWarn.load();
    t.inputRmsDb = m_atomicInputRmsDb.load();
    t.outputRmsDb = m_atomicOutputRmsDb.load();
    t.customScaleMask = m_customScaleMask.load();
    t.hostBpm = m_atomicHostBpm.load();
    t.effectiveDelayMs = m_atomicEffectiveDelayMs.load();
    return t;
}

void FreePitchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    if (numSamples <= 0) return;

    // Clear unused output channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, numSamples);
    }

    // 1. Snapshot Parameters Atomically
    auto getFloat = [](std::atomic<float>* ptr, float def) { return ptr ? ptr->load() : def; };

    int vocalModeIdx = static_cast<int>(getFloat(m_prmVocalMode, 3.0f));
    auto vocalMode = static_cast<VocalMode>(vocalModeIdx);

    int keyRoot = static_cast<int>(getFloat(m_prmKeyRoot, 0.0f));
    int scaleIdx = static_cast<int>(getFloat(m_prmScaleType, 0.0f));
    auto scaleType = static_cast<ScaleType>(scaleIdx);

    float retuneSpeed = getFloat(m_prmRetuneSpeed, 20.0f);
    float humanize = getFloat(m_prmHumanize, 30.0f);
    float flexTune = getFloat(m_prmFlexTune, 20.0f);
    bool choosyTracking = (getFloat(m_prmChoosyTracking, 0.0f) > 0.5f);
    float pitchShiftSt = getFloat(m_prmPitchShift, 0.0f);
    bool formantsOn = (getFloat(m_prmFormantsOn, 1.0f) > 0.5f);
    float formantShiftSt = getFloat(m_prmFormantShift, 0.0f);
    bool lookaheadMode = (getFloat(m_prmLookaheadMode, 0.0f) > 0.5f);
    bool masterEfxActive = (getFloat(m_prmMasterEfxActive, 1.0f) > 0.5f);

    // Update subsystem configurations
    m_pitchDetector.setVocalMode(vocalMode);
    m_pitchDetector.setChoosyTracking(choosyTracking);
    m_voicedClassifier.setChoosyTracking(choosyTracking);
    m_scaleQuantizer.setKeyRoot(keyRoot);
    m_scaleQuantizer.setScaleType(scaleType);
    m_scaleQuantizer.setCustomMask(m_customScaleMask.load());
    m_scaleQuantizer.setRetuneSpeedMs(retuneSpeed);
    m_scaleQuantizer.setHumanize(humanize);
    m_scaleQuantizer.setFlexTune(flexTune);
    m_scaleQuantizer.setPitchShift(pitchShiftSt);

    m_pitchShifter.setLookaheadMode(lookaheadMode);
    m_pitchShifter.setVocalMode(vocalMode);
    
    int newLatency = m_pitchShifter.getLatencySamples();
    if (getLatencySamples() != newLatency) {
        setLatencySamples(newLatency);
    }

    // 2. Mixdown inputs to Mono for Pitch Tracking
    if (static_cast<int>(m_monoInputBuffer.size()) < numSamples) {
        m_monoInputBuffer.resize(numSamples, 0.0f);
        m_shiftedMonoBuffer.resize(numSamples, 0.0f);
    }

    const float* inL = buffer.getReadPointer(0);
    const float* inR = (totalNumInputChannels > 1) ? buffer.getReadPointer(1) : nullptr;

    float sumSqIn = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        float mono = inR ? (0.5f * (inL[i] + inR[i])) : inL[i];
        m_monoInputBuffer[i] = mono;
        sumSqIn += mono * mono;
    }
    float inRmsDb = juce::Decibels::gainToDecibels(std::sqrt(sumSqIn / static_cast<float>(numSamples)));

    // 3. Push Samples to Pitch Detector & Run McLeod Pitch Method (MPM)
    m_pitchDetector.pushSamples(m_monoInputBuffer.data(), numSamples);

    float rawDetectedHz = 0.0f;
    float mpmConfidence = 0.0f;
    float windowRms = 0.0f;
    m_pitchDetector.processPitch(rawDetectedHz, mpmConfidence, windowRms);

    // 4. Classify Voiced vs. Consonant / Sibilant / Silence
    bool isVoiced = m_voicedClassifier.classify(m_monoInputBuffer.data(), numSamples, mpmConfidence, inRmsDb);

    // 5. Musical Scale Quantization & Expressive Correction
    float dtSeconds = static_cast<float>(numSamples) / static_cast<float>(getSampleRate());
    float centsDev = 0.0f;
    float appliedCorrCents = 0.0f;
    int detMidi = -1;
    int tgtMidi = -1;

    float targetHz = m_scaleQuantizer.processTuning(rawDetectedHz, isVoiced, dtSeconds,
                                                    centsDev, appliedCorrCents, detMidi, tgtMidi);

    // 6. Real-Time TD-PSOLA Pitch Shifting & Formant Preservation
    m_pitchShifter.process(m_monoInputBuffer.data(), m_shiftedMonoBuffer.data(), numSamples,
                           rawDetectedHz, targetHz, isVoiced, formantsOn, formantShiftSt);

    // Copy shifted signal to channels
    float* outL = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i) {
        outL[i] = m_shiftedMonoBuffer[i];
    }
    float* outR = (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : nullptr;
    if (outR) {
        for (int i = 0; i < numSamples; ++i) {
            outR[i] = m_shiftedMonoBuffer[i];
        }
    }

    // 7. Update and Process Post-Pitch Modular EFX Rack
    m_efxRack.setMasterBypass(!masterEfxActive);

    m_efxRack.setEqParams(getFloat(m_prmEqEnabled, 1.0f) > 0.5f,
                          getFloat(m_prmEqHpfFreq, 80.0f),
                          getFloat(m_prmEqLowMidFreq, 500.0f),
                          getFloat(m_prmEqLowMidGain, 0.0f),
                          getFloat(m_prmEqHighMidFreq, 3000.0f),
                          getFloat(m_prmEqHighMidGain, 0.0f),
                          getFloat(m_prmEqAirFreq, 12000.0f),
                          getFloat(m_prmEqAirGain, 0.0f),
                          getFloat(m_prmEqOutGain, 0.0f));

    m_efxRack.setCompParams(getFloat(m_prmCompEnabled, 1.0f) > 0.5f,
                           getFloat(m_prmCompThreshold, -18.0f),
                           getFloat(m_prmCompRatio, 3.5f),
                           getFloat(m_prmCompAttack, 15.0f),
                           getFloat(m_prmCompRelease, 120.0f),
                           getFloat(m_prmCompMakeup, 3.0f));

    m_efxRack.setFlangerParams(getFloat(m_prmFlangerEnabled, 0.0f) > 0.5f,
                              getFloat(m_prmFlangerRate, 0.5f),
                              getFloat(m_prmFlangerDepth, 50.0f),
                              getFloat(m_prmFlangerFeedback, 30.0f),
                              getFloat(m_prmFlangerMix, 40.0f));

    m_efxRack.setChorusParams(getFloat(m_prmChorusEnabled, 0.0f) > 0.5f,
                             static_cast<int>(getFloat(m_prmChorusMode, 0.0f)),
                             getFloat(m_prmChorusRate, 1.2f),
                             getFloat(m_prmChorusDepth, 40.0f),
                             getFloat(m_prmHaasDelay, 12.0f),
                             getFloat(m_prmHaasWidth, 80.0f),
                             getFloat(m_prmChorusMix, 50.0f));

    m_efxRack.setDistParams(getFloat(m_prmDistEnabled, 0.0f) > 0.5f,
                           static_cast<int>(getFloat(m_prmDistMode, 0.0f)),
                           getFloat(m_prmDistDrive, 20.0f),
                           getFloat(m_prmDistTone, 8000.0f),
                           getFloat(m_prmDistMix, 30.0f));

    // Live DAW Host Tempo Detection & Delay Tempo Sync
    float currentBpm = 120.0f;
    if (auto* playHead = getPlayHead()) {
        if (auto posOpt = playHead->getPosition()) {
            if (auto bpmOpt = posOpt->getBpm()) {
                if (*bpmOpt > 20.0 && *bpmOpt < 400.0) {
                    currentBpm = static_cast<float>(*bpmOpt);
                }
            }
        }
    }
    m_atomicHostBpm.store(currentBpm);

    int syncMode = static_cast<int>(getFloat(m_prmDelaySyncMode, 0.0f));
    int subdivIdx = static_cast<int>(getFloat(m_prmDelaySubdiv, 7.0f));
    float freeTimeMs = getFloat(m_prmDelayTime, 250.0f);
    float manualBpm = getFloat(m_prmDelayBpm, 120.0f);

    float effectiveDelayMs = calculateDelayTimeMs(syncMode, freeTimeMs, subdivIdx, currentBpm, manualBpm);
    m_atomicEffectiveDelayMs.store(effectiveDelayMs);

    m_efxRack.setDelayParams(getFloat(m_prmDelayEnabled, 0.0f) > 0.5f,
                            static_cast<int>(getFloat(m_prmDelayMode, 0.0f)),
                            effectiveDelayMs,
                            getFloat(m_prmDelayFeedback, 35.0f),
                            getFloat(m_prmDelayFreqShift, 0.0f),
                            getFloat(m_prmDelayDiffusion, 0.0f),
                            getFloat(m_prmDelayTone, 6000.0f),
                            getFloat(m_prmDelayMix, 30.0f));

    m_efxRack.process(outL, outR, numSamples);

    // Compute output RMS & sanitize
    float sumSqOut = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        float s = outL[i];
        if (!std::isfinite(s)) s = 0.0f;
        outL[i] = s;
        sumSqOut += s * s;
        if (outR) {
            float sr = outR[i];
            if (!std::isfinite(sr)) sr = 0.0f;
            outR[i] = sr;
            sumSqOut += sr * sr;
        }
    }
    float outRmsDb = juce::Decibels::gainToDecibels(std::sqrt(sumSqOut / static_cast<float>(numSamples * (outR ? 2 : 1))));

    // 8. Publish Telemetry via Lock-Free Atomics
    m_atomicDetectedFreq.store(rawDetectedHz);
    m_atomicTargetFreq.store(targetHz);
    m_atomicDetectedMidi.store(detMidi);
    m_atomicTargetMidi.store(tgtMidi);
    m_atomicCentsDev.store(centsDev);
    m_atomicAppliedCorr.store(appliedCorrCents);
    m_atomicVoicedConfidence.store(mpmConfidence);
    m_atomicIsVoiced.store(isVoiced);
    m_atomicCompGR.store(m_efxRack.getCompressorGainReductionDb());
    m_atomicHaasPhaseWarn.store(m_efxRack.getHaasPhaseWarning());
    m_atomicInputRmsDb.store(inRmsDb);
    m_atomicOutputRmsDb.store(outRmsDb);
}

juce::AudioProcessorEditor* FreePitchAudioProcessor::createEditor() {
    return new FreePitchAudioProcessorEditor(*this);
}

void FreePitchAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = m_apvts.copyState();
    state.setProperty("customScaleMask", static_cast<int>(m_customScaleMask.load()), nullptr);
    state.setProperty("isDarkTheme", m_isDarkTheme.load(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FreePitchAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(m_apvts.state.getType())) {
        juce::ValueTree state = juce::ValueTree::fromXml(*xmlState);
        m_apvts.replaceState(state);
        if (state.hasProperty("customScaleMask")) {
            m_customScaleMask.store(static_cast<uint32_t>(static_cast<int>(state.getProperty("customScaleMask"))));
        }
        if (state.hasProperty("isDarkTheme")) {
            m_isDarkTheme.store(static_cast<bool>(state.getProperty("isDarkTheme")));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FreePitchAudioProcessor();
}
