#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace Theme
{
    namespace Colours
    {
        // Base
        inline const juce::Colour background      { 0xFF0A0A0A };
        inline const juce::Colour sectionBackground { 0xFF121212 };
        inline const juce::Colour headerBackground { 0xFF1A1A1A };
        inline const juce::Colour border          { 0xFF333333 };
        inline const juce::Colour accent          { 0xFFFFFFFF };
        
        // Grid & Graph
        inline const juce::Colour gridMajor       { accent.withAlpha (0.12f) };
        inline const juce::Colour gridMinor       { accent.withAlpha (0.04f) };
        inline const juce::Colour waveform        { juce::Colours::azure };
        
        // Envelope
        inline const juce::Colour envelopeLine    { accent };
        inline const juce::Colour envelopeFillTop { accent.withAlpha (0.15f) };
        inline const juce::Colour envelopeFillBot { accent.withAlpha (0.02f) };
        inline const juce::Colour point           { accent };
        inline const juce::Colour anchor          { accent.withAlpha (0.7f) };
        
        // Playhead
        inline const juce::Colour playhead        { accent.withAlpha (0.8f) };
        inline const juce::Colour playheadGlow    { accent.withAlpha (0.15f) };
        
        // Text
        inline const juce::Colour textMain        { accent };
        inline const juce::Colour textDimmed      { accent.withAlpha (0.5f) };
        inline const juce::Colour textLabel       { accent.withAlpha (0.7f) };
        
        // UI States
        inline const juce::Colour uiHover         { accent.withAlpha (0.15f) };
        inline const juce::Colour uiSelected      { accent.withAlpha (0.3f) };
        inline const juce::Colour uiDisabledOverlay { 0xFF000000 };
        inline const juce::Colour midiIndicator   { 0xFF32CD32 }; // LimeGreen

        // Meters
        inline const juce::Colour meterBackground { 0xFF000000 };
        inline const juce::Colour meterFill       { accent };
        inline const juce::Colour meterReduction  { 0xFFFF4444 };
    }
}
