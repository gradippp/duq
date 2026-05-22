#pragma once
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/ControlKnobComponent.h"
#include "../components/SelectableLabel.h"
#include "../../Globals.h"

class CompactTimingSlider : public juce::Slider
{
public:
    CompactTimingSlider(const juce::String& label);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::String labelName;
    void showValueEntryDialog();
};

class CompactKnob : public juce::Slider
{
public:
    CompactKnob(const juce::String& label);
    ~CompactKnob() override;
    void paint(juce::Graphics& g) override;

private:
    juce::String labelName;
};

class HeaderSection : public juce::Component,
                      public juce::ChangeListener
{
public:
    HeaderSection();
    ~HeaderSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setVersionString(const juce::String& version);
    void setProjectURI(const juce::String& uri);
    void setPresetName(const juce::String& name);

    void setUndoCallback(std::function<void()> cb);
    void setRedoCallback(std::function<void()> cb);
    
    void setSaveProjectCallback(std::function<void()> cb) { onSaveProject = std::move(cb); }
    void setLoadProjectCallback(std::function<void()> cb) { onLoadProject = std::move(cb); }
    void setInitPresetCallback(std::function<void()> cb) { onInitPreset = std::move(cb); }
    void setAboutCallback(std::function<void()> cb) { onAboutClicked = std::move(cb); }
    void setSettingsCallback(std::function<void()> cb) { onSettingsClicked = std::move(cb); }

    void setupAttachments(juce::AudioProcessorValueTreeState& vts);

    void setUndoManager(juce::UndoManager* um);
    void updateUndoState(bool canUndo, bool canRedo);
    void refreshTheme();

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    juce::UndoManager* undoManager = nullptr;
    juce::String versionString{ PROJECT_VERSION };
    juce::String projectURI{ PROJECT_URI };
    juce::String presetName{ Defaults::projectName };

    juce::DrawableButton undoButton{ "undo", juce::DrawableButton::ImageFitted };
    juce::DrawableButton redoButton{ "redo", juce::DrawableButton::ImageFitted };
    
    juce::DrawableButton saveProjectButton{ "save_project", juce::DrawableButton::ImageFitted };
    juce::DrawableButton initPresetButton{ "init_preset", juce::DrawableButton::ImageFitted };
    juce::DrawableButton settingsButton{ "settings", juce::DrawableButton::ImageFitted };
    
    SelectableLabel presetNameLabel;
    SelectableLabel brandLabel;

    CompactTimingSlider lookaheadSlider;

    CompactKnob mixKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lookaheadAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    std::function<void()> undoCallback;
    std::function<void()> redoCallback;
    std::function<void()> onSaveProject;
    std::function<void()> onLoadProject;
    std::function<void()> onInitPreset;
    std::function<void()> onAboutClicked;
    std::function<void()> onSettingsClicked;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderSection)
};
