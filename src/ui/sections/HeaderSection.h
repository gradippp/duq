#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HeaderSection : public juce::Component
{
public:
    HeaderSection();
    ~HeaderSection() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setVersionString(const juce::String& version);
    void setProjectURI(const juce::String& uri);

    void setUndoCallback(std::function<void()> cb);
    void setRedoCallback(std::function<void()> cb);
    void updateUndoState(bool canUndo, bool canRedo);

private:
    juce::String versionString{ PROJECT_VERSION };
    juce::String projectURI{ PROJECT_URI };

    juce::DrawableButton undoButton{ "undo", juce::DrawableButton::ImageFitted };
    juce::DrawableButton redoButton{ "redo", juce::DrawableButton::ImageFitted };

    std::function<void()> undoCallback;
    std::function<void()> redoCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderSection)
};
