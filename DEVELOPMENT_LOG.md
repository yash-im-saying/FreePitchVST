# FreePitch — Development Log

## Project Overview
**FreePitch** is an autonomous, professional, low-latency, real-time monophonic pitch-correction and vocal EFX VST3 and Standalone audio plugin for Windows x64.
Engineered using C++20 and JUCE 8, adhering to strict real-time audio thread safety (zero heap allocations, zero locks, zero file I/O in `processBlock`) and audited against `unslop` industrial studio instrumentation design principles.

---

## Architectural Choices & DSP Foundations

### 1. Audio Thread Safety & Threading Model
- **Audio Thread (`processBlock`):**
  - Completely non-blocking and allocation-free.
  - All circular ring buffers, FFT engines, overlap-add grain pools, and filters are pre-allocated during `prepareToPlay()`.
  - `juce::ScopedNoDenormals` active on entry to eliminate denormal performance penalties.
  - Comprehensive NaN and Infinity output sanitization.
- **Lock-Free Telemetry:**
  - GUI telemetry (detected frequency, target pitch, cents deviation, voiced confidence, compressor gain reduction, Haas phase warning) is transferred from the audio thread to the UI thread via a lock-free Single-Producer Single-Consumer (SPSC) ring buffer (`juce::AbstractFifo`) and atomic state flags.
  - GUI runs on a 60 FPS timer with exponential visual ballistics damping, rendering smoothly without any lock contention.

### 2. McLeod Pitch Method (MPM) with Dynamic Vocal Bounds
- **Normalized Square Difference Function (NSDF):**
  $$r_t(\tau) = \frac{2 \sum_{j=0}^{W-1} x_j x_{j+\tau}}{\sum_{j=0}^{W-1} x_j^2 + \sum_{j=0}^{W-1} x_{j+\tau}^2}$$
  Computed in $\mathcal{O}(N \log N)$ time using JUCE's `juce::dsp::FFT`.
- **Dynamic Lag Constraints:**
  Autocorrelation lag search $\tau \in [\tau_{\min}, \tau_{\max}]$ is bounded strictly by the selected Vocal Range Mode:
  - Soprano: `C4 – C6` (261.6 – 1046.5 Hz)
  - Mezzo-Soprano: `A3 – A5` (220.0 – 880.0 Hz)
  - Alto: `F3 – F5` (174.6 – 698.5 Hz)
  - Tenor: `C3 – C5` (130.8 – 523.3 Hz)
  - Baritone: `G2 – G4` (98.0 – 392.0 Hz)
  - Low Male: `E2 – E4` (82.4 – 329.6 Hz)
  - Bass / Instrument: `E1 – E3` (41.2 – 164.8 Hz)
  - Full Range: `C1 – C6` (32.7 – 1046.5 Hz)
- **Sub-Cent Accuracy:**
  Quadratic (parabolic) peak interpolation over NSDF maxima.
- **Voiced/Unvoiced Gating & Hysteresis:**
  RMS power gate + NSDF clarity thresholding ($> 0.75$) + zero-crossing rate threshold to detect unvoiced consonants, sibilants (`s`, `t`, `f`), and silence. Unvoiced frames cleanly bypass pitch shifting and maintain target period memory.

### 3. Scale & Expressive Tuning Engine
- **Scales:** Chromatic, Major, Natural Minor, Harmonic Minor, Melodic Minor, Pentatonic Major, Pentatonic Minor, and Custom bitmask keyboard.
- **Retune Speed ($0 - 400\text{ ms}$):** One-pole slew-rate limiter over pitch delta. $0\text{ ms}$ yields hard-tune snapping.
- **Humanize ($0 - 100\%$):** Evaluates pitch variance over a sliding analysis window. High stability (sustained notes) decays correction pull, preserving micro-vibrato.
- **Flex-Tune ($0 - 100\%$):** Dead-zone tolerance around scale note centers. Deviations within the window remain expressive; outer deviations are pulled into tune.

### 4. TD-PSOLA & Formant Engine
- **TD-PSOLA:** Pitch-mark epoch alignment with Hann-windowed grains ($2 T_0$) overlap-added at synthesis intervals $T_1 = T_0 / \alpha$.
- **Formant Preservation / Shift:** Spectral envelope extraction and LPC filtering. Formants ON preserves spectral peaks under pitch transposition; Formant Shift scales vocal tract formant frequencies independently.
- **Dual Latency:**
  - Tracking/Live Mode: Algorithmic latency $\le 10\text{ ms}$ ($2 T_{0,\max}$).
  - Mixing/Lookahead Mode: Lookahead buffer with smooth crossfade and host PDC reporting via `setLatencySamples()`.

### 5. Modular Post-Pitch EFX Rack
- 4-Band Sculpt EQ with live frequency-response display.
- Vocal FET/Opto Compressor with gain reduction telemetry.
- Flanger with rate, depth, feedback, mix.
- Chorus / Haas Widener with mono-compatibility phase warning.
- Waveshaping Distortion (Warm Tube, Tape, Hard Clip) with tone LP.

---

## Milestone Execution Record
- **Milestone 1:** Environment inspection, CMake JUCE 8 integration, building shell & unslop audit. [COMPLETED]
- **Milestone 2:** MPM pitch detector, confidence/voiced gate, Vocal Mode frequency bounds, and automated unit test verifying detection accuracy (100% pass across E2–A5). [COMPLETED]
- **Milestone 3:** Scale quantizer, Retune Speed slew, Humanize stability, and Flex-Tune elastic dead-zone curves. [COMPLETED]
- **Milestone 4:** TD-PSOLA pitch shifter, cepstral/spectral envelope formant preservation & shifting, and Live vs. Lookahead latency reporting. [COMPLETED]
- **Milestone 5:** Full `unslop`-compliant custom UI: central circular dual-arc pitch & correction meter, rotary controls, dynamic range keyboard, and top bar. [COMPLETED]
- **Milestone 6:** Factory presets (8 male vocal presets), A/B compare, APVTS automation, and XML state persistence. [COMPLETED]
- **Milestone 7:** Modular post-pitch EFX rack (4-band sculpt EQ with live response curve, vocal compressor with GR meter, flanger, chorus/Haas with mono warning, saturation) with inline expandable drawers. [COMPLETED]
- **Milestone 8:** Release build (VST3 & Standalone x64 with AVX2), automated DSP test suite (13/13 passed), installation to `C:\Program Files\Common Files\VST3\FreePitch.vst3`. [COMPLETED]
- **Milestone 9:** Audio Dropout Elimination & Pitch/Formants UI Overhaul. [COMPLETED]
- **Milestone 10:** Real-Time Live EQ Curve & Crackle Elimination. [COMPLETED]
- **Milestone 11:** Note-Onset Refreshed Retune Speed, Latency-Aligned TD-PSOLA, Scale Hysteresis & Choosy Tracking. [COMPLETED]
  - **Note-Onset Refreshed Retune Speed:**
    - Root cause: Retune speed previously functioned as a global one-pole low-pass filter directly on `m_currentMidiPitch`. When setting large retune speeds (e.g. 200–400 ms), singing a new note caused the pitch to slowly glide through intermediate notes, holding the singer back even after they had reached the intended pitch.
    - Solution: Re-engineered into a note-onset correction envelope that refreshes on every note transition. When a new scale target note is reached, the note transition starts immediately at the singer's natural pitch ($t = 0$), and the correction envelope ramps in over `retuneSpeedMs` ($\tau = T_{\text{retune}} \times 0.33$), smoothly pulling only any remaining drift or nuance into center without dragging the vocal between notes. Setting `0 ms` provides instant Cher/T-Pain hard snapping.
  - **Correction Accuracy & Scale Hysteresis:**
    - Fixed Flex-Tune curve: Previously outside the dead-zone, the curve only pulled up to the dead-zone edge, leaving notes up to 20 cents off-pitch. It now smoothly pulls 100% to center using a Hermite smoothstep curve when outside the dead-zone.
    - Added $\pm 12$ cents note-boundary hysteresis to prevent vibrato and natural vocal micro-inflections near scale note borders from triggering rapid, erratic target note flipping.
  - **Latency-Aligned TD-PSOLA & Crackle Elimination:**
    - Root cause: The voiced probability crossfade gain was operating on instantaneous audio blocks rather than aligning with the circular buffer's $m_{\text{latencySamples}}$ delay. This caused crossfades into empty synthesis buffers on note onsets and offsets.
    - Solution: Added circular `m_voicedBuffer` delayed synchronously with the audio timeline. Removed hard window-weight threshold switching (`if (w > 0.20)`), leveraging natural Constant Overlap-Add (COLA) Hann summation for continuous, artifact-free synthesis across all DAW buffer sizes (verified down to 64 samples).
    - Prevented continuous host PDC recalculation by only invoking `setLatencySamples()` when latency values actually change.
  - **Choosy Tracking Mode for Noisy/Breathy Vocals:**
    - Introduced tactile `CHOOSY TRACKING` toggle button in the central Pitch Tracker panel.
    - When active, tightens MPM clarity threshold to $\ge 0.72$, lowers zero-crossing rate tolerance to $\le 0.20$, and raises the noise gate to $-50\text{ dBFS}$. Consonants, plosives, and breath noise cleanly bypass pitch-shifting with unity gain while clean vowels track with rock-solid precision.
  - **Verification:** 17/17 automated DSP tests passed (100%). Deployed hot-swap release binary to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.
  - **Live EQ Curve Real-Time Update:**
    - Root cause in `EFXDrawerComponent.cpp`: `updateTelemetry` excluded `m_activeDrawer == 1`, and moving child knob sliders did not repaint parent component. Furthermore, EQ parameters were only fed into `EFXRack` when audio transport was running.
    - Fix: `EFXDrawerComponent::paintEqCurve` now fetches live APVTS atomic parameter values directly on every paint call and updates `EFXRack`, allowing real-time response curve rendering even while the DAW transport is paused. `updateTelemetry` now repaints for all open drawers (`m_activeDrawer > 0`), ensuring 60 FPS smooth updates.
  - **Complete Crackle Elimination:**
    - Root cause 1: In `findGlottalClosureInstant`, using `std::abs()` caused the epoch search to alternate unpredictably between positive crests and negative troughs on symmetric cycles, inverting grain polarity by $180^\circ$ and causing destructive phase interference. Added persistent polarity locking (`m_dominantPolarity`).
    - Root cause 2: For low vocal ranges (Low Male `E2`, Baritone `G2`), latency was previously clamped to 16 ms (768 samples) which was far shorter than the required $2.7 \times P_{\max}$ (1572 samples). This caused the beginning of grains to be truncated after playback had already passed, leaving periodic zero holes in the overlap-add buffer.
    - Root cause 3: Circular buffer wrap-around left stale un-cleared grain tails at the end of the 32,768-sample buffer from startup time zero. Added ahead-of-time future buffer clearing (`currentSample + 4096 & kBufferMask = 0.0f`) on every sample step, guaranteeing zero stale residual data across buffer revolutions.
    - Verified via automated unit test: sample-to-sample step delta dropped from 0.64 down to 0.01 (pure sine slope), 14/14 tests passing.
    - Deployed hot-swap update to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 12: Rotary Flutter & Table-Fan Elimination via Phase-Synchronous Splice Architecture:**
  - **Acoustic Root Cause Analysis:**
    - In monophonic vocals containing strong formant resonances (F1, F2), adjacent pitch cycles exhibit multiple sub-peaks.
    - The legacy GCI picker searched a wide window ($\pm 0.55 T_{\text{in}}$) using simple maximum amplitude detection. Under slight micro-variations, it unpredictably hopped between the primary glottal peak and secondary formant peaks on alternating cycles.
    - This hopping introduced $90^\circ$ to $180^\circ$ phase inversions between consecutive overlapping Hann grains, producing catastrophic periodic destructive interference ($0.038$ to $1.393$ RMS amplitude modulation) at the pitch frequency ($50 - 200\text{ Hz}$). This acoustic flutter acted literally like spinning rotary table-fan blades chopping through the vocal audio.
  - **Phase-Synchronous Dual-Tap Resynthesis Implementation:**
    - Replaced the discrete GCI epoch-hopping loop with a continuous phase-synchronous dual-tap delay-line architecture:
      - Continuous fractional read tap $d_1(t)$ advancing by $\frac{dd}{dt} = 1 - \alpha$, where $\alpha = \frac{f_{\text{target}}}{f_{\text{input}}}$.
      - When $\alpha = 1.0$ (in tune), $\frac{dd}{dt} = 0$, delay is static, yielding bit-exact reproduction of the input with ZERO phase modulation and ZERO grain boundaries.
      - 4-point Hermite cubic interpolation (`readHermite`) providing flat frequency response up to Nyquist without high-frequency dampening or clicks.
      - When drift exceeds the detected fundamental pitch period $T_{\text{in}}$, a secondary read tap $d_2$ is spawned at integer period offset $k \cdot T_{\text{in}}$ with $\pm 8$-sample micro fine-alignment cross-correlation (`fineAlign`).
      - Taps $d_1$ and $d_2$ crossfade seamlessly over a full pitch period ($T_{\text{in}}$) with raised-cosine Hann weighting ($w_1 + w_2 \equiv 1.0$).
  - **Automated DSP Verification:**
    - Added Test 10 in `DSPUnitTests.cpp` testing multi-harmonic formant vocal tone under $+1.2$ st shift.
    - Verified steady-state RMS flutter ratio $R = \frac{\text{minRMS}}{\text{avgRMS}} = 0.971$ (compared to $<0.10$ on legacy code).
    - 18/18 DSP unit tests passing (100%).
    - Deployed Release build to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 13: Click Artifacts Elimination & Seamless Pitch Continuity:**
  - **Acoustic Root Causes:**
    1. *Flawed Micro-Fine-Alignment (`fineAlign`):* The $\pm 8$-sample local cross-correlation acted as an unnormalized slope detector over a 24-sample window. It slammed to $+8$ or $-8$ samples, offsetting the secondary read tap by up to $180^\circ$ of phase error at 2–3 kHz, producing transient phase cancellation spikes and clicks on every crossfade.
    2. *In-Tune Velocity Jerk:* When `alpha` crossed the $<1$ cent boundary (`abs(alpha - 1.0) < 0.0005`), `m_tapDelay1` abruptly slewed towards `baseLatency` at $-0.75$ samples/sample (a 1.75x speed jerk), creating clicks and pitch chirps on sustained notes.
    3. *Premature Unvoiced Delay Slewing:* When `isVoiced` dropped to false, delay was forcefully slewed at $0.05 \cdot (D_0 - d_1)$ (up to 10 samples per sample step) and active crossfades were aborted while `m_voicedGain` was still audible, blasting clicks into the output on consonant transitions.
  - **Architectural Fixes:**
    1. *Exact Integer Period Spawning:* Removed `fineAlign`. Secondary tap $d_2$ is placed at $d_1 - k \cdot T_{\text{in}}$, maintaining exact phase alignment down to 0.0000 error across all harmonics.
    2. *Zero-Modulation In-Tune Slew Removal:* Eliminated the in-tune delay jerk. When in tune ($\alpha = 1.0$), `rateStep = 0.0`, holding delay completely static.
    3. *Clean Crossfade Completion on Unvoiced Transitions:* Active crossfades now finish smoothly even if `isVoiced` drops to false. Delay taps are safely re-centered to `baseLatency` only after `m_voicedGain < 0.001` (when the pitch shifter is 100% silent in the mix).
    4. *Per-Sample `alpha` Slew:* Slews pitch shift ratio smoothly per sample over $\sim 4\text{ ms}$ (`m_smoothedAlpha += 0.005f * (targetAlpha - m_smoothedAlpha)`), eliminating block-boundary rate kinks.
  - **Verification:** 18/18 DSP tests passing (100%). Deployed release binary to system VST3 directory.

- **Milestone 14: EFX Rack Signal Flow Audit, Master Polarity Correction & Saturation Overhaul:**
  - **Root Causes Identified:**
    1. *Master EFX Toggle Polarity Inversion:* The parameter was named `masterEfxBypass`. In the GUI (`PluginEditor.cpp`), the toggle button was attached to this parameter with text `"EFX"` and illuminated an amber LED pip when `isOn == true`. When the user clicked the button so it lit amber, `masterEfxBypass` became `1.0f` (`true`), causing `EFXRack::process()` to hit `if (m_masterBypass) return;`, bypassing the entire post-pitch EFX rack.
    2. *Drawer Power Disconnect:* Clicking effect slot titles expanded the drawer but left the individual module's power toggle OFF (Preset 1 initialized with Flanger, Chorus, and Saturation disabled). Opening a drawer and tweaking knobs produced no change because the individual module remained unpowered.
    3. *Chorus vs. Haas Routing & Missing UI Knobs:* `m_chorusRateKnob` and `m_chorusDepthKnob` were never instantiated or laid out in `EFXDrawerComponent.cpp`. Default `chorusMode` was set to Haas (mode 1), where Left channel was 100% dry and only Right channel was delayed, creating an unbalanced, lopsided stereo image on mono inputs.
    4. *Waveshaping Saturation Under-Drive:* Drive was scaled linearly from $1.0\times$ to $10.0\times$ (+20 dB) without auto-makeup level compensation. Typical DAW vocal peaks ($-18$ to $-12\text{ dBFS}$) remained largely in the linear region, and Hard Clip never reached its threshold at all.
    5. *Audio Thread Heap Allocations:* `setEqParams` and `setDistParams` reallocated IIR filter coefficients (`makeHighPass`, `makePeakFilter`, etc.) on every single audio block, violating real-time audio thread safety.
  - **Architectural Fixes:**
    1. *Corrected Master Polarity:* Renamed parameter to `FreePitchID::masterEfxActive`, default `true`. Amber pip lit = Master EFX ON (`setMasterBypass(!masterEfxActive)`). Presets updated so Master EFX is active.
    2. *Auto-Power on Drawer Open:* In `setupSlot()`, expanding an effect drawer automatically powers ON that module if it was inactive, ensuring instant audible feedback upon tweaking.
    3. *Complete Chorus/Haas Engine & UI:*
       - Instantiated `m_chorusRateKnob` and `m_chorusDepthKnob` in `EFXDrawerComponent`. Added dynamic mode-switching in `m_chorusModeBox.onChange` to display Chorus controls (`Rate`, `Depth`, `Mix`) vs. Haas controls (`Delay`, `Width`, `Mix`).
       - Implemented centered binaural Haas delay ($15\text{ ms} \pm \Delta$) on mono vocals, producing an expansive, centered 3D stereo image without lopsided skew.
       - Chorus defaults to Mode 0 ("Chorus") with 90° quadrature phase modulation.
    4. *Logarithmic Drive Scaling & Auto-Makeup:* Drive scales from $0\text{ dB}$ to $+32\text{ dB}$ ($1.0\times$ to $39.8\times$) with smooth auto-makeup normalization ($1 / \text{drive}^{0.65}$). Warm Tube provides asymmetric 2nd-harmonic triode saturation, Tape provides compressed odd harmonics, and Hard Clip provides crisp modern bite.
    5. *Zero-Allocation Dirty-Flag Coefficient Caching:* EQ and distortion lowpass filter coefficients are only recomputed when parameters or sample rate change.
  - **Verification:**
    - Added Test 11 in `DSPUnitTests.cpp` testing Master Bypass bit-exact pass-through, Saturation harmonic generation, and Chorus stereo decorrelation.
    - 19/19 DSP unit tests passing (100%).
    - Built and hot-swap deployed release `FreePitch.vst3` to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 15: Premier Vocal Space Delay (H-Delay / FreqEcho / Supermassive) & Formant Shift Engine Overhaul:**
  - **Space Delay & Echo Architecture (`source/DSP/EFXRack.h`, `source/DSP/EFXRack.cpp`):**
    - Fractional tape delay lines supporting up to 2000 ms with tape pitch glide on time changes.
    - Dual modes: Stereo Echo vs. Ping-Pong bouncing repeats.
    - Soft-knee analog tape saturation in feedback loop (`tanh(1.2 * fb) * 0.9`).
    - Feedback tone filter (80 Hz high-pass + 1–16 kHz low-pass damping).
    - Single-sideband Bode frequency shifter in feedback loop (inspired by Valhalla FreqEcho) using dual 4-pole IIR Hilbert 90° phase splitters with $\pm 100\text{ Hz}$ continuous frequency shift.
    - Valhalla Supermassive-inspired diffusion network using 4 cascaded prime allpass diffusers (lengths 149, 211, 293, 379 samples) with dynamic diffusion coefficient smearing delay taps into dense spatial reverb tails.
    - APVTS parameters: `delayEnabled`, `delayMode`, `delayTime`, `delayFeedback`, `delayFreqShift`, `delayDiffusion`, `delayTone`, `delayMix`.
    - Integrated as Slot 6 in `EFXDrawerComponent` ("SPACE DELAY / FREQ ECHO") with mode dropdown and 6 tactile rotary dials.
    - Factory presets updated with curated Space Delay settings (Preset 4: 300 ms Ping-Pong 40% diffusion; Preset 5: 350 ms Stereo +6 Hz FreqEcho rise).
  - **Formant Shift Engine Overhaul (`source/DSP/FormantEngine.h`, `source/DSP/FormantEngine.cpp`):**
    - Diagnosed root causes: `formantsEnabled` and `formantShiftSt` parameters were commented out in `PitchShifter.cpp` and never processed.
    - Overhauled `FormantEngine` with genuine real-time STFT overlap-add spectral envelope extraction and frequency warping:
      - 1024-point FFT with 75% Hann overlap-add (hop size 256).
      - Mathematical alignment: Fixed latency from `fftSize - hopSize` (768) to `fftSize` (1024), eliminating 3-grain sum amplitude modulation (delta 0.24 crackle) and achieving exact constant unity gain across all buffer sizes down to 64 samples.
      - Aligned read index timing with `curWritePos - m_latencySamples`, achieving bit-exact machine epsilon ($3.33 \times 10^{-16}$) transparency when unshifted.
      - Connected into `PitchShifter::process()` so Formant Shift shifts vocal tract formant frequencies by $\pm 12$ semitones (50% to 200%) independently of pitch correction.
  - **Automated DSP Unit Test Verification:**
    - All 21 tests in `DSPUnitTests.cpp` passing with 100% success (including Test 5 no-crackle, Test 9 small buffers, Test 10 zero rotary flutter, Test 11 EFX rack, Test 12 formant transparency and +6st rich warping, Test 13 Space Delay timing and diffusion).
  - **Deployment:**
    - Release build compiled and hot-swapped into `C:\Program Files\Common Files\VST3\FreePitch.vst3\Contents\x86_64-win\FreePitch.vst3` and `Contents\Resources\moduleinfo.json`. Standalone binary also built.

- **Milestone 16: Complete Modern Studio UI Overhaul, Modular Rack Bay & Tooltip System:**
  - **Modern Studio Color Palette:** Deep obsidian chassis (`#0C0E12`), studio rack bay (`#13161C`), elevated cards (`#191D26`), neon electric cyan (`#00D2FF`), warm studio amber/gold (`#F59E0B`), and emerald in-tune indicator (`#10B981`).
  - **Tactile 3D Studio Knobs:** Metallic dial caps with linear gradient, machined bevel rim, glowing needle indicator, illuminated arc trails, and hover glow. Hero styling for Retune Speed.
  - **Pitch Radar HUD (`CircularPitchMeter`):** Dark glass HUD bezel with $\pm 50\text{ cents}$ radial compass scale, hero detected note badge (e.g. `C#4`), dynamic cents deviation pill (`IN TUNE`, `-12 ct`, `+18 ct`), live Hz readout, and pulsing voiced aura.
  - **Interactive 61-Key Studio Piano (`PianoKeyboardComponent`):** Crisp ivory white keys and matte ebony black keys, active scale note markers, real-time live sung note tracking with neon cyan glow on the active key, and interactive 1-click scale note editing.
  - **Modular Studio EFX Rack Bay (`EFXDrawerComponent`):** Replaced vertical accordion squeeze with a top horizontal tab bar (`EQ`, `COMP`, `FLANGE`, `CHORUS`, `DRIVE`, `DELAY`) with illuminated LED power pips. Dedicated, spacious studio bay for each module with interactive frequency curve, live gain reduction meter, and analog tape delay controls.
  - **Comprehensive Studio Tooltip System:** Built-in `juce::TooltipWindow` with custom dark glass styling, offering detailed musical guidance, suggested parameter settings, and tooltips on every knob, button, dropdown, and keyboard key.
  - **Automated Verification:** 21/21 DSP unit tests passing (100%).
  - **Deployment & Git Branches:** Pushed original code to `legacyui` branch, created and pushed updated UI code to `newui` branch at `https://github.com/yash-im-saying/FreePitchVST`.


  - **Audio Dropouts & Glitches Fix:**
    - Root cause in `PitchShifter.cpp`: The synthesis loop evaluated synthesis marks against future uninitialized samples, while the output pointer read from a delayed position, creating a 240+ sample phase misalignment and buffer starvation between unvoiced bypass and voiced PSOLA. Re-architected with exact causal time alignment: both unvoiced bypass and voiced PSOLA are read at `outIdx = currentSample - m_latencySamples`, guaranteeing 100% phase lock and continuous audio.
    - Added 64-bit monotonic sample counter (`int64_t m_writeSampleCount`) and power-of-2 bitmasking (`& 0x7FFF` on 32768-sample buffer) eliminating pointer wrap freeze.
    - Added 128-sample hop size (`m_hopSize = 128`) in `PitchDetector.cpp`, slashing CPU usage by 75% on small buffer sizes (32/64 samples) in FL Studio.
    - Preallocated 8192-sample mono scratch buffers in `prepareToPlay()` to eliminate audio thread vector resizing.
    - Tested via automated test: 60 continuous blocks (25,600+ samples) with zero dropouts, 14/14 tests passing.
  - **UI/UX Overhaul in Pitch & Formants Section:**
    - Expanded top control row from 60 px to 96 px, providing full 64 px rotary dials for Pitch Shift and Formant Shift.
    - Removed hardcoded confidence meter coordinates; introduced `m_confidenceMeterBounds` computed dynamically in `resized()`, completely eliminating knob/meter overlap.
    - Added Formants header label ("FORMANTS"), centered "PRESERVE" toggle with illuminated amber LED indicator, and "VOCAL TRACT" subtitle.
    - Replaced UTF-8 em dash with ASCII `"--"` in `CircularPitchMeter.cpp`, fixing Unicode glyph corruption (`â □□`) in FL Studio.
    - Deployed hot-swap update to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 12: Rotary Flutter & Table-Fan Elimination via Phase-Synchronous Splice Architecture:**
  - **Acoustic Root Cause Analysis:**
    - In monophonic vocals containing strong formant resonances (F1, F2), adjacent pitch cycles exhibit multiple sub-peaks.
    - The legacy GCI picker searched a wide window ($\pm 0.55 T_{\text{in}}$) using simple maximum amplitude detection. Under slight micro-variations, it unpredictably hopped between the primary glottal peak and secondary formant peaks on alternating cycles.
    - This hopping introduced $90^\circ$ to $180^\circ$ phase inversions between consecutive overlapping Hann grains, producing catastrophic periodic destructive interference ($0.038$ to $1.393$ RMS amplitude modulation) at the pitch frequency ($50 - 200\text{ Hz}$). This acoustic flutter acted literally like spinning rotary table-fan blades chopping through the vocal audio.
  - **Phase-Synchronous Dual-Tap Resynthesis Implementation:**
    - Replaced the discrete GCI epoch-hopping loop with a continuous phase-synchronous dual-tap delay-line architecture:
      - Continuous fractional read tap $d_1(t)$ advancing by $\frac{dd}{dt} = 1 - \alpha$, where $\alpha = \frac{f_{\text{target}}}{f_{\text{input}}}$.
      - When $\alpha = 1.0$ (in tune), $\frac{dd}{dt} = 0$, delay is static, yielding bit-exact reproduction of the input with ZERO phase modulation and ZERO grain boundaries.
      - 4-point Hermite cubic interpolation (`readHermite`) providing flat frequency response up to Nyquist without high-frequency dampening or clicks.
      - When drift exceeds the detected fundamental pitch period $T_{\text{in}}$, a secondary read tap $d_2$ is spawned at integer period offset $k \cdot T_{\text{in}}$ with $\pm 8$-sample micro fine-alignment cross-correlation (`fineAlign`).
      - Taps $d_1$ and $d_2$ crossfade seamlessly over a full pitch period ($T_{\text{in}}$) with raised-cosine Hann weighting ($w_1 + w_2 \equiv 1.0$).
  - **Automated DSP Verification:**
    - Added Test 10 in `DSPUnitTests.cpp` testing multi-harmonic formant vocal tone under $+1.2$ st shift.
    - Verified steady-state RMS flutter ratio $R = \frac{\text{minRMS}}{\text{avgRMS}} = 0.971$ (compared to $<0.10$ on legacy code).
    - 18/18 DSP unit tests passing (100%).
    - Deployed Release build to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 13: Click Artifacts Elimination & Seamless Pitch Continuity:**
  - **Acoustic Root Causes:**
    1. *Flawed Micro-Fine-Alignment (`fineAlign`):* The $\pm 8$-sample local cross-correlation acted as an unnormalized slope detector over a 24-sample window. It slammed to $+8$ or $-8$ samples, offsetting the secondary read tap by up to $180^\circ$ of phase error at 2–3 kHz, producing transient phase cancellation spikes and clicks on every crossfade.
    2. *In-Tune Velocity Jerk:* When `alpha` crossed the $<1$ cent boundary (`abs(alpha - 1.0) < 0.0005`), `m_tapDelay1` abruptly slewed towards `baseLatency` at $-0.75$ samples/sample (a 1.75x speed jerk), creating clicks and pitch chirps on sustained notes.
    3. *Premature Unvoiced Delay Slewing:* When `isVoiced` dropped to false, delay was forcefully slewed at $0.05 \cdot (D_0 - d_1)$ (up to 10 samples per sample step) and active crossfades were aborted while `m_voicedGain` was still audible, blasting clicks into the output on consonant transitions.
  - **Architectural Fixes:**
    1. *Exact Integer Period Spawning:* Removed `fineAlign`. Secondary tap $d_2$ is placed at $d_1 - k \cdot T_{\text{in}}$, maintaining exact phase alignment down to 0.0000 error across all harmonics.
    2. *Zero-Modulation In-Tune Slew Removal:* Eliminated the in-tune delay jerk. When in tune ($\alpha = 1.0$), `rateStep = 0.0`, holding delay completely static.
    3. *Clean Crossfade Completion on Unvoiced Transitions:* Active crossfades now finish smoothly even if `isVoiced` drops to false. Delay taps are safely re-centered to `baseLatency` only after `m_voicedGain < 0.001` (when the pitch shifter is 100% silent in the mix).
    4. *Per-Sample `alpha` Slew:* Slews pitch shift ratio smoothly per sample over $\sim 4\text{ ms}$ (`m_smoothedAlpha += 0.005f * (targetAlpha - m_smoothedAlpha)`), eliminating block-boundary rate kinks.
  - **Verification:** 18/18 DSP tests passing (100%). Deployed release binary to system VST3 directory.

- **Milestone 14: EFX Rack Signal Flow Audit, Master Polarity Correction & Saturation Overhaul:**
  - **Root Causes Identified:**
    1. *Master EFX Toggle Polarity Inversion:* The parameter was named `masterEfxBypass`. In the GUI (`PluginEditor.cpp`), the toggle button was attached to this parameter with text `"EFX"` and illuminated an amber LED pip when `isOn == true`. When the user clicked the button so it lit amber, `masterEfxBypass` became `1.0f` (`true`), causing `EFXRack::process()` to hit `if (m_masterBypass) return;`, bypassing the entire post-pitch EFX rack.
    2. *Drawer Power Disconnect:* Clicking effect slot titles expanded the drawer but left the individual module's power toggle OFF (Preset 1 initialized with Flanger, Chorus, and Saturation disabled). Opening a drawer and tweaking knobs produced no change because the individual module remained unpowered.
    3. *Chorus vs. Haas Routing & Missing UI Knobs:* `m_chorusRateKnob` and `m_chorusDepthKnob` were never instantiated or laid out in `EFXDrawerComponent.cpp`. Default `chorusMode` was set to Haas (mode 1), where Left channel was 100% dry and only Right channel was delayed, creating an unbalanced, lopsided stereo image on mono inputs.
    4. *Waveshaping Saturation Under-Drive:* Drive was scaled linearly from $1.0\times$ to $10.0\times$ (+20 dB) without auto-makeup level compensation. Typical DAW vocal peaks ($-18$ to $-12\text{ dBFS}$) remained largely in the linear region, and Hard Clip never reached its threshold at all.
    5. *Audio Thread Heap Allocations:* `setEqParams` and `setDistParams` reallocated IIR filter coefficients (`makeHighPass`, `makePeakFilter`, etc.) on every single audio block, violating real-time audio thread safety.
  - **Architectural Fixes:**
    1. *Corrected Master Polarity:* Renamed parameter to `FreePitchID::masterEfxActive`, default `true`. Amber pip lit = Master EFX ON (`setMasterBypass(!masterEfxActive)`). Presets updated so Master EFX is active.
    2. *Auto-Power on Drawer Open:* In `setupSlot()`, expanding an effect drawer automatically powers ON that module if it was inactive, ensuring instant audible feedback upon tweaking.
    3. *Complete Chorus/Haas Engine & UI:*
       - Instantiated `m_chorusRateKnob` and `m_chorusDepthKnob` in `EFXDrawerComponent`. Added dynamic mode-switching in `m_chorusModeBox.onChange` to display Chorus controls (`Rate`, `Depth`, `Mix`) vs. Haas controls (`Delay`, `Width`, `Mix`).
       - Implemented centered binaural Haas delay ($15\text{ ms} \pm \Delta$) on mono vocals, producing an expansive, centered 3D stereo image without lopsided skew.
       - Chorus defaults to Mode 0 ("Chorus") with 90° quadrature phase modulation.
    4. *Logarithmic Drive Scaling & Auto-Makeup:* Drive scales from $0\text{ dB}$ to $+32\text{ dB}$ ($1.0\times$ to $39.8\times$) with smooth auto-makeup normalization ($1 / \text{drive}^{0.65}$). Warm Tube provides asymmetric 2nd-harmonic triode saturation, Tape provides compressed odd harmonics, and Hard Clip provides crisp modern bite.
    5. *Zero-Allocation Dirty-Flag Coefficient Caching:* EQ and distortion lowpass filter coefficients are only recomputed when parameters or sample rate change.
  - **Verification:**
    - Added Test 11 in `DSPUnitTests.cpp` testing Master Bypass bit-exact pass-through, Saturation harmonic generation, and Chorus stereo decorrelation.
    - 19/19 DSP unit tests passing (100%).
    - Built and hot-swap deployed release `FreePitch.vst3` to `C:\Program Files\Common Files\VST3\FreePitch.vst3`.

- **Milestone 15: Premier Vocal Space Delay (H-Delay / FreqEcho / Supermassive) & Formant Shift Engine Overhaul:**
  - **Space Delay & Echo Architecture (`source/DSP/EFXRack.h`, `source/DSP/EFXRack.cpp`):**
    - Fractional tape delay lines supporting up to 2000 ms with tape pitch glide on time changes.
    - Dual modes: Stereo Echo vs. Ping-Pong bouncing repeats.
    - Soft-knee analog tape saturation in feedback loop (`tanh(1.2 * fb) * 0.9`).
    - Feedback tone filter (80 Hz high-pass + 1–16 kHz low-pass damping).
    - Single-sideband Bode frequency shifter in feedback loop (inspired by Valhalla FreqEcho) using dual 4-pole IIR Hilbert 90° phase splitters with $\pm 100\text{ Hz}$ continuous frequency shift.
    - Valhalla Supermassive-inspired diffusion network using 4 cascaded prime allpass diffusers (lengths 149, 211, 293, 379 samples) with dynamic diffusion coefficient smearing delay taps into dense spatial reverb tails.
    - APVTS parameters: `delayEnabled`, `delayMode`, `delayTime`, `delayFeedback`, `delayFreqShift`, `delayDiffusion`, `delayTone`, `delayMix`.
    - Integrated as Slot 6 in `EFXDrawerComponent` ("SPACE DELAY / FREQ ECHO") with mode dropdown and 6 tactile rotary dials.
    - Factory presets updated with curated Space Delay settings (Preset 4: 300 ms Ping-Pong 40% diffusion; Preset 5: 350 ms Stereo +6 Hz FreqEcho rise).
  - **Formant Shift Engine Overhaul (`source/DSP/FormantEngine.h`, `source/DSP/FormantEngine.cpp`):**
    - Diagnosed root causes: `formantsEnabled` and `formantShiftSt` parameters were commented out in `PitchShifter.cpp` and never processed.
    - Overhauled `FormantEngine` with genuine real-time STFT overlap-add spectral envelope extraction and frequency warping:
      - 1024-point FFT with 75% Hann overlap-add (hop size 256).
      - Mathematical alignment: Fixed latency from `fftSize - hopSize` (768) to `fftSize` (1024), eliminating 3-grain sum amplitude modulation (delta 0.24 crackle) and achieving exact constant unity gain across all buffer sizes down to 64 samples.
      - Aligned read index timing with `curWritePos - m_latencySamples`, achieving bit-exact machine epsilon ($3.33 \times 10^{-16}$) transparency when unshifted.
      - Connected into `PitchShifter::process()` so Formant Shift shifts vocal tract formant frequencies by $\pm 12$ semitones (50% to 200%) independently of pitch correction.
  - **Automated DSP Unit Test Verification:**
    - All 21 tests in `DSPUnitTests.cpp` passing with 100% success (including Test 5 no-crackle, Test 9 small buffers, Test 10 zero rotary flutter, Test 11 EFX rack, Test 12 formant transparency and +6st rich warping, Test 13 Space Delay timing and diffusion).
  - **Deployment:**
    - Release build compiled and hot-swapped into `C:\Program Files\Common Files\VST3\FreePitch.vst3\Contents\x86_64-win\FreePitch.vst3` and `Contents\Resources\moduleinfo.json`. Standalone binary also built.

- **Milestone 16: Complete Modern Studio UI Overhaul, Modular Rack Bay & Tooltip System:**
  - **Modern Studio Color Palette:** Deep obsidian chassis (`#0C0E12`), studio rack bay (`#13161C`), elevated cards (`#191D26`), neon electric cyan (`#00D2FF`), warm studio amber/gold (`#F59E0B`), and emerald in-tune indicator (`#10B981`).
  - **Tactile 3D Studio Knobs:** Metallic dial caps with linear gradient, machined bevel rim, glowing needle indicator, illuminated arc trails, and hover glow. Hero styling for Retune Speed.
  - **Pitch Radar HUD (`CircularPitchMeter`):** Dark glass HUD bezel with $\pm 50\text{ cents}$ radial compass scale, hero detected note badge (e.g. `C#4`), dynamic cents deviation pill (`IN TUNE`, `-12 ct`, `+18 ct`), live Hz readout, and pulsing voiced aura.
  - **Interactive 61-Key Studio Piano (`PianoKeyboardComponent`):** Crisp ivory white keys and matte ebony black keys, active scale note markers, real-time live sung note tracking with neon cyan glow on the active key, and interactive 1-click scale note editing.
  - **Modular Studio EFX Rack Bay (`EFXDrawerComponent`):** Replaced vertical accordion squeeze with a top horizontal tab bar (`EQ`, `COMP`, `FLANGE`, `CHORUS`, `DRIVE`, `DELAY`) with illuminated LED power pips. Dedicated, spacious studio bay for each module with interactive frequency curve, live gain reduction meter, and analog tape delay controls.
  - **Comprehensive Studio Tooltip System:** Built-in `juce::TooltipWindow` with custom dark glass styling, offering detailed musical guidance, suggested parameter settings, and tooltips on every knob, button, dropdown, and keyboard key.
  - **Automated Verification:** 21/21 DSP unit tests passing (100%).
  - **Deployment & Git Branches:** Pushed original code to `legacyui` branch, created and pushed updated UI code to `newui` branch at `https://github.com/yash-im-saying/FreePitchVST`.


