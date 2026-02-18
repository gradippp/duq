#include "HeaderSection.h"
#include "../utils/IconFactory.h"

HeaderSection::HeaderSection()
{
    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                juce::Colours::white.withAlpha(0.08f));

            auto normal = Icons::load(iconName, juce::Colours::white);
            auto over = Icons::load(iconName, juce::Colours::white.withAlpha(0.85f));
            auto down = Icons::load(iconName, juce::Colours::white.withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);
        };

    setupIconButton(undoButton, "undo");
    setupIconButton(redoButton, "redo");
    setupIconButton(saveProjectButton, "save");
    setupIconButton(initPresetButton, "close"); 
    
    undoButton.setTooltip("Undo");
    redoButton.setTooltip("Redo");
    saveProjectButton.setTooltip("Save Project Preset");
    initPresetButton.setTooltip("Init Preset (Reset State)");

    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    addAndMakeVisible(saveProjectButton);
    addAndMakeVisible(initPresetButton);

    addAndMakeVisible(presetNameLabel);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    presetNameLabel.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::bold));
    presetNameLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    presetNameLabel.setText(presetName.toUpperCase(), juce::dontSendNotification);
    presetNameLabel.onSingleClick = [this] { if (onLoadProject) onLoadProject(); };

    mixKnob = std::make_unique<ControlKnobComponent>("Mix", 100.0f, "%");
    addAndMakeVisible(mixKnob.get());

    undoButton.onClick = [this] { if (undoCallback) undoCallback(); };
    redoButton.onClick = [this] { if (redoCallback) redoCallback(); };
    saveProjectButton.onClick = [this] { if (onSaveProject) onSaveProject(); };
    initPresetButton.onClick = [this] { if (onInitPreset) onInitPreset(); };
}

//==============================================================================

void HeaderSection::setPresetName(const juce::String& name)
{
    presetName = name;
    presetNameLabel.setText(presetName.toUpperCase(), juce::dontSendNotification);
    repaint();
}

void HeaderSection::updateUndoState(bool canUndo, bool canRedo)
{
    undoButton.setEnabled(canUndo);
    redoButton.setEnabled(canRedo);

    undoButton.setAlpha(canUndo ? 1.0f : 0.4f);
    redoButton.setAlpha(canRedo ? 1.0f : 0.4f);
}

void HeaderSection::setUndoCallback(std::function<void()> cb)
{
    undoCallback = std::move(cb);
}

void HeaderSection::setRedoCallback(std::function<void()> cb)
{
    redoCallback = std::move(cb);
}

void HeaderSection::setVersionString(const juce::String& version)
{
    versionString = version;
    repaint();
}

void HeaderSection::setProjectURI(const juce::String& uri)
{
    projectURI = uri;
    repaint();
}

//==============================================================================

void HeaderSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // ---------- Background: Modern Dark Charcoal ----------
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillAll();

    // Subtle metallic top highlight
    g.setColour(juce::Colours::white.withAlpha(0.03f));
    g.fillRect(bounds.removeFromTop(1.0f));

    // ---------- Bottom Divider ----------
    g.setColour(juce::Colours::black);
    g.drawLine(0.0f, bounds.getBottom() - 1.0f, bounds.getRight(), bounds.getBottom() - 1.0f, 1.0f);

    const int leftPadding = 24;

    // ---------- Brand Title (Left) ----------
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::Font("Segoe UI", 28.0f, juce::Font::bold));
    juce::Rectangle<int> titleArea(leftPadding, 0, 100, (int)getHeight());
    g.drawText("DUQ", titleArea, juce::Justification::centredLeft);

    // ---------- Preset "Bay" (Center) ----------
    auto centerArea = getLocalBounds().withSizeKeepingCentre(280, 28).toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(centerArea, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(centerArea, 2.0f, 1.0f);
}

//==============================================================================

void HeaderSection::resized()
{
    auto area = getLocalBounds();

    // --- Left Brand ---
    area.removeFromLeft(120);

    const int buttonSize = 24;
    const int spacing = 4;

    // --- Center Preset Group ---
    // [Name Bay (280px)] [Init] [Save]
    auto centerGroupArea = getLocalBounds().withSizeKeepingCentre(400, getHeight());
    
    // Name Bay centered
    int bayWidth = 280;
    auto bayRect = centerGroupArea.withSizeKeepingCentre(bayWidth, 28);
    presetNameLabel.setBounds(bayRect);
    
    // Init and Save next to the bay
    auto initPos = bayRect.getRelativePoint(1.0f, 0.5f).translated(spacing, -buttonSize/2);
    initPresetButton.setBounds(initPos.x, initPos.y, buttonSize, buttonSize);

    auto savePos = initPos.translated(buttonSize + spacing, 0);
    saveProjectButton.setBounds(savePos.x, savePos.y, buttonSize, buttonSize);

    // --- Right Tools Area ---
    auto rightArea = getLocalBounds().removeFromRight(200).reduced(10, 0);
    
    // Mix Knob at the far right
    if (mixKnob)
    {
        auto knobArea = rightArea.removeFromRight(60);
        mixKnob->setBounds(knobArea.reduced(0, 5));
    }

    rightArea.removeFromRight(15); // Gap

    // Undo / Redo
    redoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
    rightArea.removeFromRight(spacing);
    undoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
}
