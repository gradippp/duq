#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class ControlSliderComponent : public juce::Component
{
public:
    ControlSliderComponent(const juce::String& label,
        const juce::String& unitSuffix);

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getSlider() { return slider; }

private:
    juce::String labelText;
    juce::String unit;

    juce::Slider slider;
    juce::Label valueLabel;

    void updateValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlSliderComponent)
};
