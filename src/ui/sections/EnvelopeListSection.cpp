#include "EnvelopeListSection.h"

EnvelopeListSection::EnvelopeListSection()
{
    addButton.setButtonText("+");
    addButton.setTooltip("Add a new envelope");

    addAndMakeVisible(addButton);

    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    // ---- Create default model first ----
    envelopes.emplace_back();

    // ---- Create default row ----
    auto* row = rows.add(new EnvelopeRowComponent("Default"));

    row->onDeleteRequested = [this, row]()
        {
            removeRow(row);
        };

    row->onSelected = [this, row]()
        {
            int index = rows.indexOf(row);
            selectEnvelope(index);
        };

    rowContainer.addAndMakeVisible(row);

    // ---- Select first envelope ----
    selectEnvelope(0);

    // ---- Add button logic ----
    addButton.onClick = [this]()
        {
            const int newIndex = rows.size();

            envelopes.emplace_back();

            auto* newRow = rows.add(
                new EnvelopeRowComponent("Env " + juce::String(newIndex + 1))
            );

            newRow->onDeleteRequested = [this, newRow]()
                {
                    removeRow(newRow);
                };

            newRow->onSelected = [this, newRow]()
                {
                    int index = rows.indexOf(newRow);
                    selectEnvelope(index);
                };

            rowContainer.addAndMakeVisible(newRow);
            selectEnvelope(newIndex);
            viewport.setViewPosition(0, newIndex * 28); // automatic scrolling

            resized();
        };
}

void EnvelopeListSection::updateSelectedEnvelope(const EnvelopeData& data)
{
    if (selectedIndex >= 0 &&
        selectedIndex < envelopes.size())
    {
        envelopes[selectedIndex] = data;
    }
}

EnvelopeData* EnvelopeListSection::getSelectedEnvelope()
{
    if (selectedIndex >= 0 &&
        selectedIndex < static_cast<int>(envelopes.size()))
    {
        return &envelopes[selectedIndex];
    }

    return nullptr;
}

void EnvelopeListSection::selectEnvelope(int index)
{
    if (index < 0 || index >= envelopes.size())
        return;

    selectedIndex = index;

    for (int i = 0; i < rows.size(); ++i)
        rows[i]->setActive(i == index);

    if (onEnvelopeSelected)
        onEnvelopeSelected(envelopes[index]);
}


void EnvelopeListSection::removeRow(EnvelopeRowComponent* row)
{
    int index = rows.indexOf(row);

    if (index >= 0 && index < envelopes.size())
        envelopes.erase(envelopes.begin() + index);

    rows.removeObject(row, true);

    if (selectedIndex == index)
    {
        if (!rows.isEmpty())
            selectEnvelope(juce::jlimit(0, rows.size() - 1, index - 1));
        else
            selectedIndex = -1;
    }
    else if (selectedIndex > index)
    {
        selectEnvelope(selectedIndex - 1);
    }

    resized();
    repaint();
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
