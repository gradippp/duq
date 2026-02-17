#pragma once

class AddEnvelopeAction : public juce::UndoableAction
{
public:
    AddEnvelopeAction(EnvelopeListSection& section, int index)
        : list(section), insertIndex(index) {
    }

    bool perform() override
    {
        list.addEnvelopeAt(insertIndex,
            std::make_unique<EnvelopeData>());
        return true;
    }

    bool undo() override
    {
        removed = list.removeEnvelopeAt(insertIndex);
        return true;
    }

private:
    EnvelopeListSection& list;
    int insertIndex;
    std::unique_ptr<EnvelopeData> removed;
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
