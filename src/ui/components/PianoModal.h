#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class PianoModal : public juce::Component,
    private juce::MidiKeyboardStateListener
{
public:
    PianoModal(int initialNote);
    ~PianoModal() override;

    std::function<void(int)> onNoteSelected;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // ===== MIDI Keyboard =====
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard;

    // ===== Octave Controls =====
    juce::TextButton octaveMinus{ "-" };
    juce::TextButton octavePlus{ "+" };

    int currentNote = 60;
    int currentOctave = 0;

    void updateOctaveView();

    // ===== MidiKeyboardStateListener =====
    void handleNoteOn(juce::MidiKeyboardState*,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;

    void handleNoteOff(juce::MidiKeyboardState*,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;
};
