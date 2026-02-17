#pragma once
#include <juce_core/juce_core.h>

inline juce::String midiNoteNumberToName(int noteNumber)
{
    noteNumber = juce::jlimit(0, 127, noteNumber);

    static const char* noteNames[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const int pitchClass = noteNumber % 12;
    const int octave = (noteNumber / 12) - 1;

    return juce::String(noteNames[pitchClass]) + juce::String(octave);
}
