#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PresetSection : public juce::Component,
    private juce::ListBoxModel
{
public:
    PresetSection();
    ~PresetSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTargetEnvelope(juce::ValueTree envelope);
    void setUndoManager(juce::UndoManager& um);

    std::function<void()> onClose;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int rowNumber, const juce::MouseEvent& e) override;

private:
    void refreshPresetList();

    juce::ValueTree targetEnvelope;
    juce::UndoManager* undoManager = nullptr;

    juce::DrawableButton closeButton{ "close", juce::DrawableButton::ImageFitted };
    juce::Label titleLabel{ "title", "SELECT PRESET" };

    juce::ListBox presetList;
    std::vector<juce::File> presetFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSection)
};
