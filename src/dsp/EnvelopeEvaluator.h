#pragma once

#include "EnvelopeCurves.h"
#include <cmath>
#include <algorithm>

namespace EnvelopeEvaluator
{
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
        float shapedT = EnvelopeCurves::applyCurve(t, s.curve, s.type);

        return p1.y + (p2.y - p1.y) * shapedT;
    }
}
