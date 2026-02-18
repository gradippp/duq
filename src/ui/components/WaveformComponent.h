#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class WaveformComponent : public juce::Component,
    private juce::Timer
{
public:
    WaveformComponent();

    void setSampleBuffer(const std::atomic<int>* writePos,
        const float* sampleData,
        int bufferSize);

    void setViewState(float zoomX, float offsetX, float zoomY = 1.0f, float offsetY = 0.0f);

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    const std::atomic<int>* writePosition = nullptr;
    const float* samples = nullptr;
    int bufferLength = 0;

    float zoomX = 1.0f;
    float offsetX = 0.0f;
    float zoomY = 1.0f;
    float offsetY = 0.0f;
};
