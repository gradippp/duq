#include "MidiUtils.h"
#include "../../utils/ConfigManager.h"

juce::String midiNoteNumberToName(int noteNumber)
{
    noteNumber = juce::jlimit(0, 127, noteNumber);

    static const char* noteNames[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const int pitchClass = noteNumber % 12;
    
    juce::SharedResourcePointer<ConfigManager> config;
    const int octave = (noteNumber / 12) - 1 + config->getMidiOctaveOffset();

    return juce::String(noteNames[pitchClass]) + juce::String(octave);
}
