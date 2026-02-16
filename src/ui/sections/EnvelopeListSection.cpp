#include "EnvelopeListSection.h"

EnvelopeListSection::EnvelopeListSection()
{
    addButton.setButtonText("+");

    addAndMakeVisible(addButton);

    // Setup viewport
    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false); // vertical only
    addAndMakeVisible(viewport);

    rows.add(new EnvelopeRowComponent("Default"));

    for (auto* row : rows)
        rowContainer.addAndMakeVisible(row);

    addButton.onClick = [this]()
        {
            const int newIndex = rows.size() + 1;

            auto* newRow = rows.add(
                new EnvelopeRowComponent("Env " + juce::String(newIndex))
            );

            rowContainer.addAndMakeVisible(newRow);

            resized();
        };
}

void EnvelopeListSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);

    // Outer border
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1);

    // ===== Header Area =====
    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.15f));
    g.fillRect(headerArea);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));

    g.drawText("Envelopes",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);
}

void EnvelopeListSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr int headerHeight = 32;
    constexpr int addButtonHeight = 28;
    constexpr int rowHeight = 28;

    // Remove header
    bounds.removeFromTop(headerHeight);

    // Bottom add button
    auto buttonArea = bounds.removeFromBottom(addButtonHeight);
    addButton.setBounds(buttonArea.reduced(6));

    // Viewport takes remaining space
    viewport.setBounds(bounds);

    // ===== Layout rows inside rowContainer =====

    int totalHeight = rows.size() * rowHeight;

    rowContainer.setSize(bounds.getWidth(), totalHeight);

    auto rowBounds = rowContainer.getLocalBounds();

    for (auto* row : rows)
        row->setBounds(rowBounds.removeFromTop(rowHeight));
}
