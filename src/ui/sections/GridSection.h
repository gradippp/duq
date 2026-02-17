#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"
#include "../components/PointComponent.h"
#include "../components/AnchorComponent.h"

class GridSection : public juce::Component
{
public:
    GridSection();

    void setEnvelope(EnvelopeData*);

    juce::Point<float> normalizedToPixel(juce::Point<float>) const;
    juce::Point<float> pixelToNormalized(juce::Point<float>) const;

    void paint(juce::Graphics&) override;
    void resized() override;
    float getCurveForSegment(int index) const;

private:
    void drawGrid(juce::Graphics&);
    void rebuildPointComponents();
    void updatePointPositions();

    juce::Rectangle<int> viewArea;

    EnvelopeData* envelope = nullptr;

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;
};
