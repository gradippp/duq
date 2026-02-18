#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GridSection;

class AnchorComponent : public juce::Component
{
public:
    AnchorComponent(GridSection& owner, juce::ValueTree node);

    void setNormalizedPosition(juce::Point<float>);

    std::function<void(juce::ValueTree)> onDragStart;
    std::function<void(juce::ValueTree, float)> onDragMove;
    std::function<void(juce::ValueTree)> onDragEnd;

private:
    GridSection& grid;
    juce::ValueTree point;     // Stable identity
    float startCurve = 0.0f;

    juce::Point<int> dragStartMouse;

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
};