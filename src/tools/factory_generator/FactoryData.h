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

    FactoryAsset getDefaultEnvelope();
    FactoryAsset getSidechainEnvelope();
    FactoryAsset getSineEnvelope();

    std::vector<FactoryAsset> getAllThemes();
    std::vector<FactoryAsset> getAllEnvelopes();
}
