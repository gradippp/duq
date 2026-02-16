#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/ControlKnobComponent.h"

class ControlSection : public juce::Component
{
public:
    ControlSection();

    void resized() override;
    void paint(juce::Graphics& g) override;

    ControlKnobComponent& getRateKnob() { return rateKnob; }
    ControlKnobComponent& getDepthKnob() { return depthKnob; }
    ControlKnobComponent& getSmoothKnob() { return smoothKnob; }

private:
    bool rateIsFrequencyMode = true;
    void applyRateMode();
    ControlKnobComponent rateKnob{ "RATE", "Hz" };
    ControlKnobComponent depthKnob{ "Depth", "%" };
    ControlKnobComponent smoothKnob{ "Smooth", "%" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlSection)
};
