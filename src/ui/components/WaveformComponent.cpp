#include "WaveformComponent.h"
#include "../../Globals.h"

WaveformComponent::WaveformComponent()
{
    setOpaque(false);
    setInterceptsMouseClicks(false, false);
}

WaveformComponent::~WaveformComponent()
{
}

void WaveformComponent::setSampleBuffers(
    const std::atomic<int>* writePos,
    std::span<const float> preData,
    std::span<const float> postData,
    std::span<const float> sidechainData)
{
    writePosition = writePos;
    samplesPre = preData;
    samplesPost = postData;
    samplesSidechain = sidechainData;
    bufferLength = (int)preData.size();
    cacheDirty = true;
}

void WaveformComponent::setViewState(float zx, float ox, float zy, float oy)
{
    zoomX = zx;
    offsetX = ox;
    zoomY = zy;
    offsetY = oy;
    cacheDirty = true;
    repaint();
}

void WaveformComponent::onFrameTick()
{
    if (isVisible())
        repaint();
}

void WaveformComponent::rebuildCache(int quality)
{
    cacheDirty = false;
    cachedSidechainPath.clear();
    cachedPrePath.clear();
    cachedPostPath.clear();

    if (samplesPre.empty() || samplesPost.empty() || bufferLength <= 0 || getWidth() <= 0 || getHeight() <= 0)
        return;

    const int width = getWidth();
    const int height = getHeight();
    const float visibleWidthNorm = 1.0f / zoomX;
    const float visibleHeightNorm = 1.0f / zoomY;
    const float startSample = offsetX * bufferLength;
    const float visibleSamples = visibleWidthNorm * bufferLength;
    const float samplesPerPixel = visibleSamples / (float)width;

    auto createWaveformPath = [&](std::span<const float> data) -> juce::Path
    {
        juce::Path path;
        if (data.empty()) return path;

        int step = 1;
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
                float v = data[(size_t)i];
                minVal = std::min(minVal, v);
                maxVal = std::max(maxVal, v);
            }

            if (minVal > maxVal)
                continue;

            float normYTop = (maxVal + 1.0f) * 0.5f;
            float normYBottom = (minVal + 1.0f) * 0.5f;
            float nyTop = (normYTop - offsetY) / visibleHeightNorm;
            float nyBottom = (normYBottom - offsetY) / visibleHeightNorm;
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

    cachedSidechainPath = (!samplesSidechain.empty() && config->getShowSidechainSignal()) ? createWaveformPath(samplesSidechain) : juce::Path();
    cachedPrePath = config->getShowSourceSignal() ? createWaveformPath(samplesPre) : juce::Path();
    cachedPostPath = createWaveformPath(samplesPost);

    lastWritePosition = writePosition != nullptr ? writePosition->load(std::memory_order_relaxed) : -1;
    lastBufferLength = bufferLength;
    lastWidth = width;
    lastHeight = height;
    lastZoomX = zoomX;
    lastOffsetX = offsetX;
    lastZoomY = zoomY;
    lastOffsetY = offsetY;
    lastQuality = quality;
    lastShowSidechain = config->getShowSidechainSignal();
    lastShowSource = config->getShowSourceSignal();
}

void WaveformComponent::paint(juce::Graphics& g)
{
    if (samplesPre.empty() || samplesPost.empty() || bufferLength <= 0)
        return;

    int quality = config->getWaveformQuality();
    int writePos = writePosition != nullptr ? writePosition->load(std::memory_order_relaxed) : -1;

    // "Real" changes (zoom/pan/size/quality/config) must rebuild immediately.
    bool structuralChange = cacheDirty
        || bufferLength != lastBufferLength
        || getWidth() != lastWidth
        || getHeight() != lastHeight
        || zoomX != lastZoomX
        || offsetX != lastOffsetX
        || zoomY != lastZoomY
        || offsetY != lastOffsetY
        || quality != lastQuality
        || config->getShowSidechainSignal() != lastShowSidechain
        || config->getShowSourceSignal() != lastShowSource;

    // Playback advances writePos every block, which would rebuild the full
    // min/max scan + 3 Paths at 60Hz. Only rebuild for playback motion once the
    // write head has moved at least ~1 pixel, and cap that path to ~30fps.
    bool motionChange = false;
    if (!structuralChange && writePos != lastWritePosition && bufferLength > 0)
    {
        int delta = writePos - lastWritePosition;
        if (delta < 0) delta += bufferLength;         // ring-aware
        float samplesPerPixel = (getWidth() > 0)
            ? ((1.0f / zoomX) * (float)bufferLength) / (float)getWidth()
            : (float)bufferLength;
        double now = juce::Time::getMillisecondCounterHiRes();
        if ((float)delta >= samplesPerPixel && (now - lastRebuildMs) >= 33.0)
        {
            motionChange = true;
            lastRebuildMs = now;
        }
    }

    if (structuralChange || motionChange)
        rebuildCache(quality);

    // 1. Draw Sidechain (Background)
    if (!cachedSidechainPath.isEmpty())
    {
        g.setColour(Theme::get(ThemeManager::sidechain).withAlpha(1.0f)); 
        g.strokePath(cachedSidechainPath, juce::PathStrokeType(1.2f));
    }

    // 2. Draw Pre (Dry)
    if (!cachedPrePath.isEmpty())
    {
        g.setColour(Theme::get(ThemeManager::waveform).withAlpha(0.3f));
        g.strokePath(cachedPrePath, juce::PathStrokeType(1.0f));
    }

    // 3. Draw Post (Wet)
    g.setColour(Theme::get(ThemeManager::waveform));
    g.strokePath(cachedPostPath, juce::PathStrokeType(1.5f));

    // subtle center line (0.0 amplitude -> normY = 0.5)
    const int height = getHeight();
    const int width = getWidth();
    const float visibleHeightNorm = 1.0f / zoomY;
    float nyCenter = (0.5f - offsetY) / visibleHeightNorm;
    float yCenter = (1.0f - nyCenter) * height;
    g.setColour(Theme::get(ThemeManager::gridMinor));
    g.drawLine(0.0f, yCenter, (float)width, yCenter);
}

