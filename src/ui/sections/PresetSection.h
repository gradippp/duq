#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DuqAudioProcessor;

class PresetSection : public juce::Component,
    private juce::ListBoxModel
{
public:
    enum class Mode
    {
        Envelope,
        Project,
        Import
    };

    PresetSection(DuqAudioProcessor& p);
    ~PresetSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTargetEnvelope(juce::ValueTree envelope);
    void setMode(Mode newMode);
    void setUndoManager(juce::UndoManager& um);

    std::function<void()> onClose;
    std::function<void(juce::String)> onProjectLoaded;
    std::function<void(int)> onEnvelopeImported;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int rowNumber, const juce::MouseEvent& e) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isSelected, juce::Component* existingComponentToUpdate) override;

private:
    void refreshPresetList();
    void filterPresets();
    void deletePreset(int index);

    DuqAudioProcessor& processor;
    Mode mode = Mode::Envelope;

    juce::ValueTree targetEnvelope;
    juce::UndoManager* undoManager = nullptr;

    juce::DrawableButton closeButton{ "close", juce::DrawableButton::ImageFitted };
    juce::Label titleLabel{ "title", "SELECT PRESET" };

    // Search
    juce::TextEditor searchEditor;
    juce::String searchText;

    // List
    juce::ListBox presetList;
    std::vector<juce::File> allFiles;
    std::vector<juce::File> filteredFiles;

    class PresetRowComponent : public juce::Component
    {
    public:
        PresetRowComponent(PresetSection& owner, int index);
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseEnter(const juce::MouseEvent&) override { isHovering = true; repaint(); }
        void mouseExit(const juce::MouseEvent&) override { isHovering = false; repaint(); }
        void mouseDown(const juce::MouseEvent& e) override;

        void update(int newIndex, bool isSelected);

    private:
        PresetSection& owner;
        int rowDataIndex;
        bool isSelected = false;
        bool isHovering = false;
        juce::DrawableButton deleteButton{ "delete", juce::DrawableButton::ImageFitted };
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSection)
};
