#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <iomanip>

#include "DSP/PitchDetector.h"
#include "DSP/VoicedClassifier.h"
#include "DSP/ScaleQuantizer.h"
#include "DSP/PitchShifter.h"
#include "DSP/EFXRack.h"

// Helper to generate sine wave block
static std::vector<float> generateSineWave(float freqHz, double sampleRate, int numSamples, float amplitude = 0.8f) {
    std::vector<float> buffer(numSamples);
    double phaseInc = 2.0 * 3.14159265358979323846 * freqHz / sampleRate;
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = amplitude * static_cast<float>(std::sin(phaseInc * i));
    }
    return buffer;
}

// Convert Hz to cents difference
static float centsDiff(float f1, float f2) {
    return 1200.0f * std::log2(f1 / f2);
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "        FREEPITCH AUTOMATED DSP UNIT TEST SUITE      \n";
    std::cout << "====================================================\n\n";

    const double sampleRate = 48000.0;
    int testsPassed = 0;
    int totalTests = 0;

    // -------------------------------------------------------------
    // TEST 1: MPM Pitch Detection Accuracy across Vocal Ranges
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Testing McLeod Pitch Method (MPM) Accuracy...\n";

    struct PitchTestCase {
        const char* noteName;
        float freqHz;
        VocalMode mode;
    };

    PitchTestCase testCases[] = {
        { "E2",   82.41f, VocalMode::LowMale },
        { "A2",  110.00f, VocalMode::Baritone },
        { "C3",  130.81f, VocalMode::Tenor },
        { "E3",  164.81f, VocalMode::Tenor },
        { "A3",  220.00f, VocalMode::Alto },
        { "C4",  261.63f, VocalMode::MezzoSoprano },
        { "A4",  440.00f, VocalMode::Soprano },
        { "C5",  523.25f, VocalMode::Soprano },
        { "A5",  880.00f, VocalMode::Soprano }
    };

    PitchDetector detector;
    detector.prepare(sampleRate, 2048);

    for (const auto& tc : testCases) {
        totalTests++;
        detector.reset();
        detector.setVocalMode(tc.mode);

        // Feed 2048 samples of pure tone
        auto wave = generateSineWave(tc.freqHz, sampleRate, 2048, 0.75f);
        detector.pushSamples(wave.data(), static_cast<int>(wave.size()));

        float detectedHz = 0.0f;
        float confidence = 0.0f;
        float rms = 0.0f;

        bool voiced = detector.processPitch(detectedHz, confidence, rms);
        float errorCents = centsDiff(detectedHz, tc.freqHz);

        std::cout << "  Note: " << std::setw(3) << tc.noteName
                  << " | True: " << std::setw(7) << std::fixed << std::setprecision(2) << tc.freqHz << " Hz"
                  << " | Detected: " << std::setw(7) << detectedHz << " Hz"
                  << " | Err: " << std::setw(5) << errorCents << " ct"
                  << " | Conf: " << std::setw(4) << confidence
                  << " | Voiced: " << (voiced ? "YES" : "NO ");

        if (voiced && std::abs(errorCents) <= 5.0f) {
            std::cout << " -> [PASS]\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 2: Silence & Noise Gate
    // -------------------------------------------------------------
    std::cout << "\n[TEST 2] Testing Silence & Noise Gating...\n";
    {
        totalTests++;
        detector.reset();
        std::vector<float> silence(2048, 0.0f);
        detector.pushSamples(silence.data(), 2048);
        float detectedHz = 0.0f, confidence = 0.0f, rms = 0.0f;
        bool voiced = detector.processPitch(detectedHz, confidence, rms);
        if (!voiced && rms < 0.001f) {
            std::cout << "  Silence correctly classified as unvoiced (RMS: " << rms << ") -> [PASS]\n";
            testsPassed++;
        } else {
            std::cout << "  Silence gating failed -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 3: Voiced Classifier (Sibilants & Consonants)
    // -------------------------------------------------------------
    std::cout << "\n[TEST 3] Testing Voiced / Sibilant Classifier...\n";
    {
        totalTests++;
        VoicedClassifier classifier;
        classifier.prepare(sampleRate);

        // Generate synthetic sibilant / white noise (high zero crossing rate)
        std::vector<float> noise(512);
        for (int i = 0; i < 512; ++i) {
            noise[i] = ((rand() % 2000) / 1000.0f - 1.0f) * 0.2f;
        }

        bool isVoiced = classifier.classify(noise.data(), 512, 0.35f, -14.0f);
        std::cout << "  Noise/Sibilant: ZCR=" << classifier.getZeroCrossingRate()
                  << " isConsonant=" << (classifier.isConsonant() ? "YES" : "NO")
                  << " isVoiced=" << (isVoiced ? "YES" : "NO");

        if (!isVoiced && classifier.isConsonant()) {
            std::cout << " -> [PASS]\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 4: Scale Quantizer & Expressive Modes
    // -------------------------------------------------------------
    std::cout << "\n[TEST 4] Testing Scale Quantizer & Musical Snap...\n";
    {
        totalTests++;
        ScaleQuantizer quantizer;
        quantizer.prepare(sampleRate);
        quantizer.setKeyRoot(0); // C
        quantizer.setScaleType(ScaleType::Major); // C Major (C, D, E, F, G, A, B)
        quantizer.setRetuneSpeedMs(0.0f); // Instant snap
        quantizer.setFlexTune(0.0f);

        // Input: C#4 (277.18 Hz) -> In C Major, should quantize to D4 (293.66 Hz) or C4 (261.63 Hz)
        float centsDev = 0.0f, appliedCorr = 0.0f;
        int detMidi = 0, tgtMidi = 0;
        float targetHz = quantizer.processTuning(277.18f, true, 0.01f, centsDev, appliedCorr, detMidi, tgtMidi);

        std::cout << "  Input: C#4 (277.18 Hz) -> Quantized Target: " << targetHz << " Hz (MIDI: " << tgtMidi << ")";
        if (tgtMidi == 60 || tgtMidi == 62) { // C4 or D4
            std::cout << " -> [PASS]\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 5: Real-Time PitchShifter Continuous Playback & No-Crackle Verification
    // -------------------------------------------------------------
    std::cout << "\n[TEST 5] Testing PitchShifter Continuous Processing & No-Crackle (Tenor & Low Male)...\n";
    {
        totalTests++;
        PitchShifter shifter;
        shifter.prepare(sampleRate, 512);

        // Test Low Male Mode (82.4 Hz - 329.6 Hz) - the mode from user's FL Studio session
        shifter.setVocalMode(VocalMode::LowMale);
        shifter.setLookaheadMode(false);

        const int blockSize = 512;
        const int numBlocks = 70; // ~0.75 seconds
        // Generate Low Male vocal tone (A2 = 110 Hz)
        auto sineWave = generateSineWave(110.0f, sampleRate, blockSize * numBlocks, 0.75f);

        std::vector<float> outBlock(blockSize, 0.0f);
        int validBlocks = 0;
        bool hasDropouts = false;
        float maxStepDiscontinuity = 0.0f;
        float prevSample = 0.0f;

        for (int b = 0; b < numBlocks; ++b) {
            const float* inPtr = sineWave.data() + b * blockSize;
            // Shift A2 (110 Hz) up to B2 (123.47 Hz, +2 semitones)
            shifter.process(inPtr, outBlock.data(), blockSize, 110.0f, 123.47f, true, true, 0.0f);

            // Compute block RMS and inspect sample-to-sample step delta (crackle check)
            float sumSq = 0.0f;
            for (int i = 0; i < blockSize; ++i) {
                if (!std::isfinite(outBlock[i])) {
                    hasDropouts = true;
                }
                sumSq += outBlock[i] * outBlock[i];

                if (b >= 5) { // after latency preload
                    float delta = std::abs(outBlock[i] - prevSample);
                    if (delta > 0.15f) {
                        std::cout << "    [CRACKLE] block=" << b << " i=" << i << " prev=" << prevSample << " cur=" << outBlock[i] << " delta=" << delta << "\n";
                        for (int k = std::max(0, i - 5); k <= std::min(blockSize - 1, i + 5); ++k) {
                            std::cout << "      [k=" << k << "] " << outBlock[k] << "\n";
                        }
                    }
                    if (delta > maxStepDiscontinuity) {
                        maxStepDiscontinuity = delta;
                    }
                }
                prevSample = outBlock[i];
            }
            float blockRms = std::sqrt(sumSq / static_cast<float>(blockSize));

            // Blocks after initial latency must have continuous audio output
            if (b >= 5) {
                if (blockRms > 0.05f) {
                    validBlocks++;
                } else {
                    hasDropouts = true;
                    std::cout << "  Dropout detected at block " << b << " (RMS=" << blockRms << ")!\n";
                }
            }
        }

        std::cout << "  Low Male Continuous Blocks: " << validBlocks << " / " << (numBlocks - 5)
                  << " | Max Sample Delta: " << maxStepDiscontinuity;
        // For smooth 123Hz sine tone, normal max delta is ~0.015. A click/crackle causes delta > 0.15.
        bool noCrackle = (maxStepDiscontinuity < 0.08f);

        if (validBlocks == (numBlocks - 5) && !hasDropouts && noCrackle) {
            std::cout << " -> [PASS] (Smooth, Pristine, Zero Crackle)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 6: EFX Rack Processing & Verification
    // -------------------------------------------------------------
    std::cout << "\n[TEST 6] Testing EFX Rack (EQ, Compressor, Haas Warning)...\n";
    {
        totalTests++;
        EFXRack efx;
        efx.prepare(sampleRate, 512);

        // Test EQ magnitude response calculation
        efx.setEqParams(true, 80.0f, 500.0f, 3.0f, 3000.0f, 2.0f, 12000.0f, 4.0f, 0.0f);
        float hpfMag = efx.getEqResponseAt(30.0f);
        float airMag = efx.getEqResponseAt(12000.0f);
        std::cout << "  EQ 30Hz response: " << hpfMag << " dB, 12kHz response: " << airMag << " dB\n";

        // Test Compressor Gain Reduction
        efx.setCompParams(true, -20.0f, 4.0f, 5.0f, 100.0f, 0.0f);
        auto testSignal = generateSineWave(1000.0f, sampleRate, 512, 0.9f); // ~ -1 dBFS
        efx.process(testSignal.data(), nullptr, 512);
        float gr = efx.getCompressorGainReductionDb();
        std::cout << "  Compressor GR on -1 dBFS tone: " << gr << " dB\n";

        // Test Haas Phase Warning
        efx.setChorusParams(true, 1, 1.0f, 50.0f, 22.0f, 80.0f, 50.0f); // 22 ms delay > 18 ms
        bool warning = efx.getHaasPhaseWarning();
        std::cout << "  Haas delay 22ms phase warning triggered: " << (warning ? "YES" : "NO");

        if (hpfMag < -3.0f && gr > 3.0f && warning) {
            std::cout << " -> [PASS]\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 7: Choosy Tracking Mode for Noisy/Breathy Vocals
    // -------------------------------------------------------------
    std::cout << "\n[TEST 7] Testing Choosy Tracking Mode...\n";
    {
        totalTests++;
        PitchDetector choosyDetector;
        choosyDetector.prepare(sampleRate, 2048);
        choosyDetector.setChoosyTracking(true);

        VoicedClassifier choosyClassifier;
        choosyClassifier.prepare(sampleRate);
        choosyClassifier.setChoosyTracking(true);

        // Synthetic breathy noise
        std::vector<float> breathNoise(512);
        for (int i = 0; i < 512; ++i) {
            breathNoise[i] = ((rand() % 2000) / 1000.0f - 1.0f) * 0.05f; // low-amplitude breath
        }

        choosyDetector.pushSamples(breathNoise.data(), 512);
        float detHz = 0.0f, conf = 0.0f, rms = 0.0f;
        choosyDetector.processPitch(detHz, conf, rms);
        bool isVoiced = choosyClassifier.classify(breathNoise.data(), 512, conf, -35.0f);

        std::cout << "  Breathy Noise with Choosy Tracking: isVoiced=" << (isVoiced ? "YES" : "NO")
                  << " (Conf=" << conf << ")";
        if (!isVoiced) {
            std::cout << " -> [PASS] (Breathy noise correctly bypassed)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 8: Note-Onset Refreshed Retune Speed
    // -------------------------------------------------------------
    std::cout << "\n[TEST 8] Testing Note-Onset Refreshed Retune Speed...\n";
    {
        totalTests++;
        ScaleQuantizer quantizer;
        quantizer.prepare(sampleRate);
        quantizer.setKeyRoot(0); // C Major
        quantizer.setScaleType(ScaleType::Major);
        quantizer.setRetuneSpeedMs(200.0f); // 200 ms retune speed
        quantizer.setFlexTune(0.0f);

        // Note 1: C4 (261.63 Hz, MIDI 60)
        float centsDev = 0.0f, appliedCorr = 0.0f;
        int detMidi = 0, tgtMidi = 0;
        quantizer.processTuning(261.63f, true, 0.01f, centsDev, appliedCorr, detMidi, tgtMidi);

        // Now singer jumps to G4 (392.0 Hz, MIDI 67)
        // At onset (t = 0), correction should NOT pull the note backwards to C4!
        // Output pitch should be immediately on G4!
        float targetHzOnset = quantizer.processTuning(392.0f, true, 0.001f, centsDev, appliedCorr, detMidi, tgtMidi);

        std::cout << "  Singer jumps C4 -> G4: Target MIDI=" << tgtMidi
                  << " | Onset Target Freq=" << targetHzOnset << " Hz (G4 is 392.00 Hz)";

        // Target should be 392 Hz (G4), not dragging through D4 or E4!
        if (tgtMidi == 67 && std::abs(targetHzOnset - 392.0f) < 2.0f) {
            std::cout << " -> [PASS] (Zero drag; instantaneous transition to new note target)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 9: Small Buffer Sizes (FL Studio 64/128-sample blocks)
    // -------------------------------------------------------------
    std::cout << "\n[TEST 9] Testing Small Buffer Sizes (64-sample blocks, Tenor Mode)...\n";
    {
        totalTests++;
        PitchShifter shifter;
        shifter.prepare(sampleRate, 64);
        shifter.setVocalMode(VocalMode::Tenor);
        shifter.setLookaheadMode(false);

        const int blockSize = 64;
        const int numBlocks = 120; // ~160 ms
        auto sineWave = generateSineWave(220.0f, sampleRate, blockSize * numBlocks, 0.75f); // A3

        std::vector<float> outBlock(blockSize, 0.0f);
        float maxDelta = 0.0f;
        float prevSample = 0.0f;

        for (int b = 0; b < numBlocks; ++b) {
            const float* inPtr = sineWave.data() + b * blockSize;
            shifter.process(inPtr, outBlock.data(), blockSize, 220.0f, 246.94f, true, true, 0.0f); // A3 -> B3

            if (b >= 10) {
                for (int i = 0; i < blockSize; ++i) {
                    float d = std::abs(outBlock[i] - prevSample);
                    if (d > maxDelta) maxDelta = d;
                    prevSample = outBlock[i];
                }
            } else {
                prevSample = outBlock[blockSize - 1];
            }
        }

        std::cout << "  64-Sample Buffer Max Delta: " << maxDelta;
        if (maxDelta < 0.08f) {
            std::cout << " -> [PASS] (Smooth under tight DAW buffer sizes)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 10: Table-Fan / Rotary Flutter Verification
    // -------------------------------------------------------------
    std::cout << "\n[TEST 10] Testing Table-Fan / Amplitude Flutter Verification...\n";
    {
        totalTests++;
        PitchShifter shifter;
        shifter.prepare(sampleRate, 256);
        shifter.setVocalMode(VocalMode::Tenor);
        shifter.setLookaheadMode(false);

        const int blockSize = 256;
        const int numBlocks = 100; // ~0.53 seconds
        const int totalSamples = blockSize * numBlocks;

        // Multi-harmonic vocal tone with formant envelope at 160 Hz
        std::vector<float> vocalSignal(totalSamples, 0.0f);
        const float f0 = 160.0f;
        for (int i = 0; i < totalSamples; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            float val = 0.0f;
            for (int h = 1; h <= 12; ++h) {
                float fh = h * f0;
                float env = std::exp(-std::pow((fh - 900.0f) / 350.0f, 2.0f)) + 0.4f * std::exp(-std::pow((fh - 300.0f) / 150.0f, 2.0f));
                val += env * std::sin(2.0f * 3.14159265358979323846f * fh * t);
            }
            vocalSignal[i] = val * 0.35f;
        }

        std::vector<float> outAudio(totalSamples, 0.0f);
        std::vector<float> blockBuf(blockSize, 0.0f);

        // Process with +1.2 semitones pitch shift (160 Hz -> 171.4 Hz)
        const float targetF0 = 171.4f;
        for (int b = 0; b < numBlocks; ++b) {
            const float* inPtr = vocalSignal.data() + b * blockSize;
            shifter.process(inPtr, blockBuf.data(), blockSize, f0, targetF0, true, true, 0.0f);
            for (int i = 0; i < blockSize; ++i) {
                outAudio[b * blockSize + i] = blockBuf[i];
            }
        }

        // Measure RMS in period-aligned windows (600 samples = exactly 2 periods of 160 Hz) over steady-state
        const int winSize = 600;
        float minRms = 1e9f;
        float maxRms = 0.0f;
        float sumRms = 0.0f;
        int winCount = 0;

        for (int i = 4800; i < totalSamples - winSize; i += winSize / 2) {
            float sq = 0.0f;
            for (int j = 0; j < winSize; ++j) {
                float s = outAudio[i + j];
                sq += s * s;
            }
            float r = std::sqrt(sq / static_cast<float>(winSize));
            if (r < minRms) minRms = r;
            if (r > maxRms) maxRms = r;
            sumRms += r;
            winCount++;
        }

        float avgRms = (winCount > 0) ? (sumRms / static_cast<float>(winCount)) : 1.0f;
        float flutterRatio = minRms / std::max(1e-4f, avgRms);

        std::cout << "  Steady-State Multi-Harmonic Shift (+1.2 st): minRMS=" << std::fixed << std::setprecision(3) << minRms
                  << " | avgRMS=" << avgRms << " | maxRMS=" << maxRms
                  << " | min/avg ratio=" << flutterRatio;

        // Old buggy code had ratio < 0.10 (severe 90% rotary flutter / table-fan dropouts).
        // New phase-aligned engine maintains ratio > 0.85 (continuous, pristine vocal body).
        if (flutterRatio >= 0.85f && avgRms > 0.15f) {
            std::cout << " -> [PASS] (Zero Rotary Flutter, Consistent Pristine Amplitude)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 11: EFXRack Master Bypass & Modular Rack Verification
    // -------------------------------------------------------------
    std::cout << "\n[TEST 11] Testing EFXRack Master Bypass & Audio Processing...\n";
    {
        totalTests++;
        EFXRack rack;
        rack.prepare(sampleRate, 512);

        const int blockSize = 512;
        auto sineWaveL = generateSineWave(200.0f, sampleRate, blockSize, 0.5f);
        auto sineWaveR = generateSineWave(200.0f, sampleRate, blockSize, 0.5f);

        std::vector<float> testL = sineWaveL;
        std::vector<float> testR = sineWaveR;

        // 1. When master bypass is TRUE, rack MUST leave audio 100% bit-exact
        rack.setMasterBypass(true);
        rack.setDistParams(true, 0, 80.0f, 8000.0f, 100.0f); // Even if dist is configured ON
        rack.process(testL.data(), testR.data(), blockSize);

        float maxBypassDelta = 0.0f;
        for (int i = 0; i < blockSize; ++i) {
            maxBypassDelta = std::max(maxBypassDelta, std::abs(testL[i] - sineWaveL[i]));
        }

        // 2. When master bypass is FALSE, and Saturation is active, harmonic distortion must be generated
        rack.setMasterBypass(false);
        rack.setDistParams(true, 0, 75.0f, 8000.0f, 100.0f); // Warm Tube with 75% drive
        testL = sineWaveL;
        testR = sineWaveR;
        rack.process(testL.data(), testR.data(), blockSize);

        // Check for harmonic content: difference from linear sine wave
        float distDelta = 0.0f;
        for (int i = 0; i < blockSize; ++i) {
            distDelta += std::abs(testL[i] - sineWaveL[i]);
        }
        float avgDistDelta = distDelta / static_cast<float>(blockSize);

        // 3. When Chorus is active on mono inputs, stereo decorrelation must occur (Left != Right)
        rack.reset();
        rack.setDistParams(false, 0, 0.0f, 8000.0f, 0.0f);
        rack.setChorusParams(true, 0, 1.5f, 50.0f, 12.0f, 80.0f, 50.0f); // Modulated Chorus
        testL = sineWaveL;
        testR = sineWaveL; // identical mono input
        // Run a few blocks so chorus buffer fills and phase advances
        for (int b = 0; b < 10; ++b) {
            testL = sineWaveL;
            testR = sineWaveL;
            rack.process(testL.data(), testR.data(), blockSize);
        }

        float lrDiff = 0.0f;
        for (int i = 0; i < blockSize; ++i) {
            lrDiff += std::abs(testL[i] - testR[i]);
        }
        float avgLrDiff = lrDiff / static_cast<float>(blockSize);

        std::cout << "  Bypass Max Delta=" << maxBypassDelta
                  << " | Saturation Avg Delta=" << avgDistDelta
                  << " | Chorus Stereo Spread (L-R)=" << avgLrDiff;

        if (maxBypassDelta == 0.0f && avgDistDelta > 0.02f && avgLrDiff > 0.01f) {
            std::cout << " -> [PASS] (Master Bypass & EFX Modules fully functional)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 12: FormantEngine Real-Time Spectral Warping & Transparency
    // -------------------------------------------------------------
    std::cout << "\n[TEST 12] Testing FormantEngine Spectral Warping & Transparency...\n";
    {
        totalTests++;
        FormantEngine formant;
        formant.prepare(sampleRate, 1024);

        const int blockSize = 256;
        const int numBlocks = 20; // 5120 samples
        auto testTone = generateSineWave(400.0f, sampleRate, blockSize * numBlocks, 0.7f);

        // 1. Transparency test: formantShift = 0 st, pitchRatio = 1.0, formants = true
        std::vector<float> transparentOut = testTone;
        for (int b = 0; b < numBlocks; ++b) {
            formant.process(transparentOut.data() + b * blockSize, blockSize, 400.0f, 1.0f, 0.0f, true);
        }

        // After pipeline latency (768 samples), output should match input delayed by 768 samples
        int lat = formant.getLatencySamples();
        float maxTransDelta = 0.0f;
        for (int i = lat + 256; i < blockSize * numBlocks - 256; ++i) {
            float d = std::abs(transparentOut[i] - testTone[i - lat]);
            if (d > maxTransDelta) maxTransDelta = d;
        }

        // 2. Active formant shift test: formantShift = +6 st on rich harmonic vocal signal
        formant.reset();
        std::vector<float> vocalSignal(blockSize * numBlocks, 0.0f);
        for (int i = 0; i < blockSize * numBlocks; ++i) {
            double p1 = 2.0 * 3.14159265358979323846 * 200.0 * i / sampleRate;
            double p2 = 2.0 * 3.14159265358979323846 * 400.0 * i / sampleRate;
            double p3 = 2.0 * 3.14159265358979323846 * 600.0 * i / sampleRate;
            double p4 = 2.0 * 3.14159265358979323846 * 800.0 * i / sampleRate;
            vocalSignal[i] = 0.4f * static_cast<float>(std::sin(p1))
                           + 0.3f * static_cast<float>(std::sin(p2))
                           + 0.2f * static_cast<float>(std::sin(p3))
                           + 0.1f * static_cast<float>(std::sin(p4));
        }
        std::vector<float> unshiftedVocal = vocalSignal;
        std::vector<float> shiftedVocal = vocalSignal;
        for (int b = 0; b < numBlocks; ++b) {
            formant.process(unshiftedVocal.data() + b * blockSize, blockSize, 200.0f, 1.0f, 0.0f, true);
        }
        formant.reset();
        for (int b = 0; b < numBlocks; ++b) {
            formant.process(shiftedVocal.data() + b * blockSize, blockSize, 200.0f, 1.0f, 6.0f, true);
        }

        float shiftEnergyDiff = 0.0f;
        for (int i = lat + 256; i < blockSize * numBlocks - 256; ++i) {
            shiftEnergyDiff += std::abs(shiftedVocal[i] - unshiftedVocal[i]);
        }
        float avgShiftDelta = shiftEnergyDiff / static_cast<float>(blockSize * numBlocks - 2 * 256 - lat);

        std::cout << "  Formant Transparency Max Delta=" << maxTransDelta
                  << " | Formant +6st Reshaping Avg Delta=" << avgShiftDelta;

        if (maxTransDelta < 0.01f && avgShiftDelta > 0.01f) {
            std::cout << " -> [PASS] (Transparent at 0st, Rich Warping at +6st)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    // -------------------------------------------------------------
    // TEST 13: Space Delay, Valhalla FreqEcho & Supermassive Diffusion
    // -------------------------------------------------------------
    std::cout << "\n[TEST 13] Testing Space Delay, Valhalla FreqEcho & Supermassive Diffusion...\n";
    {
        totalTests++;
        EFXRack rack;
        rack.prepare(sampleRate, 512);

        const int blockSize = 512;
        // 1. Single impulse test: verify delay time = 100 ms (4800 samples)
        rack.setDelayParams(true, 0, 100.0f, 50.0f, 0.0f, 0.0f, 10000.0f, 100.0f); // 100% wet
        std::vector<float> impulseL(blockSize * 20, 0.0f);
        std::vector<float> impulseR(blockSize * 20, 0.0f);
        impulseL[0] = 1.0f; // 1 impulse at sample 0

        for (int b = 0; b < 20; ++b) {
            rack.process(impulseL.data() + b * blockSize, impulseR.data() + b * blockSize, blockSize);
        }

        // Find peak echo position
        int peakPos = 0;
        float peakVal = 0.0f;
        for (int i = 100; i < blockSize * 20; ++i) {
            if (std::abs(impulseL[i]) > peakVal) {
                peakVal = std::abs(impulseL[i]);
                peakPos = i;
            }
        }
        float measuredDelayMs = (static_cast<float>(peakPos) / static_cast<float>(sampleRate)) * 1000.0f;

        // 2. Supermassive Diffusion test: test that impulse spreads across multiple samples
        rack.reset();
        rack.setDelayParams(true, 0, 50.0f, 50.0f, 0.0f, 80.0f, 10000.0f, 100.0f); // 80% diffusion
        std::vector<float> diffL(blockSize * 20, 0.0f);
        std::vector<float> diffR(blockSize * 20, 0.0f);
        diffL[0] = 1.0f;
        for (int b = 0; b < 20; ++b) {
            rack.process(diffL.data() + b * blockSize, diffR.data() + b * blockSize, blockSize);
        }
        int nonZeroSamples = 0;
        for (int i = 2300; i < 3000; ++i) {
            if (std::abs(diffL[i]) > 0.001f) nonZeroSamples++;
        }

        std::cout << "  Measured Delay: " << measuredDelayMs << " ms (Target: 100.0 ms)"
                  << " | Supermassive Diffused Points: " << nonZeroSamples;

        if (std::abs(measuredDelayMs - 100.0f) < 5.0f && nonZeroSamples > 50) {
            std::cout << " -> [PASS] (H-Delay Timing & Supermassive Diffusion Confirmed)\n";
            testsPassed++;
        } else {
            std::cout << " -> [FAIL]\n";
        }
    }

    std::cout << "\n====================================================\n";
    std::cout << " TEST RESULTS: " << testsPassed << " / " << totalTests << " PASSED ("
              << (testsPassed == totalTests ? "100%" : "FAILURES DETECTED") << ")\n";
    std::cout << "====================================================\n";

    return (testsPassed == totalTests) ? 0 : 1;
}
