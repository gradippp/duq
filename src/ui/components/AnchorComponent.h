#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"

class GridSection;

class AnchorComponent : public juce::Component, public juce::TooltipClient
{
public:
    AnchorComponent(GridSection& owner, juce::ValueTree node);

    juce::String getTooltip() override;

    void setNormalizedPosition(juce::Point<float>);

    std::function<void(juce::ValueTree)> onDragStart;
    std::function<void(juce::ValueTree, float)> onDragMove;
    std::function<void(juce::ValueTree)> onDragEnd;

private:
    void showTensionDialog();

    GridSection& grid;
    juce::ValueTree segment;     // Stable identity
    float startCurve = 0.5f;

    juce::Point<int> dragStartMouse;
    bool isDragging = false;

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
};
