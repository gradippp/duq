#include "EnvelopeRowComponent.h"
#include "../utils/IconFactory.h"
#include "../utils/MidiUtils.h"
#include "../utils/PresetManager.h"
#include "../utils/FontManager.h"
#include "../components/PianoModal.h"
#include "../../PluginProcessor.h"

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
    nameLabel.setFont(FontManager::getInterRegular(13.0f));

    nameLabel.onSingleClick = [this]
        {
            if (onSelected)
                onSelected();
        };

    nameLabel.onRightClick = [this]
        {
            showContextMenu();
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
    noteButton.getLookAndFeel().setDefaultSansSerifTypeface(FontManager::getJetBrainsMono(12.0f).getTypefacePtr());

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
            auto initialFile = PresetManager::getEnvelopeDirectory()
                .getChildFile(envelope["name"].toString());

            auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

            auto chooser = std::make_shared<juce::FileChooser>("Save Envelope Preset",
                initialFile,
                "*" + PresetManager::envelopeExtension);

            chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();
                    if (file == juce::File())
                        return;

                    PresetManager::saveEnvelope(envelope, file);
                });
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

    bool isDisabled = (bool)envelope.getProperty("disabled", false);

    nameLabel.setColour(juce::Label::textColourId,
        isDisabled ? juce::Colours::grey : juce::Colours::white);

    float alpha = isDisabled ? 0.4f : 1.0f;
    noteButton.setAlpha(alpha);
    saveButton.setAlpha(alpha);
    replaceButton.setAlpha(alpha);
    deleteButton.setAlpha(alpha);

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
    if (e.mods.isRightButtonDown())
    {
        showContextMenu();
    }
    else if (e.mods.isLeftButtonDown())
    {
        if (onSelected)
            onSelected();
    }
}

void EnvelopeRowComponent::showContextMenu()
{
    bool isDisabled = (bool)envelope.getProperty("disabled", false);

    juce::PopupMenu m;
    m.addItem(1, "Rename");
    m.addItem(2, isDisabled ? "Enable" : "Disable");

    m.showMenuAsync(juce::PopupMenu::Options(), [this, isDisabled](int result)
    {
        if (result == 1)
        {
            nameLabel.showEditor();
        }
        else if (result == 2)
        {
            if (undoManager != nullptr)
                undoManager->beginNewTransaction(isDisabled ? "Enable Envelope" : "Disable Envelope");

            envelope.setProperty("disabled", !isDisabled, undoManager);
            repaint();
        }
    });
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
    bool isDisabled = (bool)envelope.getProperty("disabled", false);

    // ===============================
    // Background
    // ===============================

    juce::Colour bgColour = juce::Colours::darkgrey.withAlpha(0.2f);

    if (isSelected)
        bgColour = juce::Colours::darkgrey.withAlpha(0.5f);  // lighter when selected
    else if (isHovered)
        bgColour = juce::Colours::darkgrey.withAlpha(0.35f);

    if (isDisabled)
        bgColour = bgColour.withAlpha(0.1f);

    g.setColour(bgColour);
    g.fillRect(bounds);

    // ===============================
    // Envelope name
    // ===============================

    g.setColour(isDisabled ? juce::Colours::grey : juce::Colours::white);
    g.setFont(FontManager::getInterRegular(13.0f));

    auto nameArea = bounds;
    nameArea.removeFromLeft(noteButton.getRight());
    nameArea.removeFromRight(90);

    // (Text is drawn by Label component, but we set its color in refreshFromTree)

    // ===============================
    // MIDI Trigger Indicator (green dot)
    // ===============================

    if (isActive && !isDisabled)   // <-- ONLY for MIDI trigger now
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

    // Overlay for disabled state
    if (isDisabled)
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRect(bounds);
    }
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
