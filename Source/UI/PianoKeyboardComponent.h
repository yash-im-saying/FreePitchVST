#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "FreePitchLookAndFeel.h"
#include <functional>

class PianoKeyboardComponent : public juce::Component {
public:
    PianoKeyboardComponent();
    ~PianoKeyboardComponent() override = default;

    static constexpr int kStartMidi = 24; // C1
    static constexpr int kEndMidi   = 84; // C6
    static constexpr int kNumKeys   = kEndMidi - kStartMidi + 1; // 61 keys

    void setVocalMode(VocalMode mode);
    void setScale(int keyRoot, ScaleType scaleType, uint32_t customMask);
    void updateLiveNotes(int detectedMidi, int targetMidi, bool isVoiced);

    std::function<void(uint32_t)> onCustomMaskChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    VocalMode m_vocalMode = VocalMode::Tenor;
    int m_keyRoot = 0;
    ScaleType m_scaleType = ScaleType::Chromatic;
    uint32_t m_customMask = 0x0FFF;

    int m_detectedMidi = -1;
    int m_targetMidi = -1;
    bool m_isVoiced = false;

    // Helper functions
    static bool isBlackKey(int midiNote);
    int getMidiFromPosition(float x, float y) const;
    bool isNoteInScale(int midiNote) const;
};
