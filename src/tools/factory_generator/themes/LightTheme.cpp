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

        tm.setColour(ThemeManager::gridMajor, juce::Colour(0xFF000000).withAlpha(0.08f));
        tm.setColour(ThemeManager::gridMinor, juce::Colour(0xFF000000).withAlpha(0.03f));
        tm.setColour(ThemeManager::waveform, juce::Colour(0xFF666666));
        tm.setColour(ThemeManager::sidechain, juce::Colour(0xFFE67E22)); 

        tm.setColour(ThemeManager::envelopeLine, accentColour);
        tm.setColour(ThemeManager::envelopeFillTop, accentColour.withAlpha(0.15f));
        tm.setColour(ThemeManager::envelopeFillBot, accentColour.withAlpha(0.02f));
        tm.setColour(ThemeManager::point, accentColour);
        tm.setColour(ThemeManager::anchor, accentColour.withAlpha(0.7f));

        tm.setColour(ThemeManager::playhead, accentColour.withAlpha(0.8f));
        tm.setColour(ThemeManager::playheadGlow, accentColour.withAlpha(0.15f));

        tm.setColour(ThemeManager::textMain, textMainColour);
        tm.setColour(ThemeManager::textDimmed, textMainColour.withAlpha(0.5f));
        tm.setColour(ThemeManager::textLabel, textMainColour.withAlpha(0.7f));

        tm.setColour(ThemeManager::uiHover, juce::Colour(0xFF000000).withAlpha(0.05f));
        tm.setColour(ThemeManager::uiSelected, accentColour.withAlpha(0.15f));
        tm.setColour(ThemeManager::uiDisabledOverlay, juce::Colour(0xFFFFFFFF).withAlpha(0.5f));
        
        tm.setColour(ThemeManager::midiIndicator, juce::Colour(0xFF27AE60));
        tm.setColour(ThemeManager::danger, juce::Colour(0xFFC0392B));

        tm.setColour(ThemeManager::meterBackground, juce::Colour(0xFFDDDDDD));
        tm.setColour(ThemeManager::meterFill, accentColour);
        tm.setColour(ThemeManager::meterReduction, juce::Colour(0xFFE74C3C));

        tm.setColour(ThemeManager::knobTrack, juce::Colour(0xFFEEEEEE));
        tm.setColour(ThemeManager::knobIndicator, accentColour);
        tm.setColour(ThemeManager::knobAccent, accentColour);
        tm.setColour(ThemeManager::knobShadow, juce::Colour(0x08000000));

        tm.setColour(ThemeManager::presetBrowserBackground, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::presetBrowserFooter, juce::Colour(0xFFF9F9F9));
        tm.setColour(ThemeManager::presetBrowserFooterLine, borderColour);

        tm.setColour(ThemeManager::contextMenuBackground, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::contextMenuText, textMainColour);
        tm.setColour(ThemeManager::contextMenuHighlight, accentColour.withAlpha(0.15f));
        tm.setColour(ThemeManager::contextMenuBorder, borderColour);

        return { "Light.duq.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
