#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../utils/FlatKnobLookAndFeel.h"

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
    ControlKnobComponent(const juce::String& name, const float initialValue,
        const juce::String& unitSuffix);

    std::function<void(juce::PopupMenu&)> extendContextMenu;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;
    std::function<void(int)> onCustomMenuResult;
    void showContextMenu();

    void setLabel(const juce::String& text);
    void refreshValueLabel();
    std::function<juce::String(double)> valueFormatter;

    std::function<void(double)> onValueChanged;

    juce::Slider& getSlider() { return knob; }

private:
    void handleCustomMenuResult(int result);

    juce::String labelText;
    juce::String unit;

    ContextSlider knob;
    juce::Label valueLabel;

    void updateValueLabel();
    void showValueEntryDialog();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlKnobComponent)

};
