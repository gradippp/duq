#pragma once

#include <algorithm>

/**
    Shared mapping for the "sync" rate mode, where the rate knob's 0..100 value
    selects a musical division (1/1 .. 1/32). Kept in one place so the DSP, the
    smoothing helper, and the UI display all agree.
*/
namespace RateUtils
{
    // Base cycle multipliers for the six sync divisions (relative to the beat).
    inline constexpr double syncCycleMultipliers[6] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };

    // Maps a 0..100 rate value to a division index 0..5.
    inline int syncDivisionIndex(double rate0to100)
    {
        return std::clamp((int)(rate0to100 / 16.66), 0, 5);
    }

    // Maps a 0..100 rate value to its base cycle multiplier.
    inline double syncDivisionMultiplier(double rate0to100)
    {
        return syncCycleMultipliers[syncDivisionIndex(rate0to100)];
    }
}
