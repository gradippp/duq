#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
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

    enum class MeterMode
    {
        AudioLevel,   // dB scaled
        Envelope      // direct 0-1
    };

    void setMode(MeterMode newMode);

    MeterComponent(std::atomic<float>& source,
        Direction dir = Direction::LeftToRight,
        const juce::String& label = {});

    void setGradient(const juce::ColourGradient& newGradient);
    void setLabel(const juce::String& newLabel);

    void paint(juce::Graphics& g) override;

private:
    MeterMode mode = MeterMode::AudioLevel;

    void timerCallback() override;

    std::atomic<float>& inputLevel;

    Direction meterDirection;

    juce::ColourGradient gradient;

    juce::String labelText;

    float smoothedLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldCount = 0;

    static constexpr float clipThreshold = 0.99f;
    static constexpr int clipHoldFrames = 30;
};
