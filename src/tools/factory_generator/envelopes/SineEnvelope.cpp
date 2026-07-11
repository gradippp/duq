#include "FactoryData.h"
#include "FactoryEnvelopeData.h"
#include <cmath>

namespace FactoryData {
    FactoryAsset getSineEnvelope() {
        EnvelopeData data;
        data.name = "Sine Wave";
        
        // Approximate a sine wave with points
        const int numPoints = 16;
        for (int i = 0; i <= numPoints; ++i) {
            float x = (float)i / (float)numPoints;
            // Sine wave from 0 to 1 range: (sin(2*PI*x) + 1) / 2
            float y = (std::sin(2.0f * 3.14159265f * x) + 1.0f) * 0.5f;
            data.shape.addPoint(x, y);
        }
        
        auto xml = data.toValueTree().createXml();
        return { "Sine_Wave.duq.env", xml->toString().toStdString() };
    }
}
