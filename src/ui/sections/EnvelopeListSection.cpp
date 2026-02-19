#include "EnvelopeListSection.h"
#include "../../PluginProcessor.h"
#include "../utils/FontManager.h"
#include "../../model/EnvelopeData.h"
#include "../../Globals.h"

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
            env.setProperty("triggerNote", getNextFreeNote(Theme::Defaults::triggerNote), nullptr);
            env.setProperty("rate", Theme::Defaults::rate, nullptr);
            env.setProperty("depth", (double)Theme::Defaults::depth, nullptr);
            env.setProperty("smooth", (double)Theme::Defaults::smooth, nullptr);
            env.setProperty("rateIsFrequencyMode", Theme::Defaults::rateIsFrequencyMode, nullptr);

            juce::ValueTree points("POINTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints; ++i)
            {
                juce::ValueTree p("POINT");
                p.setProperty("x", Theme::Defaults::defaultPoints[i].x, nullptr);
                p.setProperty("y", Theme::Defaults::defaultPoints[i].y, nullptr);
                points.addChild(p, -1, nullptr);
            }

            juce::ValueTree segments("SEGMENTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints - 1; ++i)
            {
                juce::ValueTree s("SEGMENT");
                s.setProperty("curve", Theme::Defaults::curve, nullptr);
                s.setProperty("type", Theme::Defaults::curveType, nullptr);
                segments.addChild(s, -1, nullptr);
            }

            env.addChild(points, -1, nullptr);
            env.addChild(segments, -1, nullptr);

            const int newIndex = envelopesTree.getNumChildren();
            envelopesTree.addChild(env, -1, undoManager);
            selectEnvelope(newIndex);
        };
}

EnvelopeListSection::~EnvelopeListSection()
{
}

void EnvelopeListSection::setProcessor(DuqAudioProcessor& p)
{
    processor = &p;
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

void EnvelopeListSection::valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier&)
{
    if (v == envelopesTree)
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
        row->setUndoManager(undoManager);

        row->onDeleteRequested = [this, envTree]()
            {
                if (!undoManager || !envelopesTree.isValid())
                    return;

                int index = envelopesTree.indexOf(envTree);
                if (index < 0)
                    return;

                undoManager->beginNewTransaction("Delete Envelope");
                envelopesTree.removeChild(index, undoManager);
            };


        row->onReplaceRequested = [this, envTree]()
            {
                if (onReplaceRequested)
                    onReplaceRequested(envTree);
            };

        row->onNameChanged = [this, i](const juce::String& newName)
            {
                auto envTree = envelopesTree.getChild(i);
                envTree.setProperty("name", newName.trim(), undoManager);
            };

        row->onNoteChanged = [this, envTree](int newNote) mutable
            {
                newNote = juce::jlimit(0, 127, newNote);
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

    g.fillAll(Theme::Colours::sectionBackground);

    // Outer border
    g.setColour(Theme::Colours::border);
    g.drawRect(bounds, 1);

    // ===== Header Area =====
    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(Theme::Colours::headerBackground);
    g.fillRect(headerArea);

    g.setColour(Theme::Colours::textMain);
    g.setFont(FontManager::getBarlowBold(16.0f));

    g.drawText("ENVELOPES",
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

bool EnvelopeListSection::isNoteAlreadyUsed(int note) const
{
    const int count = envelopesTree.getNumChildren();

    for (int i = 0; i < count; ++i)
    {
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
        juce::String candidate = Theme::Defaults::envelopeName + " " + juce::String(counter);
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

    return startFrom;
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
