#include "HeaderSection.h"

HeaderSection::HeaderSection()
{
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);

    undoButton.onClick = [this]
        {
            if (undoCallback)
                undoCallback();
        };

    redoButton.onClick = [this]
        {
            if (redoCallback)
                redoCallback();
        };
}

void HeaderSection::updateUndoState(bool canUndo, bool canRedo)
{
    undoButton.setEnabled(canUndo);
    redoButton.setEnabled(canRedo);
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

void HeaderSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.fillAll(juce::Colours::black);

    // Bottom divider line
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(0.0f, bounds.getBottom() - 1.0f,
        bounds.getRight(), bounds.getBottom() - 1.0f, 1.0f);

    g.setColour(juce::Colours::white);

    // Large DUQ title (Brand - 56)
    g.setFont(juce::Font(56.0f));
    g.drawText("DUQ",
        20,
        0,
        250,
        getHeight(),
        juce::Justification::centredLeft);

    // Subtitle (MIDI text - 15)
    //g.setFont(juce::Font(15.0f));
    //g.drawText("MIDI-Based Envelope Trigger",
    //    280,
    //    0,
    //    600,
    //    getHeight(),
    //    juce::Justification::centredLeft);

    const int rightPadding = 20;
    const int textWidth = 220;

    // --- URL (top right - 10) ---
    g.setFont(juce::Font(10.0f));
    g.drawText(projectURI,
        getWidth() - textWidth - rightPadding,
        5,
        textWidth,
        20,
        juce::Justification::topRight);

    // --- Version (bottom right - 12) ---
    g.setFont(juce::Font(12.0f));
    g.drawText("v" + versionString,
        getWidth() - textWidth - rightPadding,
        getHeight() - 25,
        textWidth,
        20,
        juce::Justification::bottomRight);
}

void HeaderSection::resized()
{
    auto area = getLocalBounds();

    const int buttonWidth = 70;
    const int buttonHeight = 24;
    const int padding = 20;

    auto rightArea = area.removeFromRight(180);

    undoButton.setBounds(
        rightArea.removeFromLeft(buttonWidth)
        .withSizeKeepingCentre(buttonWidth, buttonHeight));

    rightArea.removeFromLeft(10);

    redoButton.setBounds(
        rightArea.removeFromLeft(buttonWidth)
        .withSizeKeepingCentre(buttonWidth, buttonHeight));
}

