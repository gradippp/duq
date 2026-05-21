#include "FactoryData.h"
#include "model/EnvelopeData.h"

namespace FactoryData {
    FactoryAsset getSidechainEnvelope() {
        EnvelopeShape shape;
        shape.addPoint(0.0f, 0.0f);
        
        // Classic kick sidechain curve
        // Start at 0, quick rise to 1.0
        auto* p = shape.addPoint(0.25f, 1.0f);
        if (p) p->tension = 0.7f; // Add some tension for a smoother curve if your model supports it
        
        shape.addPoint(1.0f, 1.0f);
        
        auto xml = shape.toValueTree().createXml();
        return { "Kick_Sidechain.env", xml->toString().toStdString() };
    }
}
