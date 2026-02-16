#pragma once

struct EnvelopePoint
{
    float x = 0.0f;
    float y = 0.0f;

    float curve = 0.0f;
};


struct EnvelopeData
{
    double rate = 20.0;
    double depth = 100.0;
    double smooth = 0.0;

    bool rateIsFrequencyMode = true;

    std::vector<EnvelopePoint> points =
    {
        {0.0f, 0.0f},
        {1.0f, 1.0f}
    };
};
