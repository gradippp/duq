#pragma once
#include <string>
#include <vector>

struct FactoryAsset {
    std::string filename;
    std::string content;
};

namespace FactoryData {
    std::vector<FactoryAsset> getThemes();
    std::vector<FactoryAsset> getEnvelopes();
}
