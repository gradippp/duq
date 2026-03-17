#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace Theme
{
    namespace Colours
    {
        // Base
        const juce::Colour background      { 0xFF0A0A0A };
        const juce::Colour sectionBackground { 0xFF121212 };
        const juce::Colour headerBackground { 0xFF1A1A1A };
        const juce::Colour border          { 0xFF333333 };
        const juce::Colour accent          { 0xFFFFFFFF };
        
        // Grid & Graph
        const juce::Colour gridMajor       { accent.withAlpha (0.12f) };
        const juce::Colour gridMinor       { accent.withAlpha (0.04f) };
        const juce::Colour waveform        { juce::Colours::azure };
        
        // Envelope
        const juce::Colour envelopeLine    { accent };
        const juce::Colour envelopeFillTop { accent.withAlpha (0.15f) };
        const juce::Colour envelopeFillBot { accent.withAlpha (0.02f) };
        const juce::Colour point           { accent };
        const juce::Colour anchor          { accent.withAlpha (0.7f) };
        
        // Playhead
        const juce::Colour playhead        { accent.withAlpha (0.8f) };
        const juce::Colour playheadGlow    { accent.withAlpha (0.15f) };
        
        // Text
        const juce::Colour textMain        { accent };
        const juce::Colour textDimmed      { accent.withAlpha (0.5f) };
        const juce::Colour textLabel       { accent.withAlpha (0.7f) };
        
        // UI States
        const juce::Colour uiHover         { accent.withAlpha (0.15f) };
        const juce::Colour uiSelected      { accent.withAlpha (0.3f) };
        const juce::Colour uiDisabledOverlay { 0xFF000000 };
        const juce::Colour midiIndicator   { 0xFF32CD32 }; // LimeGreen
        const juce::Colour danger          { 0xFFFF4444 }; // Red

        // Meters
        const juce::Colour meterBackground { 0xFF000000 };
        const juce::Colour meterFill       { accent };
        const juce::Colour meterReduction  { 0xFFFF4444 };

        // Knobs
        const juce::Colour knobTrack       { 0xFF1A1A1A };
        const juce::Colour knobIndicator   { accent };
        const juce::Colour knobAccent      { accent };
        const juce::Colour knobShadow      { 0xFF000000 };

        // Preset Browser
        const juce::Colour presetBrowserBackground { 0xFF121212 };
        const juce::Colour presetBrowserFooter     { 0xFF000000 };
        const juce::Colour presetBrowserFooterLine { accent.withAlpha(0.1f) };

        // Context Menus
        const juce::Colour contextMenuBackground     { 0xFF1A1A1A };
        const juce::Colour contextMenuText           { accent.withAlpha(0.9f) };
        const juce::Colour contextMenuHighlight      { uiSelected };
        const juce::Colour contextMenuBorder         { border };
    }

    namespace Defaults
    {
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

        struct Point { float x, y; };
        const Point defaultPoints[] = { { 0.0f, 0.0f }, { 1.0f, 1.0f } };
        const int numDefaultPoints = 2;
    }
}
