#include "FactoryData.h"
#include "model/EnvelopeData.h"

namespace FactoryData {
    std::vector<FactoryAsset> getEnvelopes() {
        EnvelopeShape pluck;
        pluck.addPoint(0.0f, 1.0f);
        pluck.addPoint(0.1f, 0.0f);
        
        auto xml = pluck.toValueTree().createXml();
        
        std::vector<FactoryAsset> assets;
        assets.push_back({ "Basic_Pluck.env", xml->toString().toStdString() });
        return assets;
    }
}
