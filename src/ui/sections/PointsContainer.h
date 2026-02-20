#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GridBackground.h"
#include "../components/PointComponent.h"
#include "../components/AnchorComponent.h"

class GridSection;

class PointsContainer : public juce::Component
{
public:
    PointsContainer(GridSection& owner);

    void setEnvelope(juce::ValueTree newEnvelope);
    void setViewState(const GridViewState& newState);
    void updatePointPositions();
    void rebuildPointComponents();

    std::function<void(juce::ValueTree, juce::Point<float>)> onPointDrag;
    std::function<void(juce::ValueTree, float)> onAnchorDrag;
    std::function<void()> onDragEnd;

    const std::vector<std::unique_ptr<PointComponent>>& getPointComponents() const { return pointComponents; }
    const std::vector<std::unique_ptr<AnchorComponent>>& getAnchorComponents() const { return anchorComponents; }

private:
    GridSection& grid;
    GridViewState state;
    juce::ValueTree envelope;

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;
};
