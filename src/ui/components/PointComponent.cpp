#include "PointComponent.h"
#include "../sections/GridSection.h"

PointComponent::PointComponent(GridSection& owner, juce::ValueTree node)
    : grid(owner), point(node)
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

void PointComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;
    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.beginPanningAtScreenPosition(e.getScreenPosition());
        return;
    }

    dragStartNormalized = normalized;
    dragStartMouse = e.getScreenPosition();

    if (onDragStart)
        onDragStart(point);
}

void PointComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;
    // If Alt is held, forward the drag to the grid for panning.
    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.mouseDrag(e.getEventRelativeTo(&grid));
        return;
    }

    auto deltaPixels = e.getScreenPosition() - dragStartMouse;

    // Convert pixel delta to normalized delta
    auto view = grid.getViewArea();

    float dx = (float)deltaPixels.x / view.getWidth();
    float dy = -(float)deltaPixels.y / view.getHeight();

    juce::Point<float> newPos = dragStartNormalized;
    newPos.x += dx / grid.getZoomX();
    newPos.y += dy / grid.getZoomY();

    bool snapMode = e.mods.isShiftDown();

    if (onDragMove)
        onDragMove(point, newPos, snapMode);
}

void PointComponent::mouseUp(const juce::MouseEvent& e)
{
    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.mouseUp(e.getEventRelativeTo(&grid));
        return;
    }

    if (onDragEnd)
        onDragEnd(point);
}

void PointComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;

    if (!e.mods.isLeftButtonDown())
        return;

    if (auto* parent = dynamic_cast<GridSection*>(getParentComponent()))
    {
        parent->deletePoint(point);
    }
}
