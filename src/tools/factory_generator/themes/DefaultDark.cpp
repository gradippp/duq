#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    FactoryAsset getDefaultDarkTheme() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme();
        
        tm.setColour(ThemeManager::widgetBackground, juce::Colour(0xFF000000));
        tm.setColour(ThemeManager::widgetOutline, juce::Colour(0xFF333333));
        tm.setColour(ThemeManager::widgetText, juce::Colour(0xFFFFFFFF));
        tm.setColour(ThemeManager::widgetTick, juce::Colour(0xFFFFFFFF));

        return { "Default_Dark.duq.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
