#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

class ThemeManager : public juce::ChangeBroadcaster
{
public:
    enum ColourID
    {
        background,
        sectionBackground,
        headerBackground,
        border,
        accent,
        gridMajor,
        gridMinor,
        waveform,
        envelopeLine,
        envelopeFillTop,
        envelopeFillBot,
        point,
        anchor,
        playhead,
        playheadGlow,
        textMain,
        textDimmed,
        textLabel,
        uiHover,
        uiSelected,
        uiDisabledOverlay,
        midiIndicator,
        danger,
        meterBackground,
        meterFill,
        meterReduction,
        knobTrack,
        knobIndicator,
        knobAccent,
        knobShadow,
        presetBrowserBackground,
        presetBrowserFooter,
        presetBrowserFooterLine,
        contextMenuBackground,
        contextMenuText,
        contextMenuHighlight,
        contextMenuBorder,
        sidechain,
        widgetBackground,
        widgetOutline,
        widgetText,
        widgetTick
    };

    static ThemeManager& getInstance();

    juce::Colour getColour(ColourID id) const;
    void setColour(ColourID id, juce::Colour colour);

    void loadDefaultTheme();
    bool loadThemeFromFile(const juce::File& file);
    bool saveThemeToFile(const juce::File& file);
    juce::String saveThemeToXmlString() const;

    juce::String getColourName(ColourID id) const;
    juce::String getColourDescription(ColourID id) const;
    ColourID getIDFromName(const juce::String& name) const;

    static std::vector<ColourID> getAllIDs();

private:
    ThemeManager();
    ~ThemeManager() override = default;

    std::map<ColourID, juce::Colour> colours;
    
    void initializeDefaultColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};
