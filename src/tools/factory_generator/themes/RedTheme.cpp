#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    FactoryAsset getRedTheme() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme(); // Start with base dark theme
        
        const juce::Colour redAccent(0xFFE53935);
        
        tm.setColour(ThemeManager::accent, redAccent);
        tm.setColour(ThemeManager::uiSelected, redAccent.withAlpha(0.3f));
        tm.setColour(ThemeManager::envelopeLine, redAccent);
        tm.setColour(ThemeManager::envelopeFillTop, redAccent.withAlpha(0.15f));
        tm.setColour(ThemeManager::envelopeFillBot, redAccent.withAlpha(0.02f));
        tm.setColour(ThemeManager::point, redAccent);
        tm.setColour(ThemeManager::anchor, redAccent.withAlpha(0.7f));
        tm.setColour(ThemeManager::playhead, redAccent.withAlpha(0.8f));
        tm.setColour(ThemeManager::playheadGlow, redAccent.withAlpha(0.15f));
        tm.setColour(ThemeManager::textMain, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::meterFill, redAccent);
        tm.setColour(ThemeManager::knobIndicator, redAccent);
        tm.setColour(ThemeManager::knobAccent, redAccent);
        tm.setColour(ThemeManager::contextMenuHighlight, redAccent.withAlpha(0.3f));
        
        tm.setColour(ThemeManager::widgetBackground, juce::Colour(0xFF000000));
        tm.setColour(ThemeManager::widgetOutline, redAccent.withAlpha(0.4f));
        tm.setColour(ThemeManager::widgetText, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::widgetTick, redAccent);

        return { "Red.duq.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
