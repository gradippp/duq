#include "ThemeManager.h"

ThemeManager& ThemeManager::getInstance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
{
    initializeDefaultColours();
}

juce::Colour ThemeManager::getColour(ColourID id) const
{
    auto it = colours.find(id);
    if (it != colours.end())
        return it->second;
    
    return juce::Colours::black;
}

void ThemeManager::setColour(ColourID id, juce::Colour colour)
{
    colours[id] = colour;
    sendChangeMessage();
}

void ThemeManager::initializeDefaultColours()
{
    const juce::Colour accentColour = juce::Colour(0xFFFFFFFF);
    const juce::Colour borderColour = juce::Colour(0xFF333333);
    const juce::Colour uiSelectedColour = accentColour.withAlpha(0.3f);

    colours[background] = juce::Colour(0xFF0A0A0A);
    colours[sectionBackground] = juce::Colour(0xFF121212);
    colours[headerBackground] = juce::Colour(0xFF1A1A1A);
    colours[border] = borderColour;
    colours[accent] = accentColour;

    colours[gridMajor] = accentColour.withAlpha(0.12f);
    colours[gridMinor] = accentColour.withAlpha(0.04f);
    colours[waveform] = juce::Colours::azure;

    colours[envelopeLine] = accentColour;
    colours[envelopeFillTop] = accentColour.withAlpha(0.15f);
    colours[envelopeFillBot] = accentColour.withAlpha(0.02f);
    colours[point] = accentColour;
    colours[anchor] = accentColour.withAlpha(0.7f);

    colours[playhead] = accentColour.withAlpha(0.8f);
    colours[playheadGlow] = accentColour.withAlpha(0.15f);

    colours[textMain] = accentColour;
    colours[textDimmed] = accentColour.withAlpha(0.5f);
    colours[textLabel] = accentColour.withAlpha(0.7f);

    colours[uiHover] = accentColour.withAlpha(0.15f);
    colours[uiSelected] = uiSelectedColour;
    colours[uiDisabledOverlay] = juce::Colours::black;
    colours[midiIndicator] = juce::Colour(0xFF32CD32);
    colours[danger] = juce::Colour(0xFFFF4444);

    colours[meterBackground] = juce::Colours::black;
    colours[meterFill] = accentColour;
    colours[meterReduction] = juce::Colour(0xFFFF4444);

    colours[knobTrack] = juce::Colour(0xFF1A1A1A);
    colours[knobIndicator] = accentColour;
    colours[knobAccent] = accentColour;
    colours[knobShadow] = juce::Colours::black;

    colours[presetBrowserBackground] = juce::Colour(0xFF121212);
    colours[presetBrowserFooter] = juce::Colours::black;
    colours[presetBrowserFooterLine] = accentColour.withAlpha(0.1f);

    colours[contextMenuBackground] = juce::Colour(0xFF1A1A1A);
    colours[contextMenuText] = accentColour.withAlpha(0.9f);
    colours[contextMenuHighlight] = uiSelectedColour;
    colours[contextMenuBorder] = borderColour;
}

void ThemeManager::loadDefaultTheme()
{
    initializeDefaultColours();
    sendChangeMessage();
}

bool ThemeManager::loadThemeFromFile(const juce::File& file)
{
    std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName("THEME"))
        return false;

    for (int i = 0; i < xml->getNumChildElements(); ++i)
    {
        auto* child = xml->getChildElement(i);
        if (child->hasTagName("COLOUR"))
        {
            auto name = child->getStringAttribute("id");
            auto colorHex = child->getStringAttribute("hex");
            
            ColourID id = getIDFromName(name);
            if (id != (ColourID)-1)
            {
                colours[id] = juce::Colour::fromString(colorHex);
            }
        }
    }

    sendChangeMessage();
    return true;
}

bool ThemeManager::saveThemeToFile(const juce::File& file)
{
    juce::XmlElement xml("THEME");

    for (auto const& [id, color] : colours)
    {
        auto* child = xml.createNewChildElement("COLOUR");
        child->setAttribute("id", getColourName(id));
        child->setAttribute("hex", color.toDisplayString(true));
    }

    return xml.writeTo(file);
}

juce::String ThemeManager::getColourName(ColourID id) const
{
    switch (id)
    {
        case background: return "background";
        case sectionBackground: return "sectionBackground";
        case headerBackground: return "headerBackground";
        case border: return "border";
        case accent: return "accent";
        case gridMajor: return "gridMajor";
        case gridMinor: return "gridMinor";
        case waveform: return "waveform";
        case envelopeLine: return "envelopeLine";
        case envelopeFillTop: return "envelopeFillTop";
        case envelopeFillBot: return "envelopeFillBot";
        case point: return "point";
        case anchor: return "anchor";
        case playhead: return "playhead";
        case playheadGlow: return "playheadGlow";
        case textMain: return "textMain";
        case textDimmed: return "textDimmed";
        case textLabel: return "textLabel";
        case uiHover: return "uiHover";
        case uiSelected: return "uiSelected";
        case uiDisabledOverlay: return "uiDisabledOverlay";
        case midiIndicator: return "midiIndicator";
        case danger: return "danger";
        case meterBackground: return "meterBackground";
        case meterFill: return "meterFill";
        case meterReduction: return "meterReduction";
        case knobTrack: return "knobTrack";
        case knobIndicator: return "knobIndicator";
        case knobAccent: return "knobAccent";
        case knobShadow: return "knobShadow";
        case presetBrowserBackground: return "presetBrowserBackground";
        case presetBrowserFooter: return "presetBrowserFooter";
        case presetBrowserFooterLine: return "presetBrowserFooterLine";
        case contextMenuBackground: return "contextMenuBackground";
        case contextMenuText: return "contextMenuText";
        case contextMenuHighlight: return "contextMenuHighlight";
        case contextMenuBorder: return "contextMenuBorder";
    }
    return "unknown";
}

ThemeManager::ColourID ThemeManager::getIDFromName(const juce::String& name) const
{
    if (name == "background") return background;
    if (name == "sectionBackground") return sectionBackground;
    if (name == "headerBackground") return headerBackground;
    if (name == "border") return border;
    if (name == "accent") return accent;
    if (name == "gridMajor") return gridMajor;
    if (name == "gridMinor") return gridMinor;
    if (name == "waveform") return waveform;
    if (name == "envelopeLine") return envelopeLine;
    if (name == "envelopeFillTop") return envelopeFillTop;
    if (name == "envelopeFillBot") return envelopeFillBot;
    if (name == "point") return point;
    if (name == "anchor") return anchor;
    if (name == "playhead") return playhead;
    if (name == "playheadGlow") return playheadGlow;
    if (name == "textMain") return textMain;
    if (name == "textDimmed") return textDimmed;
    if (name == "textLabel") return textLabel;
    if (name == "uiHover") return uiHover;
    if (name == "uiSelected") return uiSelected;
    if (name == "uiDisabledOverlay") return uiDisabledOverlay;
    if (name == "midiIndicator") return midiIndicator;
    if (name == "danger") return danger;
    if (name == "meterBackground") return meterBackground;
    if (name == "meterFill") return meterFill;
    if (name == "meterReduction") return meterReduction;
    if (name == "knobTrack") return knobTrack;
    if (name == "knobIndicator") return knobIndicator;
    if (name == "knobAccent") return knobAccent;
    if (name == "knobShadow") return knobShadow;
    if (name == "presetBrowserBackground") return presetBrowserBackground;
    if (name == "presetBrowserFooter") return presetBrowserFooter;
    if (name == "presetBrowserFooterLine") return presetBrowserFooterLine;
    if (name == "contextMenuBackground") return contextMenuBackground;
    if (name == "contextMenuText") return contextMenuText;
    if (name == "contextMenuHighlight") return contextMenuHighlight;
    if (name == "contextMenuBorder") return contextMenuBorder;
    return (ColourID)-1;
}

std::vector<ThemeManager::ColourID> ThemeManager::getAllIDs()
{
    return {
        background, sectionBackground, headerBackground, border, accent,
        gridMajor, gridMinor, waveform,
        envelopeLine, envelopeFillTop, envelopeFillBot, point, anchor,
        playhead, playheadGlow,
        textMain, textDimmed, textLabel,
        uiHover, uiSelected, uiDisabledOverlay, midiIndicator, danger,
        meterBackground, meterFill, meterReduction,
        knobTrack, knobIndicator, knobAccent, knobShadow,
        presetBrowserBackground, presetBrowserFooter, presetBrowserFooterLine,
        contextMenuBackground, contextMenuText, contextMenuHighlight, contextMenuBorder
    };
}
