#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace Icons
{
    // Loads an SVG from BinaryData by filename (without extension)
    std::unique_ptr<juce::Drawable> load(const juce::String& name);

    // Optional: load and apply colour tint
    std::unique_ptr<juce::Drawable> load(const juce::String& name,
        juce::Colour tint);
}
