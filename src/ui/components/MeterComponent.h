#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

class MeterComponent : public juce::Component,
    private juce::Timer
{
public:
    enum class Direction
    {
        LeftToRight,
        RightToLeft
    };

    MeterComponent(std::atomic<float>& source,
        Direction dir = Direction::LeftToRight,
        const juce::String& label = {});

    void setGradient(const juce::ColourGradient& newGradient);
    void setLabel(const juce::String& newLabel);

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    std::atomic<float>& inputLevel;

    float smoothedLevel = 0.0f;

    Direction meterDirection;

    juce::ColourGradient gradient;

    juce::String labelText;
};
