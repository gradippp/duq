#pragma once

enum class CurveType
{
    Exponential = 0,
    Linear,
    Logarithmic,
    SCurve,
    Step
};

struct EnvelopePoint
{
    float x = 0.0f;
    float y = 0.0f;
};

struct EnvelopeSegment
{
    const EnvelopePoint* startPoint = nullptr;
    const EnvelopePoint* endPoint = nullptr;

    float curve = 0.5f;
    CurveType type = CurveType::Exponential;
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

    std::vector<EnvelopeSegment> segments;

    void rebuildSegments()
    {
        if (points.size() < 2)
        {
            segments.clear();
            return;
        }

        const size_t numSegments = points.size() - 1;

        // If the number of segments changed, we might need to reallocate/shift.
        // For simplicity, if it changed, we reset. 
        // In a real app we'd handle insertion/deletion more gracefully.
        if (segments.size() != numSegments)
        {
            segments.resize(numSegments);
        }

        for (size_t i = 0; i < numSegments; ++i)
        {
            segments[i].startPoint = &points[i];
            segments[i].endPoint = &points[i + 1];
        }
    }
};