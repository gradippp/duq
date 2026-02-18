#include "EnvelopeListSection.h"
#include "../../PluginProcessor.h"

EnvelopeListSection::EnvelopeListSection()
{
    addButton.setButtonText("+");
    addButton.setTooltip("Add a new envelope");

    addAndMakeVisible(addButton);

    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    addButton.onClick = [this]()
        {
            if (!undoManager || !envelopesTree.isValid())
                return;

            undoManager->beginNewTransaction("Add Envelope");

            juce::ValueTree env("ENVELOPE");
            env.setProperty("name", generateDefaultName(), nullptr);
            env.setProperty("triggerNote", getNextFreeNote(36), nullptr);
            env.setProperty("rate", 20.0, nullptr);
            env.setProperty("depth", 100.0, nullptr);
            env.setProperty("smooth", 0.0, nullptr);
            env.setProperty("rateIsFrequencyMode", true, nullptr);

            juce::ValueTree points("POINTS");

            juce::ValueTree p1("POINT");
            p1.setProperty("x", 0.0f, nullptr);
            p1.setProperty("y", 0.0f, nullptr);
            p1.setProperty("curve", 0.0f, nullptr);

            juce::ValueTree p2("POINT");
            p2.setProperty("x", 1.0f, nullptr);
            p2.setProperty("y", 1.0f, nullptr);
            p2.setProperty("curve", 0.0f, nullptr);

            points.addChild(p1, -1, nullptr);
            points.addChild(p2, -1, nullptr);

            env.addChild(points, -1, nullptr);

            envelopesTree.addChild(env, -1, undoManager);
        };
}

EnvelopeListSection::~EnvelopeListSection()
{
}

void EnvelopeListSection::setProcessor(DuqAudioProcessor& p)
{
    envelopesTree = p.getEnvelopesTree();

    envelopesTree.addListener(this);

    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    rebuildRowsFromModel();
}


void EnvelopeListSection::rebuildRowsFromModel()
{
    rowContainer.removeAllChildren();
    rows.clear();

    const int count = envelopesTree.getNumChildren();

    for (int i = 0; i < count; ++i)
    {
        auto envTree = envelopesTree.getChild(i);

        auto* row = new EnvelopeRowComponent(envTree);

        row->onNameChanged = [this, i](const juce::String& newName)
            {
                auto envTree = envelopesTree.getChild(i);
                envTree.setProperty("name", newName.trim(), undoManager);
            };

        row->onNoteChanged = [this, envTree](int newNote) mutable
            {
                newNote = juce::jlimit(0, 127, newNote);

                if (!isNoteAlreadyUsed(newNote, envelopesTree.indexOf(envTree)))
                    envTree.setProperty("triggerNote", newNote, undoManager);
            };

        row->onSelected = [this, i]()
            {
                selectEnvelope(i);
            };

        rows.add(row);
        rowContainer.addAndMakeVisible(row);
    }

    if (selectedIndex >= count)
        selectedIndex = count - 1;

    if (count == 0)
    {
        selectedIndex = -1;

        if (onEnvelopeSelected)
            onEnvelopeSelected(juce::ValueTree());

        resized();
        return;
    }

    if (selectedIndex < 0)
        selectedIndex = 0;

    selectEnvelope(selectedIndex);
    resized();
}


void EnvelopeListSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}

void EnvelopeListSection::selectEnvelope(int index)
{
    if (index < 0 || index >= envelopesTree.getNumChildren())
        return;

    selectedIndex = index;

    for (int i = 0; i < rows.size(); ++i)
        rows[i]->setSelected(i == index);

    if (onEnvelopeSelected)
        onEnvelopeSelected(envelopesTree.getChild(index));
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

void EnvelopeListSection::setSelectedIndex(int index)
{
    selectedIndex = index;
}

int EnvelopeListSection::getSelectedIndex() const
{
    return selectedIndex;
}

int EnvelopeListSection::getEnvelopeCount() const
{
    return envelopesTree.getNumChildren();
}

bool EnvelopeListSection::isNoteAlreadyUsed(int note, int ignoreIndex) const
{
    const int count = envelopesTree.getNumChildren();

    for (int i = 0; i < count; ++i)
    {
        if (i == ignoreIndex)
            continue;

        if ((int)envelopesTree.getChild(i)["triggerNote"] == note)
            return true;
    }

    return false;
}

juce::String EnvelopeListSection::generateDefaultName() const
{
    int counter = 1;

    while (true)
    {
        juce::String candidate = "Env " + juce::String(counter);
        bool exists = false;

        for (int i = 0; i < envelopesTree.getNumChildren(); ++i)
        {
            if (envelopesTree.getChild(i)["name"].toString() == candidate)
            {
                exists = true;
                break;
            }
        }

        if (!exists)
            return candidate;

        ++counter;
    }
}

int EnvelopeListSection::getNextFreeNote(int startFrom) const
{
    for (int note = startFrom; note <= 127; ++note)
    {
        if (!isNoteAlreadyUsed(note))
            return note;
    }

    return 36;
}

void EnvelopeListSection::updateMidiActivity(DuqAudioProcessor& processor)
{
    for (int i = 0; i < rows.size(); ++i)
    {
        int note = (int)envelopesTree.getChild(i)["triggerNote"];
        bool isActive = processor.isNoteActive(note);
        rows[i]->setActive(isActive);
    }
}
