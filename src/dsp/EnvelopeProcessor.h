#pragma once

#include <vector>
#include "../model/EnvelopeData.h"

struct DSPPoint
{
    float x = 0.0f;
    float y = 0.0f;
};

struct DSPSegment
{
    float curve = 0.5f;
    CurveType type = CurveType::Exponential;
};

struct DSPEnvelope
{
    std::vector<DSPPoint> points;
    std::vector<DSPSegment> segments;

    // Steady-state smoothed shape over phase [0,1] (empty when smooth == 0).
    // When present, the DSP samples this instead of evaluating the raw curve,
    // so the audio matches the on-screen smoothness line. Built in syncToDSP.
    std::vector<float> smoothedShape;

    double rate = 20.0;
    double phaseIncrement = 0.0;
    float depth = 1.0f;
    float smooth = 0.0f;
    float smoothCoeff = 1.0f;

    int triggerNote = 60;
    bool isFrequencyMode = true;
    bool isDisabled = false;
};

struct DSPState
{
    std::vector<DSPEnvelope> envelopes;
};

struct EnvelopeVoice
{
    int envelopeIndex = -1;
    double currentPhase = 0.0;
    bool isActive = false;
    int noteNumber = -1;
    float currentGain = 1.0f;
    size_t lastSegmentIndex = 0;

    // Block-rate interpolation
    float targetGain = 1.0f;
    float gainDelta = 0.0f;
};
