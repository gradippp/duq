#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/EnvelopeRowComponent.h"

class DuqAudioProcessor;

class EnvelopeListSection : public juce::Component,
    public juce::DragAndDropContainer,
    public juce::DragAndDropTarget,
    private juce::ValueTree::Listener
{
public:
    class CustomButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override;
        juce::Font getTextButtonFont(juce::TextButton&, int) override;
    };

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
    std::function<void(juce::ValueTree)> onSaveRequested;
    std::function<void()> onImportRequested;

    void setUndoManager(juce::UndoManager& um);

    int getNextFreeNote(int startFrom = 36) const;

    juce::String generateDefaultName() const;

    void updateMidiActivity(DuqAudioProcessor& processor);

    // Drag and Drop Target
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

private:
    // ValueTree listener
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    void rebuildRowsFromModel();

    juce::ValueTree envelopesTree;
    DuqAudioProcessor* processor = nullptr;

    juce::Viewport viewport;
    juce::Component rowContainer;
    CustomButtonLookAndFeel buttonLnf;
    juce::TextButton addButton;
    juce::TextButton importButton;
    juce::OwnedArray<EnvelopeRowComponent> rows;

    juce::UndoManager* undoManager = nullptr;

    int selectedIndex = -1;
    int dropIndex = -1;
    bool isDragging = false;

    bool isNoteAlreadyUsed(int note) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeListSection)
};