#include "PianoModal.h"

PianoModal::PianoModal(int initialNote)
    : keyboard(keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard)
{
    currentNote = juce::jlimit(0, 127, initialNote);

    // ===== Keyboard =====
    keyboard.setAvailableRange(0, 127);
    keyboard.setScrollButtonsVisible(false);
    addAndMakeVisible(keyboard);

    // ===== Octave =====
    currentOctave = (currentNote / 12) - 2;
    currentOctave = juce::jlimit(-1, 8, currentOctave);

    addAndMakeVisible(octavePlus);
    addAndMakeVisible(octaveMinus);

    octavePlus.onClick = [this]
        {
            currentOctave = juce::jlimit(-1, 8, currentOctave + 1);
            updateOctaveView();
        };

    octaveMinus.onClick = [this]
        {
            currentOctave = juce::jlimit(-1, 8, currentOctave - 1);
            updateOctaveView();
        };

    keyboardState.addListener(this);

    updateOctaveView();

    const float keyWidth = 16.0f;
    const int whiteKeysPerOctave = 7;
    const int buttonWidth = 26;
    const int padding = 6;

    int keyboardWidth = (int)(whiteKeysPerOctave * keyWidth);

    int totalWidth = padding * 2 + keyboardWidth + buttonWidth;
    int totalHeight = 80;

    setSize(totalWidth, totalHeight);
}

PianoModal::~PianoModal()
{
    keyboardState.removeListener(this);
}

void PianoModal::updateOctaveView()
{
    const float keyWidth = 16.0f;

    keyboard.setKeyWidth(keyWidth);

    int baseNote = (currentOctave + 2) * 12;
    baseNote = juce::jlimit(0, 116, baseNote);

    keyboard.setAvailableRange(baseNote, baseNote + 11);

    keyboard.repaint();
}

void PianoModal::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.96f));

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void PianoModal::resized()
{
    const int padding = 6;
    const int buttonWidth = 26;

    auto area = getLocalBounds().reduced(padding);

    // Right column for buttons
    auto buttonArea = area.removeFromRight(buttonWidth);

    octavePlus.setBounds(
        buttonArea.removeFromTop(buttonArea.getHeight() / 2).reduced(2));

    octaveMinus.setBounds(
        buttonArea.reduced(2));

    // Remaining area is exactly keyboard width
    keyboard.setBounds(area);
}

void PianoModal::handleNoteOn(juce::MidiKeyboardState*,
    int,
    int midiNoteNumber,
    float)
{
    currentNote = midiNoteNumber;

    if (onNoteSelected)
        onNoteSelected(currentNote);

    if (auto* callout =
        findParentComponentOfClass<juce::CallOutBox>())
    {
        callout->dismiss();
    }
}

void PianoModal::handleNoteOff(juce::MidiKeyboardState*,
    int,
    int,
    float)
{
}
