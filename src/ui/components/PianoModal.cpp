#include "PianoModal.h"

PianoModal::PianoModal(int initialNote)
    : keyboard(keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard)
{
    currentNote = juce::jlimit(0, 127, initialNote);

    // ---- Keyboard Setup ----
    keyboard.setKeyWidth(20.0f);
    keyboard.setScrollButtonsVisible(false);

    addAndMakeVisible(keyboard);

    // ---- Octave Slider ----
    octaveSlider.setRange(-1, 8, 1); // MIDI supports C-1 to G9
    octaveSlider.setValue((currentNote / 12) - 1);

    octaveSlider.setSliderStyle(juce::Slider::IncDecButtons);
    octaveSlider.setTextBoxStyle(juce::Slider::TextBoxRight,
        false, 50, 20);

    octaveSlider.onValueChange = [this]
        {
            const int octave = (int)octaveSlider.getValue();
            const int baseNote = (octave + 1) * 12;

            keyboard.setAvailableRange(
                juce::jlimit(0, 127, baseNote),
                juce::jlimit(0, 127, baseNote + 11));
        };

    addAndMakeVisible(octaveSlider);

    // ---- Set Initial Visible Octave ----
    {
        const int octave = (currentNote / 12) - 1;
        const int baseNote = (octave + 1) * 12;

        keyboard.setAvailableRange(
            juce::jlimit(0, 127, baseNote),
            juce::jlimit(0, 127, baseNote + 11));
    }

    // ---- Keyboard Listener ----
    keyboardState.addListener(this);

    setSize(400, 140);
}

PianoModal::~PianoModal()
{
    keyboardState.removeListener(this);
}

void PianoModal::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.96f));

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void PianoModal::resized()
{
    auto area = getLocalBounds().reduced(8);

    octaveSlider.setBounds(area.removeFromTop(30));
    area.removeFromTop(8);

    keyboard.setBounds(area);
}

void PianoModal::handleNoteOn(juce::MidiKeyboardState*,
    int /*midiChannel*/,
    int midiNoteNumber,
    float /*velocity*/)
{
    currentNote = midiNoteNumber;

    if (onNoteSelected)
        onNoteSelected(currentNote);

    // Close CallOutBox if shown inside one
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
    // Not needed
}
