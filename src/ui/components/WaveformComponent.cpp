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

void WaveformComponent::setSampleBuffers(
    const std::atomic<int>* writePos,
    const float* preData,
    const float* postData,
    const float* sidechainData,
    int bufferSize)
{
    writePosition = writePos;
    samplesPre = preData;
    samplesPost = postData;
    samplesSidechain = sidechainData;
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
    if (!samplesPre || !samplesPost || bufferLength <= 0)
        return;

    const int width = getWidth();
    const int height = getHeight();

    const float visibleWidthNorm = 1.0f / zoomX;
    const float visibleHeightNorm = 1.0f / zoomY;
    const float startSample = offsetX * bufferLength;
    const float visibleSamples = visibleWidthNorm * bufferLength;
    const float samplesPerPixel = visibleSamples / (float)width;

    auto createWaveformPath = [&](const float* data) -> juce::Path
    {
        juce::Path path;
        if (!data) return path;

        int step = 1;
        int quality = config->getWaveformQuality();
        if (quality == 0)      step = 4;
        else if (quality == 1) step = 2;

        bool started = false;
        for (int x = 0; x < width; x += step)
        {
            float curSamplesPerPixel = samplesPerPixel * step;
            float sampleIdx = startSample + (x * samplesPerPixel);
            int start = (int)sampleIdx;
            int end = (int)(sampleIdx + curSamplesPerPixel);

            start = juce::jlimit(0, bufferLength - 1, start);
            end = juce::jlimit(start + 1, bufferLength, end);

            float minVal = 1.0f;
            float maxVal = -1.0f;

            int sampleStep = (quality == 0) ? 16 : (quality == 1) ? 4 : 1;
            for (int i = start; i < end; i += sampleStep)
            {
                float v = data[i];
                minVal = std::min(minVal, v);
                maxVal = std::max(maxVal, v);
            }
            
            if (minVal > maxVal) continue;

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
                path.startNewSubPath((float)x, yTop);
                path.lineTo((float)x, yBottom);
                started = true;
            }
            else
            {
                path.lineTo((float)x, yTop);
                path.lineTo((float)x, yBottom);
            }
        }
        return path;
    };

    // 1. Draw Sidechain (Background)
    if (samplesSidechain && config->getShowSidechainSignal())
    {
        auto scPath = createWaveformPath(samplesSidechain);
        g.setColour(T_COL(sidechain).withAlpha(1.0f)); 
        g.strokePath(scPath, juce::PathStrokeType(1.2f));
    }

    // 2. Draw Pre (Dry)
    if (config->getShowSourceSignal())
    {
        auto prePath = createWaveformPath(samplesPre);
        g.setColour(T_COL(waveform).withAlpha(0.3f));
        g.strokePath(prePath, juce::PathStrokeType(1.0f));
    }

    // 3. Draw Post (Wet)
    auto postPath = createWaveformPath(samplesPost);
    g.setColour(T_COL(waveform));
    g.strokePath(postPath, juce::PathStrokeType(1.5f));

    // subtle center line (0.0 amplitude -> normY = 0.5)
    float nyCenter = (0.5f - offsetY) / visibleHeightNorm;
    float yCenter = (1.0f - nyCenter) * height;
    g.setColour(T_COL(gridMinor));
    g.drawLine(0.0f, yCenter, (float)width, yCenter);
}

