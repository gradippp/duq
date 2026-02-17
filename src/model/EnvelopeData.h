#pragma once

struct EnvelopePoint
{
    float x = 0.0f;
    float y = 0.0f;

    float curve = 0.0f;
};

struct EnvelopeViewState
{
    float zoomX = 1.0f;
    float zoomY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    int gridPower = 4; // 1/16
};

struct EnvelopeData
{
    juce::String name;
    int triggerNote = 60;

    double rate = 20.0;
    double depth = 100.0;
    double smooth = 0.0;

    bool rateIsFrequencyMode = true;

    EnvelopeViewState viewState;

    std::vector<EnvelopePoint> points =
    {
        {0.0f, 0.0f},
        {1.0f, 1.0f}
    };
};