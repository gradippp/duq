#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GridSection;

class PointComponent : public juce::Component, public juce::TooltipClient
{
public:
    PointComponent(GridSection& owner, juce::ValueTree pointNode);

    juce::String getTooltip() override;

    void setNormalizedPosition(juce::Point<float> p);
    juce::Point<float> getNormalizedPosition() const;

    // Identity-based callbacks (NOT index-based)
    std::function<void(juce::ValueTree)> onDragStart;
    std::function<void(juce::ValueTree,
        juce::Point<float>,
        bool)> onDragMove;
    std::function<void(juce::ValueTree)> onDragEnd;

    void paint(juce::Graphics&) override;

    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    void showPositionDialog();

    GridSection& grid;
    juce::ValueTree point;   // Stable identity
    juce::Point<float> normalized{ 0.0f, 0.0f };

    juce::Point<float> dragStartNormalized;
    juce::Point<int> dragStartMouse;

    bool isHovering = false;
    bool isDragging = false;
};
