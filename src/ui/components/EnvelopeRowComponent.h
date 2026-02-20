#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "SelectableLabel.h"

class EnvelopeRowComponent : public juce::Component,
    private juce::ValueTree::Listener
{
public:
    EnvelopeRowComponent(juce::ValueTree envelopeTree);
    ~EnvelopeRowComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    void setSelected(bool shouldBeSelected);

    void setUndoManager(juce::UndoManager* um) { undoManager = um; }

    std::function<void()> onDeleteRequested;
    std::function<void()> onReplaceRequested;
    std::function<void()> onSaveRequested;
    std::function<void()> onSelected;
    std::function<void(int)> onNoteChanged;
    std::function<void(const juce::String&)> onNameChanged;

private:
    // ValueTree model
    juce::ValueTree envelope;

    // UI
    SelectableLabel nameLabel;
    juce::TextButton noteButton{ "-" };
    juce::DrawableButton saveButton{ "save", juce::DrawableButton::ImageFitted };
    juce::DrawableButton replaceButton{ "replace", juce::DrawableButton::ImageFitted };
    juce::DrawableButton deleteButton{ "delete", juce::DrawableButton::ImageFitted };

    juce::UndoManager* undoManager = nullptr;

    bool isActive = false;
    bool isHovered = false;
    bool isSelected = false;

    void showContextMenu();
    void refreshFromTree();

    // ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeRowComponent)
};
