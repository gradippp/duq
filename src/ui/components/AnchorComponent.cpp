#include "AnchorComponent.h"
#include "../sections/GridSection.h"
#include "../../model/EnvelopeData.h"
#include "../../utils/ConfigManager.h"
#include "../../Globals.h"

AnchorComponent::AnchorComponent(GridSection& owner,
    juce::ValueTree node)
    : grid(owner), segment(node)
{
    setSize(10, 10);
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

void AnchorComponent::setNormalizedPosition(juce::Point<float> p)
{
    auto pixel = grid.normalizedToPixel(p);
    setCentrePosition((int)pixel.x, (int)pixel.y);
}

void AnchorComponent::paint(juce::Graphics& g)
{
    g.setColour(T_COL(anchor));
    g.drawEllipse(getLocalBounds().toFloat(), 2.0f);
}

void AnchorComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!segment.isValid())
        return;

    if (e.mods.isRightButtonDown())
    {
        juce::PopupMenu m;
        m.addItem((int)CurveType::Exponential + 1, "Exponential", true, (int)segment.getProperty("type", (int)CurveType::Exponential) == (int)CurveType::Exponential);
        m.addItem((int)CurveType::Linear + 1, "Linear", true, (int)segment.getProperty("type", (int)CurveType::Exponential) == (int)CurveType::Linear);
        m.addItem((int)CurveType::Logarithmic + 1, "Logarithmic", true, (int)segment.getProperty("type", (int)CurveType::Exponential) == (int)CurveType::Logarithmic);
        m.addItem((int)CurveType::SCurve + 1, "S-Curve", true, (int)segment.getProperty("type", (int)CurveType::Exponential) == (int)CurveType::SCurve);
        m.addItem((int)CurveType::Step + 1, "Step", true, (int)segment.getProperty("type", (int)CurveType::Exponential) == (int)CurveType::Step);

        m.addSeparator();
        m.addItem(10, "Set Tension...");
        m.addItem(11, "Reset Tension");

        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
        {
            if (result >= 1 && result <= 5)
            {
                auto& um = grid.getUndoManager();
                um.beginNewTransaction("Change Curve Type");
                segment.setProperty("type", result - 1, &um);
                grid.repaint();
            }
            else if (result == 10)
            {
                showTensionDialog();
            }
            else if (result == 11)
            {
                auto& um = grid.getUndoManager();
                um.beginNewTransaction("Reset Tension");
                juce::SharedResourcePointer<ConfigManager> config;
                segment.setProperty("curve", config->getDefaultTension(), &um);
                grid.repaint();
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
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        juce::SharedResourcePointer<ConfigManager> config;
        startCurve = (float)segment.getProperty("curve", config->getDefaultTension());
        dragStartMouse = e.getScreenPosition();

        if (onDragStart)
            onDragStart(segment);
    }
}

void AnchorComponent::showTensionDialog()
{
    juce::SharedResourcePointer<ConfigManager> config;
    auto* aw = new juce::AlertWindow("Set Tension", "Enter tension value (0.0 to 1.0):", juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor("tension", juce::String((float)segment.getProperty("curve", config->getDefaultTension())), "Tension:");
    aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result)
    {
        if (result == 1)
        {
            float val = aw->getTextEditorContents("tension").getFloatValue();
            auto& um = grid.getUndoManager();
            um.beginNewTransaction("Set Tension");
            segment.setProperty("curve", juce::jlimit(0.0f, 1.0f, val), &um);
            grid.repaint();
        }
        delete aw;
    }));
}

void AnchorComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!segment.isValid() || !isDragging)
        return;

    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.mouseDrag(e.getEventRelativeTo(&grid));
        return;
    }

    auto deltaPixels = e.getScreenPosition() - dragStartMouse;
    const float sensitivity = 0.005f;
    float newCurve = juce::jlimit(0.0f, 1.0f, startCurve - deltaPixels.y * sensitivity);

    if (onDragMove)
        onDragMove(segment, newCurve);
}

void AnchorComponent::mouseUp(const juce::MouseEvent& e)
{
    if (!segment.isValid())
        return;

    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        grid.mouseUp(e.getEventRelativeTo(&grid));
        return;
    }

    if (isDragging)
    {
        isDragging = false;
        if (onDragEnd)
            onDragEnd(segment);
    }

    // Restore cursor based on hover state
    if (!getBounds().contains(e.getPosition()))
    {
        // If we're not hovering anymore, we can let it revert to parent cursor
        // but if we're still over it, the component's default (set in constructor) will take over.
    }
}

void AnchorComponent::mouseEnter(const juce::MouseEvent& e)
{
    mouseMove(e);
}

void AnchorComponent::mouseMove(const juce::MouseEvent& e)
{
    if (e.mods.isAltDown() && grid.getUniformZoom() > 1.0f)
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        return;
    }
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

void AnchorComponent::mouseExit(const juce::MouseEvent&)
{
    // Let it revert to parent's cursor automatically
}
