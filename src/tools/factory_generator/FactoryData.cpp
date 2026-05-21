#include "FactoryData.h"

namespace FactoryData {
    std::vector<FactoryAsset> getAllThemes() {
        std::vector<FactoryAsset> assets;
        assets.push_back(getDefaultDarkTheme());
        assets.push_back(getLightTheme());
        assets.push_back(getRedTheme());
        return assets;
    }

    std::vector<FactoryAsset> getAllEnvelopes() {
        std::vector<FactoryAsset> assets;
        assets.push_back(getDefaultEnvelope());
        assets.push_back(getSidechainEnvelope());
        assets.push_back(getSineEnvelope());
        return assets;
    }
}
