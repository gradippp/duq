#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    std::vector<FactoryAsset> getThemes() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme();
        
        return {
            { "Default_Dark.theme", tm.saveThemeToXmlString() }
        };
    }
}
