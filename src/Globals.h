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

        // Meters
        const juce::Colour meterBackground { 0xFF000000 };
        const juce::Colour meterFill       { accent };
        const juce::Colour meterReduction  { 0xFFFF4444 };
    }
}
