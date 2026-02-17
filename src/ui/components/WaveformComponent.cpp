#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    setOpaque(true);
    setInterceptsMouseClicks(false, false);
    startTimerHz(60);
}

void WaveformComponent::setSampleBuffer(
    const std::atomic<int>* writePos,
    const float* sampleData,
    int bufferSize)
{
    writePosition = writePos;
    samples = sampleData;
    bufferLength = bufferSize;
}

void WaveformComponent::setViewState(float zx, float ox)
{
    zoomX = zx;
    offsetX = ox;
}

void WaveformComponent::timerCallback()
{
    if (isVisible())
        repaint();
}

void WaveformComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111111));

    if (!samples || bufferLength <= 0)
        return;

    const int width = getWidth();
    const int height = getHeight();
    const float centerY = height * 0.5f;

    const float samplesPerPixel =
        (float)bufferLength / (float)width;

    g.setColour(juce::Colours::azure);

    for (int x = 0; x < width; ++x)
    {
        int start = (int)(x * samplesPerPixel);
        int end = (int)((x + 1) * samplesPerPixel);

        start = juce::jlimit(0, bufferLength - 1, start);
        end = juce::jlimit(start + 1, bufferLength, end);

        float minVal = 1.0f;
        float maxVal = -1.0f;

        for (int i = start; i < end; ++i)
        {
            float v = samples[i];
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
        }

        float yTop = juce::jmap(maxVal, -1.0f, 1.0f,
            (float)height, 0.0f);

        float yBottom = juce::jmap(minVal, -1.0f, 1.0f,
            (float)height, 0.0f);

        g.drawLine((float)x, yTop,
            (float)x, yBottom, 1.0f);
    }

    // subtle center line
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawLine(0, centerY, width, centerY);
}

