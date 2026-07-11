#pragma once

#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>

/**
    Shared envelope smoothing used by BOTH the DSP (PluginProcessor::syncToDSP)
    and the visual overlay (EnvelopePathRenderer), so the audio gain and the
    drawn "smoothness line" are identical by construction.

    The smoothing is a steady-state, phase-locked one-pole low-pass over the
    looped envelope shape: it warms up several cycles so the result is periodic
    (the value at phase 0 carries over from the end of the previous cycle),
    matching how a looping LFO smoother behaves.
*/
namespace EnvelopeSmoothing
{
    // Effective cycles-per-second used when deriving the smoothing coefficient.
    // Frequency mode: the raw Hz value. Sync mode: the rate knob (0..100) maps to
    // a division multiplier, matched to a 120 BPM placeholder (multiplier * 2.0).
    inline double effectiveCyclesPerSecond(double rawRate, bool isFreqMode)
    {
        if (isFreqMode)
            return rawRate;

        static const double cycleMultipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
        int idx = std::clamp((int)(rawRate / 16.66), 0, 5);
        return cycleMultipliers[idx] * 2.0;
    }

    // Fills 'out' with (steps + 1) samples of the steady-state smoothed shape over
    // phase [0, 1]. evaluateY(x) must return the raw envelope value (0..1) at phase x.
    // If smoothing is disabled (smoothMs <= 0 or rate <= 0), 'out' receives the raw
    // shape (identity), so callers can always sample 'out' by phase.
    inline void computeSteadyStateSmoothing(const std::function<float(float)>& evaluateY,
                                            float smoothMs,
                                            double effectiveRate,
                                            std::vector<float>& out,
                                            int steps = 400)
    {
        out.assign(static_cast<size_t>(steps) + 1, 0.0f);

        const float smoothTimeSec = smoothMs / 1000.0f;
        if (smoothTimeSec <= 0.0f || effectiveRate <= 0.0)
        {
            for (int i = 0; i <= steps; ++i)
                out[static_cast<size_t>(i)] = evaluateY((float)i / (float)steps);
            return;
        }

        const double durationSec = 1.0 / effectiveRate;
        const double stepsPerSec = (double)steps / durationSec;
        const float coeff = 1.0f - std::exp(-1.0f / (smoothTimeSec * (float)stepsPerSec));

        // Warm up so the response reaches steady state (periodic).
        float y = 1.0f;
        for (int cycle = 0; cycle < 10; ++cycle)
            for (int i = 0; i <= steps; ++i)
                y += (evaluateY((float)i / (float)steps) - y) * coeff;

        // out[0] holds the post-warm-up value; subsequent samples advance then store
        // (matching the renderer's draw pass so the line and audio align exactly).
        out[0] = y;
        for (int i = 1; i <= steps; ++i)
        {
            y += (evaluateY((float)i / (float)steps) - y) * coeff;
            out[static_cast<size_t>(i)] = y;
        }
    }

    // Linear interpolation over a phase-domain table produced by
    // computeSteadyStateSmoothing. Returns 1.0 (unity) for an empty table.
    inline float sampleShape(const std::vector<float>& shape, double phase)
    {
        const int n = (int)shape.size();
        if (n == 0) return 1.0f;
        if (n == 1) return shape[0];

        double p = std::clamp(phase, 0.0, 1.0) * (double)(n - 1);
        int i0 = (int)p;
        int i1 = std::min(i0 + 1, n - 1);
        float frac = (float)(p - (double)i0);
        return shape[static_cast<size_t>(i0)] + (shape[static_cast<size_t>(i1)] - shape[static_cast<size_t>(i0)]) * frac;
    }
}
