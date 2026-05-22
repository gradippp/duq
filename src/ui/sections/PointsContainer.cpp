#include "PointsContainer.h"
#include "GridSection.h"
#include "../../PluginProcessor.h"
#include "../../dsp/EnvelopeCurves.h"

PointsContainer::PointsContainer(GridSection& owner) : grid(owner)
{
}

void PointsContainer::setEnvelope(juce::ValueTree newEnvelope)
{
    envelope = newEnvelope;
    rebuildPointComponents();
}

void PointsContainer::setViewState(const GridViewState& newState)
{
    state = newState;
    updatePointPositions();
}

void PointsContainer::rebuildPointComponents()
{
    for (auto& p : pointComponents)
        removeChildComponent(p.get());

    for (auto& a : anchorComponents)
        removeChildComponent(a.get());

    pointComponents.clear();
    anchorComponents.clear();

    if (!envelope.isValid())
        return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid())
        return;

    const int numPoints = points.getNumChildren();
    juce::Component::SafePointer<GridSection> safeGrid(&grid);
    juce::Component::SafePointer<PointsContainer> safeThis(this);

    // POINT COMPONENTS
    for (int i = 0; i < numPoints; ++i)
    {
        auto node = points.getChild(i);
        auto comp = std::make_unique<PointComponent>(grid, node);

        comp->onDragStart = [safeGrid, node]([[maybe_unused]] juce::ValueTree vt)
        {
            if (safeGrid == nullptr) return;
            safeGrid->setDraggingPoint(true);
            safeGrid->activeDragNode = node;
            safeGrid->activeDragPosition = { (float)node["x"], (float)node["y"] };
            if (safeGrid->getUndoManagerPtr())
                safeGrid->getUndoManagerPtr()->beginNewTransaction("Move Envelope Point");
        };

        comp->onDragMove = [safeGrid, safeThis, compPtr = comp.get()](juce::ValueTree targetNode, juce::Point<float> pos, bool snapMode)
        {
            if (safeGrid == nullptr || safeThis == nullptr || !targetNode.isValid()) return;

            auto pointsVT = safeGrid->getEnvelope().getChildWithName("POINTS");
            if (!pointsVT.isValid()) return;

            const int index = pointsVT.indexOf(targetNode);
            const int totalPoints = pointsVT.getNumChildren();

            // X constraints
            if (index == 0) pos.x = 0.0f;
            else if (index == totalPoints - 1) pos.x = 1.0f;
            else {
                float leftX = (float)pointsVT.getChild(index - 1)["x"] + 0.0001f;
                float rightX = (float)pointsVT.getChild(index + 1)["x"] - 0.0001f;
                pos.x = juce::jlimit(leftX, rightX, pos.x);
            }

            pos.y = juce::jlimit(0.0f, 1.0f, pos.y);

            if (snapMode) {
                float snapStep = 1.0f / (1 << safeGrid->getGridPower());
                if (index > 0 && index < totalPoints - 1) pos.x = safeGrid->snapValue(pos.x, snapStep);
                pos.y = safeGrid->snapValue(pos.y, snapStep);
            }

            safeGrid->activeDragNode = node;
            safeGrid->activeDragPosition = pos;
            compPtr->setNormalizedPosition(pos);

            // Live anchor updates
            safeThis->updatePointPositions();
            
            // Explicitly update path renderer with new drag data
            safeGrid->updatePathRenderer();
            safeGrid->repaint();
        };

        comp->onDragEnd = [safeGrid](juce::ValueTree node)
        {
            if (safeGrid == nullptr || !node.isValid()) return;

            float oldX = node["x"];
            float oldY = node["y"];

            if (oldX != safeGrid->activeDragPosition.x || oldY != safeGrid->activeDragPosition.y)
            {
                node.setProperty("x", safeGrid->activeDragPosition.x, safeGrid->getUndoManagerPtr());
                node.setProperty("y", safeGrid->activeDragPosition.y, safeGrid->getUndoManagerPtr());
            }

            safeGrid->activeDragNode = {};
            safeGrid->setDraggingPoint(false);
        };

        addAndMakeVisible(comp.get());
        pointComponents.push_back(std::move(comp));
    }

    // ANCHOR COMPONENTS
    auto segments = envelope.getChildWithName("SEGMENTS");
    if (segments.isValid())
    {
        for (int i = 0; i < segments.getNumChildren(); ++i)
        {
            auto node = segments.getChild(i);
            auto anchor = std::make_unique<AnchorComponent>(grid, node);

            anchor->onDragStart = [safeGrid](juce::ValueTree targetNode)
            {
                if (safeGrid == nullptr) return;
                safeGrid->setDraggingAnchor(true);
                safeGrid->activeAnchorNode = targetNode;
                safeGrid->activeDragCurve = (float)targetNode["curve"];
                if (safeGrid->getUndoManagerPtr())
                    safeGrid->getUndoManagerPtr()->beginNewTransaction("Move Curve");
            };

            anchor->onDragMove = [safeGrid](juce::ValueTree targetNode, float newCurve)
            {
                if (safeGrid == nullptr || !targetNode.isValid()) return;
                safeGrid->activeAnchorNode = targetNode;
                safeGrid->activeDragCurve = juce::jlimit(-1.0f, 1.0f, newCurve);
                safeGrid->updatePointPositions();
                
                // Explicitly update path renderer with new curve data
                safeGrid->updatePathRenderer();
                safeGrid->repaint();
            };

            anchor->onDragEnd = [safeGrid](juce::ValueTree targetNode)
            {
                if (safeGrid == nullptr || !targetNode.isValid()) return;
                targetNode.setProperty("curve", safeGrid->activeDragCurve, safeGrid->getUndoManagerPtr());
                safeGrid->activeAnchorNode = {};
                safeGrid->setDraggingAnchor(false);
            };

            addAndMakeVisible(anchor.get());
            anchorComponents.push_back(std::move(anchor));
        }
    }

    updatePointPositions();
}

void PointsContainer::updatePointPositions()
{
    if (!envelope.isValid()) return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid()) return;

    const int numPoints = points.getNumChildren();

    for (int i = 0; i < numPoints && i < (int)pointComponents.size(); ++i)
    {
        auto point = points.getChild(i);
        float x = (point == grid.activeDragNode) ? grid.activeDragPosition.x : (float)point["x"];
        float y = (point == grid.activeDragNode) ? grid.activeDragPosition.y : (float)point["y"];
        pointComponents[i]->setNormalizedPosition({ x, y });
    }

    auto segments = envelope.getChildWithName("SEGMENTS");
    for (int i = 0; i < numPoints - 1 && i < (int)anchorComponents.size(); ++i)
    {
        auto p1 = points.getChild(i);
        auto p2 = points.getChild(i + 1);

        float x1 = (p1 == grid.activeDragNode) ? grid.activeDragPosition.x : (float)p1["x"];
        float y1 = (p1 == grid.activeDragNode) ? grid.activeDragPosition.y : (float)p1["y"];
        float x2 = (p2 == grid.activeDragNode) ? grid.activeDragPosition.x : (float)p2["x"];
        float y2 = (p2 == grid.activeDragNode) ? grid.activeDragPosition.y : (float)p2["y"];

        float curve = 0.0f;
        CurveType type = CurveType::Exponential;

        if (segments.isValid() && i < segments.getNumChildren())
        {
            auto sNode = segments.getChild(i);
            curve = (sNode == grid.activeAnchorNode) ? grid.activeDragCurve : (float)sNode["curve"];
            type = (CurveType)(int)sNode["type"];
        }

        float t = 0.5f;
        float shapedT = EnvelopeCurves::applyCurve(t, curve, type);
        float x = juce::jmap(t, x1, x2);
        float y = juce::jmap(shapedT, y1, y2);
        anchorComponents[i]->setNormalizedPosition({ x, y });
    }

    grid.repaint();
}
