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
    g.fillAll(juce::Colours::black);

    if (!samples || bufferLength == 0)
        return;

    const int width = getWidth();
    const int height = getHeight();

    const int writeIndex = writePosition->load();

    // Visible range mapping
    const float visibleWidth = 1.0f / zoomX;
    const int startIndex =
        int(offsetX * bufferLength);

    const int samplesPerPixel =
        juce::jmax(1, int(bufferLength * visibleWidth / width));

    g.setColour(juce::Colours::white.withAlpha(0.25f));

    for (int x = 0; x < width; ++x)
    {
        int bufferIndex =
            (writeIndex + startIndex +
                x * samplesPerPixel) % bufferLength;

        float sample = samples[bufferIndex];
        sample = juce::jlimit(-1.0f, 1.0f, sample);

        float y =
            juce::jmap(sample,
                -1.0f, 1.0f,
                (float)height, 0.0f);

        g.drawLine((float)x,
            (float)height / 2.0f,
            (float)x,
            y);
    }
}
