#include "FactoryData.h"
#include "model/EnvelopeData.h"

namespace FactoryData {
    FactoryAsset getDefaultEnvelope() {
        EnvelopeShape shape;
        shape.addPoint(0.0f, 0.0f);
        shape.addPoint(1.0f, 1.0f);
        
        auto xml = shape.toValueTree().createXml();
        return { "Default.duq.env", xml->toString().toStdString() };
    }
}
