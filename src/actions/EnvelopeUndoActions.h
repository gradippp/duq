#pragma once

class AddEnvelopeAction : public juce::UndoableAction
{
public:
    AddEnvelopeAction(EnvelopeListSection& section, int index)
        : list(section),
        insertIndex(index),
        previousSelectedIndex(section.getSelectedIndex())
    {
    }

    bool perform() override
    {
        if (!created)
        {
            created = std::make_unique<EnvelopeData>();

            int freeNote = list.getNextFreeNote(36);
            if (freeNote >= 0)
                created->triggerNote = freeNote;
        }

        list.addEnvelopeAt(insertIndex, std::move(created));
        list.setSelectedIndex(insertIndex);
        return true;
    }

    bool undo() override
    {
        created = list.removeEnvelopeAt(insertIndex);
        list.setSelectedIndex(previousSelectedIndex);
        return true;
    }

private:
    EnvelopeListSection& list;
    int insertIndex;
    int previousSelectedIndex;
    std::unique_ptr<EnvelopeData> created;
};

class RemoveEnvelopeAction : public juce::UndoableAction
{
public:
    RemoveEnvelopeAction(EnvelopeListSection& section, int index)
        : list(section),
        removeIndex(index),
        previousSelectedIndex(section.getSelectedIndex())
    {
    }

    bool perform() override
    {
        removed = list.removeEnvelopeAt(removeIndex);

        // Select neighbor if possible
        int newSelection =
            juce::jlimit(0,
                list.getEnvelopeCount() - 1,
                removeIndex);

        list.setSelectedIndex(newSelection);

        return true;
    }

    bool undo() override
    {
        list.addEnvelopeAt(removeIndex, std::move(removed));
        list.setSelectedIndex(previousSelectedIndex);
        return true;
    }

private:
    EnvelopeListSection& list;
    int removeIndex;
    int previousSelectedIndex;
    std::unique_ptr<EnvelopeData> removed;
};

class ChangeEnvelopeNoteAction : public juce::UndoableAction
{
public:
    ChangeEnvelopeNoteAction(EnvelopeListSection& sectionRef,
        int indexToChange,
        int oldValue,
        int newValue)
        : section(sectionRef),
        index(indexToChange),
        oldNote(oldValue),
        newNote(newValue)
    {
    }

    bool perform() override
    {
        return section.applyEnvelopeNoteDirect(index, newNote);
    }

    bool undo() override
    {
        return section.applyEnvelopeNoteDirect(index, oldNote);
    }

private:
    EnvelopeListSection& section;
    int index;
    int oldNote;
    int newNote;
};
