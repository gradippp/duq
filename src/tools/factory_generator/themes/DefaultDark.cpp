#include "FactoryData.h"

namespace FactoryData {
    std::vector<FactoryAsset> getThemes() {
        return {
            { "Default_Dark.theme", R"json({
  "name": "Default Dark",
  "colours": {
    "background": "ff0f0f0f",
    "textMain": "ffffffff",
    "textDimmed": "ff888888",
    "accent": "ff3498db"
  }
})json" }
        };
    }
}
