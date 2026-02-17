#include "EnvelopeListSection.h"
#include "../../actions/EnvelopeUndoActions.h"

EnvelopeListSection::EnvelopeListSection()
{
    addButton.setButtonText("+");
    addButton.setTooltip("Add a new envelope");

    addAndMakeVisible(addButton);

    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    // ---- Create default model first ----
    envelopes.emplace_back(std::make_unique<EnvelopeData>());

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
            if (!undoManager)
                return;

            int index = rows.size();

            undoManager->perform(
                new AddEnvelopeAction(*this, index));
        };
}

void EnvelopeListSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}


void EnvelopeListSection::addEnvelopeAt(int index, std::unique_ptr<EnvelopeData> env)
{
    envelopes.insert(envelopes.begin() + index, std::move(env));

    auto* newRow = rows.insert(index,
        new EnvelopeRowComponent("Env " + juce::String(index + 1)));

    newRow->onDeleteRequested = [this, newRow]()
        {
            removeRow(newRow);
        };

    newRow->onSelected = [this, newRow]()
        {
            int i = rows.indexOf(newRow);
            selectEnvelope(i);
        };

    rowContainer.addAndMakeVisible(newRow);

    selectEnvelope(index);
    resized();
}

std::unique_ptr<EnvelopeData> EnvelopeListSection::removeEnvelopeAt(int index)
{
    if (index < 0 || index >= envelopes.size())
        return nullptr;

    auto removed = std::move(envelopes[index]);
    envelopes.erase(envelopes.begin() + index);

    rows.remove(index);
    resized();

    if (!rows.isEmpty())
        selectEnvelope(juce::jlimit(0, rows.size() - 1, index));
    else
        selectedIndex = -1;

    return removed;
}

void EnvelopeListSection::updateSelectedEnvelope(const EnvelopeData& data)
{
    if (selectedIndex >= 0 &&
        selectedIndex < static_cast<int>(envelopes.size()))
    {
        *envelopes[selectedIndex] = data;
    }
}

EnvelopeData* EnvelopeListSection::getSelectedEnvelope()
{
    if (selectedIndex >= 0 &&
        selectedIndex < static_cast<int>(envelopes.size()))
    {
        return envelopes[selectedIndex].get();
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
        onEnvelopeSelected(*envelopes[index]);
}


void EnvelopeListSection::removeRow(EnvelopeRowComponent* row)
{
    if (!undoManager)
        return;

    int index = rows.indexOf(row);

    if (index >= 0)
    {
        undoManager->perform(
            new RemoveEnvelopeAction(*this, index));
    }
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
