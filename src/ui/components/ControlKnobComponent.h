#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class ControlKnobComponent : public juce::Component
{
public:
    ControlKnobComponent(const juce::String& name,
        const juce::String& unitSuffix);

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getSlider() { return knob; }

private:
    juce::String labelText;
    juce::String unit;

    juce::Slider knob;
    juce::Label valueLabel;

    void updateValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlKnobComponent)
};
