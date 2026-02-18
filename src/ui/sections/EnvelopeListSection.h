#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/EnvelopeRowComponent.h"
#include "../../model/EnvelopeData.h"

class EnvelopeListComponent;
class ChangeEnvelopeNoteAction;
class DuqAudioProcessor;

class EnvelopeListSection : public juce::Component,
    private juce::ChangeListener
{
public:
    EnvelopeListSection();
    ~EnvelopeListSection();

    void resized() override;
    void paint(juce::Graphics& g) override;

    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    int getEnvelopeCount() const;

    EnvelopeData* getSelectedEnvelope();
    void selectEnvelope(int i);
    std::function<void(EnvelopeData*)> onEnvelopeSelected;
    void updateSelectedEnvelope(const EnvelopeData& data);

    void setUndoManager(juce::UndoManager& um);
    void addEnvelopeAt(int index, std::unique_ptr<EnvelopeData> env);
    std::unique_ptr<EnvelopeData> removeEnvelopeAt(int index);

    bool isNoteAlreadyUsed(int note, int ignoreIndex = -1) const;
    int getNextFreeNote(int startFrom = 36) const;

    // For Undo actions only. Does not create transactions.
    bool applyEnvelopeNoteDirect(int index, int newNote);
    bool applyEnvelopeNameDirect(int index, const juce::String& newName);

    juce::String generateDefaultName() const;

    void updateMidiActivity(DuqAudioProcessor& processor);

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void rebuildRowsFromModel();

    bool isInitialising = true;

    juce::Viewport viewport;
    juce::Component rowContainer;
    juce::TextButton addButton;
    juce::OwnedArray<EnvelopeRowComponent> rows;
    void removeRow(EnvelopeRowComponent* row);

    bool trySetEnvelopeNote(int index, int newNote);
    bool trySetEnvelopeName(int index, const juce::String& newName);

    juce::UndoManager* undoManager = nullptr;

    std::vector<std::unique_ptr<EnvelopeData>> envelopes;
    int selectedIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeListSection)
};
