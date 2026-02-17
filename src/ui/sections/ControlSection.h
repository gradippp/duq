#pragma once
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/ControlKnobComponent.h"
#include "../../model/EnvelopeData.h"

class ControlSection : public juce::Component
{
public:
    ControlSection();

    void resized() override;
    void paint(juce::Graphics& g) override;

    void loadEnvelope(EnvelopeData& data);

    ControlKnobComponent& getRateKnob() { return rateKnob; }
    ControlKnobComponent& getDepthKnob() { return depthKnob; }
    ControlKnobComponent& getSmoothKnob() { return smoothKnob; }

    std::function<void(const EnvelopeData&)> onEnvelopeChanged;

    void setUndoManager(juce::UndoManager& um);

private:
    bool isInitialising = true;

    EnvelopeData* currentData = nullptr;
    juce::UndoManager* undoManager = nullptr;

    bool rateIsFrequencyMode = true;
    void applyRateMode();
    std::array<ControlKnobComponent*, 3> knobList;
    std::array<double EnvelopeData::*, 3> dataMembers =
    {
        &EnvelopeData::rate,
        &EnvelopeData::depth,
        &EnvelopeData::smooth
    };

    ControlKnobComponent rateKnob{ "Frequency", 20.0f, "Hz" };
    ControlKnobComponent depthKnob{ "Depth", 100.0f, "%" };
    ControlKnobComponent smoothKnob{ "Smooth", 0.0f, "%" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlSection)
};
