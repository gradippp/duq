#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/utils/ThemeManager.h"

namespace Theme
{
    inline juce::Colour get(ThemeManager::ColourID id) { return ThemeManager::getInstance().getColour(id); }
}

namespace Defaults
{
    constexpr int maxEnvelopeSlots = 12;

    const double rate = 2.0;
    const float depth = 100.0f;
    const float smooth = 0.0f;
    const bool rateIsFrequencyMode = true;
    const int triggerNote = 36;
    
    const float lookahead = 0.0f;
    
    const float curve = 0.5f;
    const int curveType = 0; // CurveType::Exponential

    const int gridPower = 4;
    const float zoom = 1.0f;
    const float offset = 0.0f;

    const juce::String envelopeName = "Env";
    const juce::String projectName = "INIT";

    struct Point { float x, y; };
    const Point defaultPoints[] = { { 0.0f, 0.0f }, { 1.0f, 1.0f } };
    const int numDefaultPoints = 2;
}
