#include "EnvelopeListSection.h"
#include "../../actions/EnvelopeUndoActions.h"
#include "../../PluginProcessor.h"

EnvelopeListSection::EnvelopeListSection()
{
    addButton.setButtonText("+");
    addButton.setTooltip("Add a new envelope");

    addAndMakeVisible(addButton);

    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    // ---- Create default envelope first ----
    auto defaultEnv = std::make_unique<EnvelopeData>();
    defaultEnv->triggerNote = 36;
    defaultEnv->name = generateDefaultName();

    addEnvelopeAt(0, std::move(defaultEnv));

    // ---- Add button logic ----
    addButton.onClick = [this]()
        {
            if (!undoManager || isInitialising)
                return;

            int index = envelopes.size();

            undoManager->beginNewTransaction("Add Envelope");
            undoManager->perform(new AddEnvelopeAction(*this, index));
        };

    isInitialising = false;
}

EnvelopeListSection::~EnvelopeListSection()
{
    if (undoManager)
        undoManager->removeChangeListener(this);
}

void EnvelopeListSection::changeListenerCallback(juce::ChangeBroadcaster*)
{
    rebuildRowsFromModel();
}

void EnvelopeListSection::rebuildRowsFromModel()
{
    rowContainer.removeAllChildren();
    rows.clear();

    for (int i = 0; i < envelopes.size(); ++i)
    {
        auto* row = rows.insert(i,
            new EnvelopeRowComponent(*envelopes[i]));

        row->onNameChanged = [this, row](const juce::String& newName)
            {
                int rowIndex = rows.indexOf(row);
                trySetEnvelopeName(rowIndex, newName);
            };

        row->onNoteChanged = [this, row](int newNote)
            {
                int rowIndex = rows.indexOf(row);
                trySetEnvelopeNote(rowIndex, newNote);
            };

        row->onDeleteRequested = [this, row]()
            {
                removeRow(row);
            };

        row->onSelected = [this, row]()
            {
                int idx = rows.indexOf(row);
                selectEnvelope(idx);
            };

        rowContainer.addAndMakeVisible(row);
    }

    // Fix selection bounds
    if (selectedIndex >= envelopes.size())
        selectedIndex = envelopes.size() - 1;

    if (envelopes.empty())
    {
        selectedIndex = -1;

        if (onEnvelopeSelected)
            onEnvelopeSelected(nullptr);

        resized();
        return;
    }

    // If nothing selected yet, select first
    if (selectedIndex < 0)
        selectedIndex = 0;

    selectEnvelope(selectedIndex);

    resized();

}


bool EnvelopeListSection::trySetEnvelopeName(int index,
    const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int>(envelopes.size()))
        return false;

    auto trimmed = newName.trim();

    if (trimmed.isEmpty())
        return false;

    auto& env = envelopes[index];

    if (env->name == trimmed)
        return false; // no change

    if (!undoManager)
        return applyEnvelopeNameDirect(index, trimmed);

    undoManager->beginNewTransaction("Rename Envelope");

    undoManager->perform(
        new ChangeEnvelopeNameAction(*this,
            index,
            env->name,
            trimmed));

    return true;
}

bool EnvelopeListSection::applyEnvelopeNameDirect(int index,
    const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int>(envelopes.size()))
        return false;

    envelopes[index]->name = newName;

    // Update visible row immediately (no full rebuild needed)
    if (index < rows.size())
        rows[index]->setName(newName);

    return true;
}



void EnvelopeListSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
    undoManager->addChangeListener(this);
}


void EnvelopeListSection::addEnvelopeAt(
    int index,
    std::unique_ptr<EnvelopeData> env)
{
    envelopes.insert(envelopes.begin() + index,
        std::move(env));
}


std::unique_ptr<EnvelopeData> EnvelopeListSection::removeEnvelopeAt(int index)
{
    if (index < 0 || index >= envelopes.size())
        return nullptr;

    auto removed = std::move(envelopes[index]);
    envelopes.erase(envelopes.begin() + index);

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
        rows[i]->setSelected(i == index);

    if (onEnvelopeSelected)
        onEnvelopeSelected(envelopes[index].get());
}

void EnvelopeListSection::removeRow(EnvelopeRowComponent* row)
{
    if (!undoManager || isInitialising)
        return;

    int index = rows.indexOf(row);

    if (index >= 0)
    {
        undoManager->beginNewTransaction("Remove Envelope");
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
    return static_cast<int>(envelopes.size());
}

bool EnvelopeListSection::isNoteAlreadyUsed(int note, int ignoreIndex) const
{
    for (int i = 0; i < static_cast<int>(envelopes.size()); ++i)
    {
        if (i == ignoreIndex)
            continue;

        if (envelopes[i]->triggerNote == note)
            return true;
    }

    return false;
}

bool EnvelopeListSection::trySetEnvelopeNote(int index, int newNote)
{
    if (index < 0 || index >= envelopes.size())
        return false;

    newNote = juce::jlimit(0, 127, newNote);

    if (isNoteAlreadyUsed(newNote, index))
        return false;

    int oldNote = envelopes[index]->triggerNote;

    if (oldNote == newNote)
        return false;

    if (!undoManager)
        return applyEnvelopeNoteDirect(index, newNote);

    undoManager->beginNewTransaction("Change Envelope Note");

    undoManager->perform(
        new ChangeEnvelopeNoteAction(*this,
            index,
            oldNote,
            newNote));

    return true;
}

juce::String EnvelopeListSection::generateDefaultName() const
{
    int counter = 1;

    while (true)
    {
        juce::String candidate = "Env " + juce::String(counter);

        bool exists = false;

        for (const auto& env : envelopes)
        {
            if (env->name == candidate)
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


bool EnvelopeListSection::applyEnvelopeNoteDirect(int index, int newNote)
{
    if (index < 0 || index >= envelopes.size())
        return false;

    envelopes[index]->triggerNote = newNote;

    if (index < rows.size())
        rows[index]->setTriggerNote(newNote);

    return true;
}

int EnvelopeListSection::getNextFreeNote(int startFrom) const
{
    for (int note = startFrom; note <= 127; ++note)
    {
        if (!isNoteAlreadyUsed(note))
            return note;
    }

    return -1; // no notes available
}

void EnvelopeListSection::updateMidiActivity(DuqAudioProcessor& processor)
{
    for (int i = 0; i < rows.size(); ++i)
    {
        int note = envelopes[i]->triggerNote;
        bool isActive = processor.isNoteActive(note);

        rows[i]->setActive(isActive);
    }
}
