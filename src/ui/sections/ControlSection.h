#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/ControlKnobComponent.h"

class ControlSection : public juce::Component,
    private juce::ValueTree::Listener
{
public:
    ControlSection();
    ~ControlSection() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void setEnvelope(juce::ValueTree env);
    void clearEnvelope();

    void setUndoManager(juce::UndoManager& um);

    ControlKnobComponent& getRateKnob() { return rateKnob; }
    ControlKnobComponent& getDepthKnob() { return depthKnob; }
    ControlKnobComponent& getSmoothKnob() { return smoothKnob; }

private:
    // ValueTree listener
    void valueTreePropertyChanged(juce::ValueTree&,
        const juce::Identifier&) override;

    void refreshFromTree();
    void applyRateMode();

    juce::ValueTree envelope;
    juce::UndoManager* undoManager = nullptr;

    bool hasEnvelope = false;
    bool isInitialising = false;
    bool rateIsFrequencyMode = true;

    std::array<ControlKnobComponent*, 3> knobList;

    ControlKnobComponent rateKnob{ "Frequency", 20.0f, "Hz" };
    ControlKnobComponent depthKnob{ "Depth", 100.0f, "%" };
    ControlKnobComponent smoothKnob{ "Smooth", 0.0f, "%" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlSection)
};