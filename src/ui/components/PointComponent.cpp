#include "PointComponent.h"
#include "../sections/GridSection.h"

PointComponent::PointComponent(GridSection& owner, int index)
    : grid(owner), pointIndex(index)
{
    setSize(12, 12);
}

void PointComponent::setNormalizedPosition(juce::Point<float> p)
{
    normalized = p;

    auto pixel = grid.normalizedToPixel(p);
    setCentrePosition((int)pixel.x, (int)pixel.y);
}

juce::Point<float> PointComponent::getNormalizedPosition() const
{
    return normalized;
}

void PointComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.fillEllipse(getLocalBounds().toFloat());
}

void PointComponent::mouseDrag(const juce::MouseEvent& e)
{
    auto pos = grid.pixelToNormalized(e.getEventRelativeTo(&grid).position);

    if (onDrag)
        onDrag(pointIndex, pos);
}
