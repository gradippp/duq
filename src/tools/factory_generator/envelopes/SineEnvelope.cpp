#include "FactoryData.h"
#include "model/EnvelopeData.h"
#include <cmath>

namespace FactoryData {
    FactoryAsset getSineEnvelope() {
        EnvelopeShape shape;
        
        // Approximate a sine wave with points
        const int numPoints = 16;
        for (int i = 0; i <= numPoints; ++i) {
            float x = (float)i / (float)numPoints;
            // Sine wave from 0 to 1 range: (sin(2*PI*x) + 1) / 2
            float y = (std::sin(2.0f * 3.14159265f * x) + 1.0f) * 0.5f;
            shape.addPoint(x, y);
        }
        
        auto xml = shape.toValueTree().createXml();
        return { "Sine_Wave.env", xml->toString().toStdString() };
    }
}
