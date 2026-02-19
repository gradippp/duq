#pragma once

#include "../model/EnvelopeData.h"
#include <cmath>
#include <algorithm>

namespace EnvelopeCurves
{
    /**
     * Common curve evaluation for both UI and DSP.
     * @param t Normalized time (0.0 to 1.0)
     * @param curve Normalized curve (0.0 to 1.0, 0.5 is linear)
     * @param type The curve formula to use
     * @return Shaped t value (0.0 to 1.0)
     */
    inline float applyCurve(float t, float curve, CurveType type)
    {
        if (type == CurveType::Linear)
            return t;

        if (type == CurveType::Step)
            return (t >= 0.5f) ? 1.0f : 0.0f;

        // Neutral (linear) is 0.5. Map 0..1 to -1..1
        float norm = (curve - 0.5f) * 2.0f;

        if (std::abs(norm) < 0.001f)
            return t;

        // aggression: 0.0 to 1.0 (magnitude)
        float absNorm = std::abs(norm);
        
        // Use a power of 2..10 for the curve range.
        // 1.0 + 9.0 * 1.0 = 10.0 (aggressive)
        // 1.0 + 9.0 * 0.0 = 1.0 (linear)
        float pwr = 1.0f + 7.0f * absNorm;

        if (type == CurveType::Exponential)
        {
            // norm > 0: Fast start (concave down)
            // norm < 0: Slow start (concave up)
            if (norm > 0)
                return 1.0f - std::pow(1.0f - t, pwr);
            else
                return std::pow(t, pwr);
        }
        
        if (type == CurveType::Logarithmic)
        {
            // Opposite of exponential logic
            if (norm > 0)
                return std::pow(t, pwr);
            else
                return 1.0f - std::pow(1.0f - t, pwr);
        }

        if (type == CurveType::SCurve)
        {
            if (t < 0.5f)
            {
                float t2 = t * 2.0f;
                if (norm > 0)
                    return (1.0f - std::pow(1.0f - t2, pwr)) * 0.5f;
                else
                    return std::pow(t2, pwr) * 0.5f;
            }
            else
            {
                float t2 = (t - 0.5f) * 2.0f;
                if (norm > 0)
                    return 0.5f + std::pow(t2, pwr) * 0.5f;
                else
                    return 0.5f + (1.0f - std::pow(1.0f - t2, pwr)) * 0.5f;
            }
        }

        return t;
    }
}
