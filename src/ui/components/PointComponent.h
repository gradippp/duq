#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GridSection;

class PointComponent : public juce::Component
{
public:
    PointComponent(GridSection& owner, int index);

    void setNormalizedPosition(juce::Point<float> p);
    juce::Point<float> getNormalizedPosition() const;

    int getIndex() const noexcept { return pointIndex; }

    std::function<void(int, juce::Point<float>)> onDrag;

    void paint(juce::Graphics&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

private:
    GridSection& grid;
    int pointIndex;

    juce::Point<float> normalized{ 0.0f, 0.0f };
};

