#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GridSection;

class AnchorComponent : public juce::Component
{
public:
    AnchorComponent(GridSection& owner, int segmentIndex);

    void setNormalizedPosition(juce::Point<float> p);

    std::function<void(int)> onDragStart;
    std::function<void(int, float)> onDragMove;
    std::function<void(int)> onDragEnd;

    void paint(juce::Graphics&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent& e);
    void mouseUp(const juce::MouseEvent&) override;

private:
    GridSection& grid;
    int segmentIndex;

    float startCurve = 0.0f;
    int dragStartY = 0;
};
