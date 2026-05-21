#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    FactoryAsset getLightTheme() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme(); // Reset first to ensure all fields are populated
        
        const juce::Colour accentColour(0xFFD32F2F); // Red accent for light theme
        const juce::Colour borderColour(0xFFDDDDDD);
        const juce::Colour textMainColour(0xFF111111);

        tm.setColour(ThemeManager::background, juce::Colour(0xFFF2F2F2));
        tm.setColour(ThemeManager::sectionBackground, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::headerBackground, juce::Colour(0xFFEAEAEA));
        tm.setColour(ThemeManager::border, borderColour);
        tm.setColour(ThemeManager::accent, accentColour);

        tm.setColour(ThemeManager::gridMajor, juce::Colour(0xFF000000).withAlpha(0.12f));
        tm.setColour(ThemeManager::gridMinor, juce::Colour(0xFF000000).withAlpha(0.06f));
        tm.setColour(ThemeManager::waveform, juce::Colour(0xFF444444));
        tm.setColour(ThemeManager::sidechain, juce::Colour(0xFFD35400)); 

        tm.setColour(ThemeManager::envelopeLine, accentColour);
        tm.setColour(ThemeManager::envelopeFillTop, accentColour.withAlpha(0.2f));
        tm.setColour(ThemeManager::envelopeFillBot, accentColour.withAlpha(0.05f));
        tm.setColour(ThemeManager::point, accentColour);
        tm.setColour(ThemeManager::anchor, accentColour.withAlpha(0.8f));

        tm.setColour(ThemeManager::playhead, accentColour.withAlpha(0.9f));
        tm.setColour(ThemeManager::playheadGlow, accentColour.withAlpha(0.2f));

        tm.setColour(ThemeManager::textMain, juce::Colour(0xFF000000));
        tm.setColour(ThemeManager::textDimmed, juce::Colour(0xFF000000).withAlpha(0.6f));
        tm.setColour(ThemeManager::textLabel, juce::Colour(0xFF000000));

        tm.setColour(ThemeManager::uiHover, juce::Colour(0xFF000000).withAlpha(0.1f));
        tm.setColour(ThemeManager::uiSelected, accentColour.withAlpha(0.25f));
        tm.setColour(ThemeManager::uiDisabledOverlay, juce::Colour(0xFF000000).withAlpha(0.35f));
        
        tm.setColour(ThemeManager::midiIndicator, juce::Colour(0xFF27AE60));
        tm.setColour(ThemeManager::danger, juce::Colour(0xFFC0392B));

        tm.setColour(ThemeManager::meterBackground, juce::Colour(0xFFE0E0E0));
        tm.setColour(ThemeManager::meterFill, accentColour);
        tm.setColour(ThemeManager::meterReduction, juce::Colour(0xFFE74C3C));

        tm.setColour(ThemeManager::knobTrack, juce::Colour(0xFFDDDDDD));
        tm.setColour(ThemeManager::knobIndicator, accentColour);
        tm.setColour(ThemeManager::knobAccent, accentColour);
        tm.setColour(ThemeManager::knobShadow, juce::Colour(0x05000000));

        tm.setColour(ThemeManager::presetBrowserBackground, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::presetBrowserFooter, juce::Colour(0xFFF9F9F9));
        tm.setColour(ThemeManager::presetBrowserFooterLine, borderColour);

        tm.setColour(ThemeManager::contextMenuBackground, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::contextMenuText, juce::Colour(0xFF000000));
        tm.setColour(ThemeManager::contextMenuHighlight, accentColour.withAlpha(0.15f));
        tm.setColour(ThemeManager::contextMenuBorder, borderColour);

        tm.setColour(ThemeManager::widgetBackground, juce::Colour(0xFFF4F4F4));
        tm.setColour(ThemeManager::widgetOutline, juce::Colour(0xFFC0C0C0));
        tm.setColour(ThemeManager::widgetText, juce::Colour(0xFF000000));
        tm.setColour(ThemeManager::widgetTick, juce::Colour(0xFF000000));

        return { "Light.duq.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
