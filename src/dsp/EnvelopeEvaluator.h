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

        // Map 0..1 to -1..1
        float ten = std::max(-1.0f, std::min(1.0f, (curve - 0.5f) * 2.0f));

        if (std::abs(ten) < 0.001f)
            return t;

        // gate12-style power formula
        float pwr = std::pow(1.1f, std::abs(ten * 50.0f));

        if (type == CurveType::Exponential)
        {
            if (ten >= 0)
                return std::pow(t, pwr);
            else
                return 1.0f - std::pow(1.0f - t, pwr);
        }
        
        if (type == CurveType::Logarithmic)
        {
            // Inverse of exponential logic
            if (ten >= 0)
                return 1.0f - std::pow(1.0f - t, pwr);
            else
                return std::pow(t, pwr);
        }

        if (type == CurveType::SCurve)
        {
            if (t < 0.5f)
            {
                float t2 = t * 2.0f;
                if (ten >= 0)
                    return std::pow(t2, pwr) * 0.5f;
                else
                    return (1.0f - std::pow(1.0f - t2, pwr)) * 0.5f;
            }
            else
            {
                float t2 = (t - 0.5f) * 2.0f;
                if (ten >= 0)
                    return 0.5f + (1.0f - std::pow(1.0f - t2, pwr)) * 0.5f;
                else
                    return 0.5f + std::pow(t2, pwr) * 0.5f;
            }
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
        
        // Safety: ensure lastIndex is valid for current points
        if (segmentIndex >= env.points.size() - 1)
            segmentIndex = 0;

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
