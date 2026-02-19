#include "PointComponent.h"
#include "../sections/GridSection.h"
#include "../../Globals.h"

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
    auto bounds = getLocalBounds().toFloat();

    if (isHovering)
    {
        g.setColour(Theme::Colours::point.withAlpha(0.3f));
        g.fillEllipse(bounds);
        g.setColour(Theme::Colours::point);
        g.fillEllipse(bounds.reduced(2.0f));
    }
    else
    {
        g.setColour(Theme::Colours::point);
        g.fillEllipse(bounds.reduced(2.0f));
    }
}

void PointComponent::mouseEnter(const juce::MouseEvent&)
{
    isHovering = true;
    repaint();
}

void PointComponent::mouseExit(const juce::MouseEvent&)
{
    isHovering = false;
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
    if (!point.isValid())
        return;

    if (!e.mods.isLeftButtonDown())
        return;

    if (auto* parent = dynamic_cast<GridSection*>(getParentComponent()))
    {
        parent->deletePoint(point);
    }
}

void PointComponent::showPositionDialog()
{
    auto points = point.getParent();
    if (!points.isValid()) return;

    int index = points.indexOf(point);
    int numPoints = points.getNumChildren();
    bool isEndpoint = (index == 0 || index == numPoints - 1);

    auto* aw = new juce::AlertWindow("Edit Position", "Enter normalized coordinates (0.0 - 1.0):", juce::MessageBoxIconType::NoIcon);

    aw->addTextEditor("x", juce::String((float)point["x"], 3), "Time (X):");
    aw->addTextEditor("y", juce::String((float)point["y"], 3), "Value (Y):");

    // Disable X editing for endpoints
    if (isEndpoint)
    {
        if (auto* editor = aw->getTextEditor("x"))
        {
            editor->setEnabled(false);
            editor->setText(juce::String((float)point["x"], 1)); 
        }
    }

    aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<GridSection> safeGrid(&grid);
    juce::ValueTree pointTree = point;

    aw->enterModalState(true, juce::ModalCallbackFunction::create([safeGrid, pointTree, isEndpoint, aw](int result) mutable
    {
        if (result == 1 && safeGrid != nullptr)
        {
            float x = aw->getTextEditorContents("x").getFloatValue();
            float y = aw->getTextEditorContents("y").getFloatValue();

            x = juce::jlimit(0.0f, 1.0f, x);
            y = juce::jlimit(0.0f, 1.0f, y);

            auto& um = safeGrid->getUndoManager();
            um.beginNewTransaction("Edit Point Position");

            if (!isEndpoint)
                pointTree.setProperty("x", x, &um);

            pointTree.setProperty("y", y, &um);
        }
        delete aw;
    }));
}
