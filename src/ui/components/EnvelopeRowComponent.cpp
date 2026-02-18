#include "EnvelopeRowComponent.h"
#include "../utils/IconFactory.h"
#include "../utils/MidiUtils.h"
#include "../components/PianoModal.h"

EnvelopeRowComponent::EnvelopeRowComponent(juce::ValueTree envelopeTree)
    : envelope(envelopeTree)
{
    envelope.addListener(this);

    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                juce::Colours::darkgrey.withAlpha(0.2f));

            auto normal = Icons::load(iconName, juce::Colours::white);
            auto over = Icons::load(iconName, juce::Colours::white.withAlpha(0.85f));
            auto down = Icons::load(iconName, juce::Colours::white.withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);

            button.setTooltip(iconName);
        };

    // ===============================
    // Name label
    // ===============================

    nameLabel.setEditable(false, true, false);
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    nameLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    nameLabel.setJustificationType(juce::Justification::centredLeft);

    nameLabel.onSingleClick = [this]
        {
            if (onSelected)
                onSelected();
        };

    nameLabel.onEditorHide = [this]
        {
            auto newName = nameLabel.getText().trim();

            if (newName.isEmpty())
                newName = "Envelope";

            if (onNameChanged)
                onNameChanged(newName);
        };

    // ===============================
    // Note button
    // ===============================

    noteButton.setClickingTogglesState(false);
    noteButton.setTooltip("Trigger MIDI note");
    noteButton.setColour(juce::TextButton::buttonColourId,
        juce::Colours::darkgrey.withAlpha(0.3f));
    noteButton.setColour(juce::TextButton::textColourOffId,
        juce::Colours::white);
    noteButton.setColour(juce::TextButton::buttonOnColourId,
        juce::Colours::darkgrey.withAlpha(0.5f));

    noteButton.onClick = [this]
        {
            int currentNote = (int)envelope["triggerNote"];

            auto modal = std::make_unique<PianoModal>(currentNote);

            modal->onNoteSelected = [this](int note)
                {
                    if (onNoteChanged)
                        onNoteChanged(note);
                };

            juce::CallOutBox::launchAsynchronously(
                std::move(modal),
                noteButton.getScreenBounds(),
                nullptr);
        };

    // ===============================
    // Icons
    // ===============================

    setupIconButton(saveButton, "save");
    setupIconButton(replaceButton, "replace");
    setupIconButton(deleteButton, "delete");

    saveButton.onClick = [this]()
        {
            // TODO: implement envelope save preset
        };

    replaceButton.onClick = [this]()
        {
            if (onReplaceRequested)
                onReplaceRequested();
        };

    deleteButton.onClick = [this]()
        {
            if (onDeleteRequested)
                onDeleteRequested();
        };

    addAndMakeVisible(saveButton);
    addAndMakeVisible(replaceButton);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(noteButton);
    addAndMakeVisible(nameLabel);

    refreshFromTree();
}

EnvelopeRowComponent::~EnvelopeRowComponent()
{
    envelope.removeListener(this);
}

void EnvelopeRowComponent::refreshFromTree()
{
    nameLabel.setText(envelope["name"].toString(),
        juce::dontSendNotification);

    int note = (int)envelope["triggerNote"];
    noteButton.setButtonText(midiNoteNumberToName(note));

    repaint();
}

void EnvelopeRowComponent::valueTreePropertyChanged(
    juce::ValueTree&,
    const juce::Identifier&)
{
    refreshFromTree();
}

void EnvelopeRowComponent::setSelected(bool shouldBeSelected)
{
    isSelected = shouldBeSelected;
    repaint();
}

void EnvelopeRowComponent::setActive(bool shouldBeActive)
{
    isActive = shouldBeActive;
    repaint();
}

void EnvelopeRowComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        if (onSelected)
            onSelected();
    }
}

void EnvelopeRowComponent::mouseEnter(const juce::MouseEvent&)
{
    isHovered = true;
    repaint();
}

void EnvelopeRowComponent::mouseExit(const juce::MouseEvent&)
{
    isHovered = false;
    repaint();
}

void EnvelopeRowComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // ===============================
    // Background
    // ===============================

    juce::Colour bgColour = juce::Colours::darkgrey.withAlpha(0.2f);

    if (isSelected)
        bgColour = juce::Colours::darkgrey.withAlpha(0.5f);  // lighter when selected
    else if (isHovered)
        bgColour = juce::Colours::darkgrey.withAlpha(0.35f);

    g.setColour(bgColour);
    g.fillRect(bounds);

    // ===============================
    // Envelope name
    // ===============================

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));

    auto nameArea = bounds;
    nameArea.removeFromLeft(noteButton.getRight());
    nameArea.removeFromRight(90);

    // ===============================
    // MIDI Trigger Indicator (green dot)
    // ===============================

    if (isActive)   // <-- ONLY for MIDI trigger now
    {
        const int dotSize = 6;

        auto dotArea = getLocalBounds()
            .removeFromRight(20)
            .withSizeKeepingCentre(dotSize, dotSize);

        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(dotArea.toFloat());
    }

    // ===============================
    // Bottom separator
    // ===============================

    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    g.drawLine(0.0f,
        (float)getHeight() - 1.0f,
        (float)getWidth(),
        (float)getHeight() - 1.0f);
}

void EnvelopeRowComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    auto buttonArea = bounds.removeFromRight(90);

    constexpr int buttonWidth = 24;
    saveButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    replaceButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));

    constexpr int noteButtonWidth = 50;
    noteButton.setBounds(bounds.removeFromLeft(noteButtonWidth).reduced(2));

    // Name label fills remaining area
    nameLabel.setBounds(bounds.reduced(6, 0));
}
