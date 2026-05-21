#include "FactoryData.h"
#include "model/EnvelopeData.h"

namespace FactoryData {
    FactoryAsset getDefaultEnvelope() {
        EnvelopeData data;
        data.name = "Default";
        data.shape.addPoint(0.0f, 0.0f);
        data.shape.addPoint(1.0f, 1.0f);
        
        auto xml = data.toValueTree().createXml();
        return { "Default.duq.env", xml->toString().toStdString() };
    }
}
