#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

class FontManager
{
public:
    static juce::Font getInterRegular(float size);
    static juce::Font getInterMedium(float size);
    static juce::Font getInterBold(float size);
    static juce::Font getJetBrainsMono(float size);
    static juce::Font getBarlowRegular(float size);
    static juce::Font getBarlowSemiBold(float size);
    static juce::Font getBarlowBold(float size);

private:
    static juce::Typeface::Ptr getTypeface(const char* resourceName);
    static std::map<const char*, juce::Typeface::Ptr> typefaceCache;
};
