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
    void deletePoint(int index);

    juce::Point<float> normalizedToPixel(juce::Point<float>) const;
    juce::Point<float> pixelToNormalized(juce::Point<float>) const;

    void paint(juce::Graphics&) override;
    void resized() override;
    float getCurveForSegment(int index) const;

    void mouseDoubleClick(const juce::MouseEvent&) override;

private:
    int gridLines = 8;

    void drawGrid(juce::Graphics&);
    void rebuildPointComponents();
    void updatePointPositions();

    juce::Rectangle<int> viewArea;

    static float snapValue(float value, float step);
    float snapStepX = 0.05f;  // 5% horizontal grid
    float snapStepY = 0.05f;  // 5% vertical grid

    EnvelopeData* envelope = nullptr;

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;
};
