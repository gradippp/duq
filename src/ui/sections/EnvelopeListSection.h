#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/EnvelopeRowComponent.h"

class DuqAudioProcessor;

class EnvelopeListSection : public juce::Component,
    private juce::ValueTree::Listener
{
public:
    EnvelopeListSection();
    ~EnvelopeListSection() override;

    void setProcessor(DuqAudioProcessor& p);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    int getEnvelopeCount() const;

    void selectEnvelope(int index);

    // Now returns ValueTree, not raw model pointer
    std::function<void(juce::ValueTree)> onEnvelopeSelected;
    std::function<void(juce::ValueTree)> onReplaceRequested;

    void setUndoManager(juce::UndoManager& um);

    bool isNoteAlreadyUsed(int note, int ignoreIndex = -1) const;
    int getNextFreeNote(int startFrom = 36) const;

    juce::String generateDefaultName() const;

    void updateMidiActivity(DuqAudioProcessor& processor);

private:
    // ValueTree listener
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    void rebuildRowsFromModel();

    juce::ValueTree envelopesTree;

    juce::Viewport viewport;
    juce::Component rowContainer;
    juce::TextButton addButton;
    juce::OwnedArray<EnvelopeRowComponent> rows;

    juce::UndoManager* undoManager = nullptr;

    int selectedIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeListSection)
};