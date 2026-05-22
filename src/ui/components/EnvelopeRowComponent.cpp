#include "EnvelopeRowComponent.h"
#include "../utils/IconFactory.h"
#include "../utils/MidiUtils.h"
#include "../../utils/PresetManager.h"
#include "../utils/FontManager.h"
#include "../components/PianoModal.h"
#include "../../PluginProcessor.h"
#include "../../Globals.h"

EnvelopeRowComponent::EnvelopeRowComponent(juce::ValueTree envelopeTree)
    : envelope(envelopeTree)
{
    envelope.addListener(this);
    ThemeManager::getInstance().addChangeListener(this);

    // ===============================
    // Name label
    // ===============================

    nameLabel.setEditable(false, true, false);
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

    saveButton.onClick = [this]()
        {
            if (onSaveRequested)
                onSaveRequested();
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

    refreshTheme();
    refreshFromTree();
}

EnvelopeRowComponent::~EnvelopeRowComponent()
{
    ThemeManager::getInstance().removeChangeListener(this);
    envelope.removeListener(this);
}

void EnvelopeRowComponent::refreshTheme()
{
    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                T_COL(uiHover));

            auto normal = Icons::load(iconName, T_COL(textMain));
            auto over = Icons::load(iconName, T_COL(textMain).withAlpha(0.85f));
            auto down = Icons::load(iconName, T_COL(textMain).withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);
        };

    setupIconButton(saveButton, "save");
    setupIconButton(replaceButton, "replace");
    setupIconButton(deleteButton, "delete");

    noteButton.setColour(juce::TextButton::buttonColourId, T_COL(uiHover));
    noteButton.setColour(juce::TextButton::textColourOffId, T_COL(textMain));
    noteButton.setColour(juce::TextButton::buttonOnColourId, T_COL(uiSelected));

    bool isDisabled = (bool)envelope.getProperty("disabled", false);
    nameLabel.setColour(juce::Label::textColourId, isDisabled ? T_COL(textDimmed) : T_COL(textMain));

    repaint();
}

void EnvelopeRowComponent::changeListenerCallback([[maybe_unused]] juce::ChangeBroadcaster* source)
{
    refreshTheme();
}

void EnvelopeRowComponent::refreshFromTree()
{
    nameLabel.setText(envelope["name"].toString(),
        juce::dontSendNotification);

    int note = (int)envelope["triggerNote"];
    noteButton.setButtonText(midiNoteNumberToName(note));

    bool isDisabled = (bool)envelope.getProperty("disabled", false);

    nameLabel.setColour(juce::Label::textColourId,
        isDisabled ? T_COL(textDimmed) : T_COL(textMain));

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
    mouseDownPos = e.getPosition();

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

void EnvelopeRowComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (e.mouseWasDraggedSinceMouseDown())
    {
        auto distance = e.getOffsetFromDragStart();
        if (std::abs(distance.x) > 5 || std::abs(distance.y) > 5)
        {
            if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
            {
                if (!container->isDragAndDropActive())
                {
                    juce::Image preview(juce::Image::ARGB, getWidth(), getHeight(), true);
                    juce::Graphics g(preview);
                    paint(g);

                    auto desc = juce::var(envelope["name"].toString());
                    container->startDragging(desc, this, preview, true, nullptr);
                }
            }
        }
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

    juce::Colour bgColour = T_COL(sectionBackground);

    if (isSelected)
        bgColour = T_COL(uiSelected);
    else if (isHovered)
        bgColour = T_COL(uiHover);

    if (isDisabled)
        bgColour = bgColour.withAlpha(0.1f);

    g.setColour(bgColour);
    g.fillRect(bounds);

    // ===============================
    // Drag Handle
    // ===============================
    {
        auto handleArea = bounds.removeFromLeft(12).reduced(4, 8);
        g.setColour(T_COL(textDimmed).withAlpha(0.3f));
        
        for (int i = 0; i < 3; ++i)
        {
            float y = static_cast<float>(handleArea.getY()) + static_cast<float>(i) * 4.0f;
            g.fillEllipse(static_cast<float>(handleArea.getX()), y, 2.0f, 2.0f);
            g.fillEllipse(static_cast<float>(handleArea.getX()) + 3.0f, y, 2.0f, 2.0f);
        }
    }

    // ===============================
    // Envelope name
    // ===============================

    g.setColour(isDisabled ? T_COL(textDimmed) : T_COL(textMain));
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

        g.setColour(T_COL(midiIndicator));
        g.fillEllipse(dotArea.toFloat());
    }

    // ===============================
    // Bottom separator
    // ===============================

    g.setColour(T_COL(border).withAlpha(0.3f));
    g.drawLine(0.0f,
        (float)getHeight() - 1.0f,
        (float)getWidth(),
        (float)getHeight() - 1.0f);

    // Overlay for disabled state
    if (isDisabled)
    {
        g.setColour(T_COL(uiDisabledOverlay).withAlpha(0.2f));
        g.fillRect(bounds);
    }
}

void EnvelopeRowComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    
    // Drag handle space
    bounds.removeFromLeft(14);

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
