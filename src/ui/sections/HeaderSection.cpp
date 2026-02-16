#include "HeaderSection.h"

HeaderSection::HeaderSection()
{
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
}
