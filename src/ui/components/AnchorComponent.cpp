#include "AnchorComponent.h"
#include "../sections/GridSection.h"

AnchorComponent::AnchorComponent(GridSection& owner, int index)
    : grid(owner), segmentIndex(index)
{
    setSize(10, 10);
}

void AnchorComponent::setNormalizedPosition(juce::Point<float> p)
{
    auto pixel = grid.normalizedToPixel(p);
    setCentrePosition((int)pixel.x, (int)pixel.y);
}

void AnchorComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::orange);
    g.drawEllipse(getLocalBounds().toFloat(), 2.0f);
}

void AnchorComponent::mouseDrag(const juce::MouseEvent& e)
{
    int dy = e.getDistanceFromDragStartY();

    float sensitivity = 0.005f;
    float newCurve =
        juce::jlimit(-1.0f, 1.0f,
            startCurve - dy * sensitivity);

    if (onDragMove)
        onDragMove(segmentIndex, newCurve);
}

void AnchorComponent::mouseDown(const juce::MouseEvent&)
{
    if (onDragStart)
        onDragStart(segmentIndex);

    startCurve = grid.getCurveForSegment(segmentIndex);
}

void AnchorComponent::mouseUp(const juce::MouseEvent&)
{
    if (onDragEnd)
        onDragEnd(segmentIndex);
}

