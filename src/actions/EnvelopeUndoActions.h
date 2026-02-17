#pragma once

class AddEnvelopeAction : public juce::UndoableAction
{
public:
    AddEnvelopeAction(EnvelopeListSection& section, int index)
        : list(section), insertIndex(index) {
    }

    bool perform() override
    {
        if (!created) // first time perform
        {
            created = std::make_unique<EnvelopeData>();

            int freeNote = list.getNextFreeNote(36);
            if (freeNote >= 0)
                created->triggerNote = freeNote;
        }

        list.addEnvelopeAt(insertIndex, std::move(created));
        return true;
    }

    bool undo() override
    {
        created = list.removeEnvelopeAt(insertIndex);
        return true;
    }

private:
    EnvelopeListSection& list;
    int insertIndex;
    std::unique_ptr<EnvelopeData> created;
};

class RemoveEnvelopeAction : public juce::UndoableAction
{
public:
    RemoveEnvelopeAction(EnvelopeListSection& section, int index)
        : list(section), removeIndex(index) {
    }

    bool perform() override
    {
        removed = list.removeEnvelopeAt(removeIndex);
        return true;
    }

    bool undo() override
    {
        list.addEnvelopeAt(removeIndex, std::move(removed));
        return true;
    }

private:
    EnvelopeListSection& list;
    int removeIndex;
    std::unique_ptr<EnvelopeData> removed;
};
