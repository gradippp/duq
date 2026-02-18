#include "AnchorComponent.h"
#include "../sections/GridSection.h"

AnchorComponent::AnchorComponent(GridSection& owner,
    juce::ValueTree node)
    : grid(owner), point(node)
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

void AnchorComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;

    startCurve = (float)point["curve"];
    dragStartMouse = e.getScreenPosition();

    if (onDragStart)
        onDragStart(point);
}

void AnchorComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;

    auto deltaPixels = e.getScreenPosition() - dragStartMouse;

    float sensitivity = 0.005f;

    float newCurve = juce::jlimit(
        -1.0f,
        1.0f,
        startCurve - deltaPixels.y * sensitivity
    );

    if (onDragMove)
        onDragMove(point, newCurve);
}

void AnchorComponent::mouseUp(const juce::MouseEvent&)
{
    if (!point.isValid())
        return;

    if (onDragEnd)
        onDragEnd(point);
}
