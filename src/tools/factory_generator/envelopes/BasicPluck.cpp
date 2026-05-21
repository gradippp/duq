#include "FactoryData.h"

namespace FactoryData {
    std::vector<FactoryAsset> getEnvelopes() {
        return {
            { "Basic_Pluck.env", R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Envelope name="Basic Pluck">
  <Point x="0.0" y="1.0" curve="0" tension="0.5"/>
  <Point x="0.1" y="0.0" curve="0" tension="0.5"/>
</Envelope>)xml" }
        };
    }
}
