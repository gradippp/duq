#include "FontManager.h"
#include "BinaryData.h"

std::map<const char*, juce::Typeface::Ptr> FontManager::typefaceCache;

juce::Typeface::Ptr FontManager::getTypeface(const char* resourceName)
{
    if (typefaceCache.count(resourceName) > 0)
        return typefaceCache[resourceName];

    int size = 0;
    const char* data = BinaryData::getNamedResource(resourceName, size);

    if (data == nullptr || size == 0)
        return nullptr;

    auto typeface = juce::Typeface::createSystemTypefaceFor(data, (size_t)size);
    typefaceCache[resourceName] = typeface;
    return typeface;
}

juce::Font FontManager::getInterRegular(float size)
{
    auto tf = getTypeface("Inter_Regular_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getInterMedium(float size)
{
    auto tf = getTypeface("Inter_Medium_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getInterBold(float size)
{
    auto tf = getTypeface("Inter_Bold_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getJetBrainsMono(float size)
{
    auto tf = getTypeface("JetBrainsMono_Medium_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getBarlowRegular(float size)
{
    auto tf = getTypeface("BarlowCondensed_Regular_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getBarlowSemiBold(float size)
{
    auto tf = getTypeface("BarlowCondensed_SemiBold_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}

juce::Font FontManager::getBarlowBold(float size)
{
    auto tf = getTypeface("BarlowCondensed_Bold_ttf");
    return tf != nullptr ? juce::Font (juce::FontOptions (tf).withHeight (size)) : juce::Font (juce::FontOptions (size));
}
