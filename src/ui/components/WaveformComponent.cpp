#include "WaveformComponent.h"
#include "../../Globals.h"

WaveformComponent::WaveformComponent()
{
    setOpaque(false);
    setInterceptsMouseClicks(false, false);
    startTimerHz(60);
}

WaveformComponent::~WaveformComponent()
{
    stopTimer();
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

void WaveformComponent::setViewState(float zx, float ox, float zy, float oy)
{
    zoomX = zx;
    offsetX = ox;
    zoomY = zy;
    offsetY = oy;
    repaint();
}

void WaveformComponent::timerCallback()
{
    if (isVisible())
        repaint();
}

void WaveformComponent::paint(juce::Graphics& g)
{
    if (!samples || bufferLength <= 0)
        return;

    const int width = getWidth();
    const int height = getHeight();
    const float centerY = height * 0.5f;

    const float visibleWidthNorm = 1.0f / zoomX;
    const float visibleHeightNorm = 1.0f / zoomY;
    const float startSample = offsetX * bufferLength;
    const float visibleSamples = visibleWidthNorm * bufferLength;
    const float samplesPerPixel = visibleSamples / (float)width;

    juce::Path waveformPath;
    bool started = false;

    for (int x = 0; x < width; ++x)
    {
        float sampleIdx = startSample + (x * samplesPerPixel);
        int start = (int)sampleIdx;
        int end = (int)(sampleIdx + samplesPerPixel);

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

        // Map amplitude [-1, 1] to normalized Y [0, 1]
        float normYTop = (maxVal + 1.0f) * 0.5f;
        float normYBottom = (minVal + 1.0f) * 0.5f;

        // Apply grid vertical zoom and offset
        float nyTop = (normYTop - offsetY) / visibleHeightNorm;
        float nyBottom = (normYBottom - offsetY) / visibleHeightNorm;

        // Map to pixels
        float yTop = (1.0f - nyTop) * (float)height;
        float yBottom = (1.0f - nyBottom) * (float)height;

        if (!started)
        {
            waveformPath.startNewSubPath((float)x, yTop);
            waveformPath.lineTo((float)x, yBottom);
            started = true;
        }
        else
        {
            waveformPath.lineTo((float)x, yTop);
            waveformPath.lineTo((float)x, yBottom);
        }
    }

    g.setColour(T_COL(waveform));
    g.strokePath(waveformPath, juce::PathStrokeType(1.0f));

    // subtle center line (0.0 amplitude -> normY = 0.5)
    float nyCenter = (0.5f - offsetY) / visibleHeightNorm;
    float yCenter = (1.0f - nyCenter) * height;
    g.setColour(T_COL(gridMinor));
    g.drawLine(0, yCenter, width, yCenter);
}

