#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "../utils/ViewportLookAndFeel.h"

class SettingsSection : public juce::Component
{
public:
    SettingsSection();
    ~SettingsSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPage(int index);
    void setProcessor(class DuqAudioProcessor* p);
    class WorkflowPage* getWorkflowPage();
    
    std::function<void()> onClose;

private:
    void updateSidebarButtons();

    struct SidebarButton : public juce::TextButton
    {
        SidebarButton(const juce::String& name) : juce::TextButton(name)
        {
            setClickingTogglesState(true);
            setRadioGroupId(100);
            setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            setColour(juce::TextButton::buttonOnColourId, T_COL(uiHover));
            setColour(juce::TextButton::textColourOffId, T_COL(textDimmed));
            setColour(juce::TextButton::textColourOnId, T_COL(accent));
        }

        void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isMouseDownOnButton) override;
    };

    std::vector<std::unique_ptr<SidebarButton>> sidebarButtons;
    std::vector<std::unique_ptr<juce::Component>> pages;
    
    juce::Viewport viewport;
    ViewportLookAndFeel viewportLNF;
    int activePageIndex = 0;
    int lastViewportWidth = 0;

    juce::DrawableButton backButton{ "back", juce::DrawableButton::ImageFitted };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsSection)
};
