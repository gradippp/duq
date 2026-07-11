#include "PointComponent.h"
#include "../sections/GridSection.h"
#include "../utils/DialogUtils.h"
#include "../../Globals.h"

PointComponent::PointComponent(GridSection& owner, juce::ValueTree node)
    : grid(owner), point(node)
{
    setSize(12, 12);
}

juce::String PointComponent::getTooltip()
{
    if (!point.isValid()) return {};
    return "X: " + juce::String((float)point["x"], 3) + ", Y: " + juce::String((float)point["y"], 3);
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
    auto bounds = getLocalBounds().toFloat();

    if (isHovering)
    {
        g.setColour(T_COL(point).withAlpha(0.3f));
        g.fillEllipse(bounds);
        g.setColour(T_COL(point));
        g.fillEllipse(bounds.reduced(2.0f));
    }
    else
    {
        g.setColour(T_COL(point));
        g.fillEllipse(bounds.reduced(2.0f));
    }
}

void PointComponent::mouseEnter(const juce::MouseEvent& e)
{
    isHovering = true;
    mouseMove(e);
    repaint();
}

void PointComponent::mouseMove(const juce::MouseEvent& e)
{
    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
    else
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void PointComponent::mouseExit(const juce::MouseEvent&)
{
    isHovering = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();
}

void PointComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!point.isValid())
        return;

    if (e.mods.isRightButtonDown())
    {
        auto points = point.getParent();
        int index = points.indexOf(point);
        bool isEndpoint = (index == 0 || index == points.getNumChildren() - 1);

        juce::PopupMenu m;
        m.addItem(1, "Edit Position...");
        m.addItem(2, "Delete Point", !isEndpoint, false);

        juce::Component::SafePointer<PointComponent> safeThis(this);

        m.showMenuAsync(juce::PopupMenu::Options(), [safeThis](int result)
        {
            if (safeThis == nullptr)
                return;

            if (result == 1)
            {
                safeThis->showPositionDialog();
            }
            else if (result == 2)
            {
                safeThis->grid.deletePoint(safeThis->point);
            }
        });
        return;
    }

    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.beginPanningAtScreenPosition(e.getScreenPosition());
        return;
    }

    if (e.mods.isLeftButtonDown())
    {
        isDragging = true;
        dragStartNormalized = normalized;
        dragStartMouse = e.getScreenPosition();

        if (onDragStart)
            onDragStart(point);
    }
}

void PointComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!point.isValid() || !isDragging)
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

    if (isDragging)
    {
        isDragging = false;
        if (onDragEnd)
            onDragEnd(point);
    }
}

void PointComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!point.isValid() || !e.mods.isLeftButtonDown())
        return;

    auto points = point.getParent();
    if (!points.isValid()) return;

    int index = points.indexOf(point);
    if (index == 0 || index == points.getNumChildren() - 1)
        return; // Cannot delete endpoints

    grid.deletePoint(point);
}

void PointComponent::showPositionDialog()
{
    auto points = point.getParent();
    if (!points.isValid()) return;

    int index = points.indexOf(point);
    int numPoints = points.getNumChildren();
    bool isEndpoint = (index == 0 || index == numPoints - 1);

    Dialogs::showTextEntry("Edit Position", "Enter normalized coordinates (0.0 - 1.0):",
        { { "x", "Time (X):", juce::String((float)point["x"], isEndpoint ? 1 : 3), !isEndpoint },
          { "y", "Value (Y):", juce::String((float)point["y"], 3), true } },
        [safeGrid = juce::Component::SafePointer<GridSection>(&grid), pointTree = point, isEndpoint]
        (const std::map<juce::String, juce::String>& values) mutable
        {
            if (safeGrid == nullptr) return;

            float x = juce::jlimit(0.0f, 1.0f, values.at("x").getFloatValue());
            float y = juce::jlimit(0.0f, 1.0f, values.at("y").getFloatValue());

            auto& um = safeGrid->getUndoManager();
            um.beginNewTransaction("Edit Point Position");

            if (!isEndpoint)
                pointTree.setProperty("x", x, &um);

            pointTree.setProperty("y", y, &um);
        });
}
