#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../utils/ConfigManager.h"

class WaveformComponent : public juce::Component
{
public:
    WaveformComponent();
    ~WaveformComponent() override;

    // Driven by the editor's shared 60Hz frame timer (via GridSection).
    void onFrameTick();

    void setSampleBuffers(const std::atomic<int>* writePos,
        const float* preData,
        const float* postData,
        const float* sidechainData,
        int bufferSize);

    void setViewState(float zoomX, float offsetX, float zoomY = 1.0f, float offsetY = 0.0f);

    void paint(juce::Graphics& g) override;

private:
    const std::atomic<int>* writePosition = nullptr;
    const float* samplesPre = nullptr;
    const float* samplesPost = nullptr;
    const float* samplesSidechain = nullptr;
    int bufferLength = 0;

    float zoomX = 1.0f;
    float offsetX = 0.0f;
    float zoomY = 1.0f;
    float offsetY = 0.0f;

    void rebuildCache(int quality);

    juce::SharedResourcePointer<ConfigManager> config;

    int lastWritePosition = -1;
    int lastBufferLength = 0;
    int lastWidth = 0;
    int lastHeight = 0;
    float lastZoomX = 1.0f;
    float lastOffsetX = 0.0f;
    float lastZoomY = 1.0f;
    float lastOffsetY = 0.0f;
    int lastQuality = -1;
    bool lastShowSidechain = false;
    bool lastShowSource = false;
    bool cacheDirty = true;
    double lastRebuildMs = 0.0;

    juce::Path cachedSidechainPath;
    juce::Path cachedPrePath;
    juce::Path cachedPostPath;
};
