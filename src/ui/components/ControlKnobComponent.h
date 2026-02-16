#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class ContextSlider : public juce::Slider
{
public:
    std::function<void(const juce::MouseEvent&)> rightClickHandler;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown() && rightClickHandler)
        {
            rightClickHandler(e);
            return;
        }

        juce::Slider::mouseDown(e);
    }
};

class ControlKnobComponent : public juce::Component
{
public:
    ControlKnobComponent(const juce::String& name,
        const juce::String& unitSuffix);

    void resized() override;
    void paint(juce::Graphics& g) override;
    //void mouseDown(const juce::MouseEvent& e) override;
    std::function<void(int)> onCustomMenuResult;
    void showContextMenu();

    juce::Slider& getSlider() { return knob; }

private:
    std::function<void(juce::PopupMenu&)> extendContextMenu;
    void handleCustomMenuResult(int result);

    juce::String labelText;
    juce::String unit;

    ContextSlider knob;
    juce::Label valueLabel;

    void updateValueLabel();
    void showValueEntryDialog();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlKnobComponent)

};