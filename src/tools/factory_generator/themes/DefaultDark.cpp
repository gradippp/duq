#include "FactoryData.h"
#include "ui/utils/ThemeManager.h"

namespace FactoryData {
    std::vector<FactoryAsset> getThemes() {
        auto& tm = ThemeManager::getInstance();
        tm.loadDefaultTheme();
        
        std::vector<FactoryAsset> assets;
        assets.push_back({ "Default_Dark.theme", tm.saveThemeToXmlString().toStdString() });
        return assets;
    }
}
