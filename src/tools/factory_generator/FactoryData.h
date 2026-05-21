#pragma once
#include <string>
#include <vector>

struct FactoryAsset {
    std::string filename;
    std::string content;
};

namespace FactoryData {
    FactoryAsset getDefaultDarkTheme();
    FactoryAsset getLightTheme();
    FactoryAsset getRedTheme();

    FactoryAsset getBasicPluckEnvelope();

    std::vector<FactoryAsset> getAllThemes();
    std::vector<FactoryAsset> getAllEnvelopes();
}
