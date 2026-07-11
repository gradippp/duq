#include "AnchorComponent.h"
#include "../sections/GridSection.h"
#include "../../model/EnvelopeData.h"
#include "../../utils/ConfigManager.h"
#include "../utils/DialogUtils.h"
#include "../../Globals.h"

AnchorComponent::AnchorComponent(GridSection& owner,
    juce::ValueTree node)
    : grid(owner), segment(node)
{
    setSize(10, 10);
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

juce::String AnchorComponent::getTooltip()
{
    if (!segment.isValid()) return {};

    float tension = segment.getProperty("curve", 0.5f);
    int typeIdx = segment.getProperty("type", 0);
    
    juce::String typeStr;
    switch ((CurveType)typeIdx)
    {
        case CurveType::Exponential: typeStr = "Exponential"; break;
        case CurveType::Linear:      typeStr = "Linear"; break;
        case CurveType::Logarithmic: typeStr = "Logarithmic"; break;
        case CurveType::SCurve:      typeStr = "S-Curve"; break;
        case CurveType::Step:        typeStr = "Step"; break;
        default:                     typeStr = "Unknown"; break;
    }

    return "Tension: " + juce::String(tension, 2) + " (" + typeStr + ")";
}

void AnchorComponent::setNormalizedPosition(juce::Point<float> p)
{
    auto pixel = grid.normalizedToPixel(p);
    setCentrePosition((int)pixel.x, (int)pixel.y);
}

void AnchorComponent::paint(juce::Graphics& g)
{
    g.setColour(Theme::get(ThemeManager::anchor));
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
    Dialogs::showTextEntry("Set Tension", "Enter tension value (0.0 to 1.0):",
        { { "tension", "Tension:", juce::String((float)segment.getProperty("curve", config->getDefaultTension())), true } },
        [safeGrid = juce::Component::SafePointer<GridSection>(&grid), segmentTree = segment]
        (const std::map<juce::String, juce::String>& values) mutable
        {
            if (safeGrid == nullptr) return;
            float val = juce::jlimit(0.0f, 1.0f, values.at("tension").getFloatValue());
            auto& um = safeGrid->getUndoManager();
            um.beginNewTransaction("Set Tension");
            segmentTree.setProperty("curve", val, &um);
            safeGrid->repaint();
        });
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

void AnchorComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!segment.isValid() || !e.mods.isLeftButtonDown())
        return;

    auto& um = grid.getUndoManager();
    um.beginNewTransaction("Reset Tension");
    juce::SharedResourcePointer<ConfigManager> config;
    segment.setProperty("curve", config->getDefaultTension(), &um);
    grid.repaint();
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
