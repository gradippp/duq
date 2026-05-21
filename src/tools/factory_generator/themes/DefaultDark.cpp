#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    FactoryAsset getDefaultDarkTheme() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme();
        
        return { "Default_Dark.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
