#include <iostream>
#include <vector>
#include <cmath>
#include "DSP/PitchDetector.h"

int main() {
    PitchDetector detector;
    detector.prepare(48000.0, 2048);
    detector.setVocalMode(VocalMode::Tenor);

    // 440 Hz sine wave
    std::vector<float> wave(2048);
    double inc = 2.0 * 3.14159265358979323846 * 440.0 / 48000.0;
    for (int i = 0; i < 2048; ++i) wave[i] = 0.75f * std::sin(inc * i);

    detector.pushSamples(wave.data(), 2048);
    float hz = 0, conf = 0, rms = 0;
    bool v = detector.processPitch(hz, conf, rms);
    std::cout << "v=" << v << " hz=" << hz << " conf=" << conf << " rms=" << rms << std::endl;
    return 0;
}
