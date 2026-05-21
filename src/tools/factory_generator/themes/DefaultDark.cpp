#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    FactoryAsset getDefaultDarkTheme() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme();
        
        return { "Default_Dark.duq.theme", tm.saveThemeToXmlString().toStdString() };
    }
}
