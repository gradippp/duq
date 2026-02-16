#include "ControlSection.h"

ControlSection::ControlSection()
{
    addAndMakeVisible(smoothKnob);
    addAndMakeVisible(rateKnob);
    addAndMakeVisible(depthKnob);
}

void ControlSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1);

    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.15f));
    g.fillRect(headerArea);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));

    g.drawText("Controls",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);
}

void ControlSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr int headerHeight = 32;
    constexpr int rowHeight = 56;
    constexpr int gap = 4;          // was 6
    constexpr int bottomPadding = 8;

    bounds.removeFromTop(headerHeight);
    bounds.removeFromBottom(bottomPadding);

    rateKnob.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(gap);

    depthKnob.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(gap);

    smoothKnob.setBounds(bounds.removeFromTop(rowHeight));
}
