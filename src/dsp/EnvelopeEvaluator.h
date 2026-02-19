#pragma once

#include "EnvelopeProcessor.h"
#include <cmath>
#include <algorithm>

namespace EnvelopeEvaluator
{
    inline float applyCurve(float t, float curve, CurveType type)
    {
        if (type == CurveType::Linear)
            return t;

        if (type == CurveType::Step)
            return (t >= 0.5f) ? 1.0f : 0.0f;

        // Use 0.5 as neutral (linear). Map 0..1 to -1..1
        float norm = std::max(-1.0f, std::min(1.0f, (curve - 0.5f) * 2.0f));

        if (std::abs(norm) < 0.001f)
            return t;

        float k = norm * 4.0f; // scale aggression

        if (type == CurveType::Exponential)
        {
            if (norm > 0)
                return 1.0f - std::pow(1.0f - t, 1.0f + k);
            else
                return std::pow(t, 1.0f - k);
        }
        
        if (type == CurveType::Logarithmic)
        {
            if (norm > 0)
                return std::pow(t, 1.0f + k);
            else
                return 1.0f - std::pow(1.0f - t, 1.0f - k);
        }

        if (type == CurveType::SCurve)
        {
            float s = 1.0f / (1.0f + std::exp(-k * (t - 0.5f) * 10.0f));
            float s0 = 1.0f / (1.0f + std::exp(-k * (-0.5f) * 10.0f));
            float s1 = 1.0f / (1.0f + std::exp(-k * (0.5f) * 10.0f));
            return (s - s0) / (s1 - s0);
        }

        return t;
    }

    inline float evaluate(const DSPEnvelope& env, double phase, size_t& lastIndex)
    {
        if (env.points.empty())
            return 1.0f;

        if (env.points.size() == 1)
            return env.points[0].y;

        // Clamp phase to 0..1 for one-shot
        phase = std::max(0.0, std::min(1.0, phase));

        // Find segment starting from lastIndex or search
        size_t segmentIndex = lastIndex;
        
        // If phase moved backwards or jump, reset search
        if (phase < (double)env.points[segmentIndex].x)
            segmentIndex = 0;

        for (size_t i = segmentIndex; i < env.points.size() - 1; ++i)
        {
            if (phase >= (double)env.points[i].x && phase <= (double)env.points[i + 1].x)
            {
                segmentIndex = i;
                break;
            }
        }

        lastIndex = segmentIndex;

        if (segmentIndex >= env.segments.size())
            return env.points.back().y;

        const auto& p1 = env.points[segmentIndex];
        const auto& p2 = env.points[segmentIndex + 1];
        const auto& s = env.segments[segmentIndex];

        float duration = p2.x - p1.x;
        if (duration <= 0.00001f)
            return p2.y;

        float t = (float)(phase - (double)p1.x) / duration;
        float shapedT = applyCurve(t, s.curve, s.type);

        return p1.y + (p2.y - p1.y) * shapedT;
    }
}
