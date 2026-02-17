#include "GridSection.h"

float applyCurve(float t, float curve)
{
    if (curve == 0.0f)
        return t;

    float k = curve * 4.0f; // scale aggression

    if (curve > 0)
        return 1.0f - std::pow(1.0f - t, 1.0f + k);
    else
        return std::pow(t, 1.0f - k);
}


GridSection::GridSection()
{
    setOpaque(true);
}

GridSection::~GridSection()
{
    if (undoManager)
        undoManager->removeChangeListener(this);
}


void GridSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
    undoManager->addChangeListener(this);
}

juce::UndoManager& GridSection::getUndoManager()
{
    jassert(undoManager != nullptr);
    return *undoManager;
}


float GridSection::snapValue(float value, float step)
{
    return std::round(value / step) * step;
}

void GridSection::setEnvelope(EnvelopeData* newEnvelope)
{
    envelope = newEnvelope;
    rebuildPointComponents();
    repaint();
}

void GridSection::deletePoint(int index)
{
    if (!envelope)
        return;

    auto& points = envelope->points;

    const int lastIndex = (int)points.size() - 1;

    // Protect endpoints
    if (index == 0 || index == lastIndex)
        return;

    if (index >= 0 && index < points.size())
    {
        EnvelopePoint removed = points[index];

        undoManager->beginNewTransaction("Delete Envelope Point");
        undoManager->perform(
            new DeletePointAction(*envelope, removed, index));
    }
}

void GridSection::resized()
{
    viewArea = getLocalBounds().reduced(20);
    updatePointPositions();
}

float GridSection::getCurveForSegment(int index) const
{
    return envelope->points[index].curve;
}

void GridSection::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!envelope)
        return;

    if (!e.mods.isLeftButtonDown())
        return;

    // If double click landed on a child component, ignore
    if (e.eventComponent != this)
        return;

    auto normalized = pixelToNormalized(e.position);

    auto& points = envelope->points;

    if (normalized.x <= 0.0f || normalized.x >= 1.0f)
        return;

    EnvelopePoint newPoint;
    newPoint.x = normalized.x;
    newPoint.y = normalized.y;
    newPoint.curve = 0.0f;

    auto it = std::lower_bound(points.begin(), points.end(), newPoint.x,
        [](const EnvelopePoint& p, float value)
        {
            return p.x < value;
        });

    int index = std::distance(points.begin(), it);

    undoManager->beginNewTransaction("Add Envelope Point");
    undoManager->perform(
        new AddPointAction(*envelope, newPoint, index));
}

juce::Point<float> GridSection::normalizedToPixel(juce::Point<float> p) const
{
    float visibleWidth = 1.0f / zoomX;
    float visibleHeight = 1.0f / zoomY;

    float nx = (p.x - offsetX) / visibleWidth;
    float ny = (p.y - offsetY) / visibleHeight;

    return {
        viewArea.getX() + nx * viewArea.getWidth(),
        viewArea.getY() + (1.0f - ny) * viewArea.getHeight()
    };
}

juce::Point<float> GridSection::pixelToNormalized(juce::Point<float> p) const
{
    float visibleWidth = 1.0f / zoomX;
    float visibleHeight = 1.0f / zoomY;

    float nx = (p.x - viewArea.getX()) / viewArea.getWidth();
    float ny = 1.0f - ((p.y - viewArea.getY()) / viewArea.getHeight());

    float realX = offsetX + nx * visibleWidth;
    float realY = offsetY + ny * visibleHeight;

    return {
        juce::jlimit(0.0f, 1.0f, realX),
        juce::jlimit(0.0f, 1.0f, realY)
    };
}

void GridSection::mouseWheelMove(const juce::MouseEvent& e,
    const juce::MouseWheelDetails& wheel)
{
    if (!envelope)
        return;

    float zoomFactor = 1.0f + wheel.deltaY * 0.2f;

    float oldZoomX = zoomX;
    float oldZoomY = zoomY;

    zoomX = juce::jlimit(minZoom, maxZoom, zoomX * zoomFactor);
    zoomY = juce::jlimit(minZoom, maxZoom, zoomY * zoomFactor);

    // Zoom around mouse position
    auto mouseNorm = pixelToNormalized(e.position);

    float visibleWidthOld = 1.0f / oldZoomX;
    float visibleHeightOld = 1.0f / oldZoomY;

    float visibleWidthNew = 1.0f / zoomX;
    float visibleHeightNew = 1.0f / zoomY;

    offsetX = mouseNorm.x -
        ((mouseNorm.x - offsetX) / visibleWidthOld) * visibleWidthNew;

    offsetY = mouseNorm.y -
        ((mouseNorm.y - offsetY) / visibleHeightOld) * visibleHeightNew;

    offsetX = juce::jlimit(0.0f, 1.0f - visibleWidthNew, offsetX);
    offsetY = juce::jlimit(0.0f, 1.0f - visibleHeightNew, offsetY);

    updatePointPositions();
    repaint();
}

void GridSection::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.2f));
    g.fillRect(viewArea);

    drawGrid(g);

    if (!envelope)
        return;

    juce::Path path;

    auto& points = envelope->points;

    auto first = normalizedToPixel({ points[0].x, points[0].y });
    path.startNewSubPath(first);

    for (int i = 0; i < (int)points.size() - 1; ++i)
    {
        auto& p1 = points[i];
        auto& p2 = points[i + 1];

        const int resolution = 40;

        for (int s = 0; s <= resolution; ++s)
        {
            float t = (float)s / resolution;

            float shapedT = applyCurve(t, p1.curve);

            float x = juce::jmap(t, p1.x, p2.x);
            float y = juce::jmap(shapedT, p1.y, p2.y);

            auto pixel = normalizedToPixel({ x, y });

            if (i == 0 && s == 0)
                path.startNewSubPath(pixel);
            else
                path.lineTo(pixel);
        }
    }

    g.setColour(juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void GridSection::drawGrid(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.05f));

    float visibleWidth = 1.0f / zoomX;
    float visibleHeight = 1.0f / zoomY;

    float startX = offsetX;
    float endX = offsetX + visibleWidth;

    float startY = offsetY;
    float endY = offsetY + visibleHeight;

    float baseStepX = 1.0f / gridLines;
    float baseStepY = 1.0f / gridLines;

    // --- Adaptive density X ---
    float pixelsPerGridX = viewArea.getWidth() * (baseStepX / visibleWidth);

    while (pixelsPerGridX < 8.0f)
    {
        baseStepX *= 2.0f;
        pixelsPerGridX *= 2.0f;
    }

    // --- Adaptive density Y ---
    float pixelsPerGridY = viewArea.getHeight() * (baseStepY / visibleHeight);

    while (pixelsPerGridY < 8.0f)
    {
        baseStepY *= 2.0f;
        pixelsPerGridY *= 2.0f;
    }

    // --- Vertical lines ---
    int firstLineX = std::floor(startX / baseStepX);
    int lastLineX = std::ceil(endX / baseStepX);

    for (int i = firstLineX; i <= lastLineX; ++i)
    {
        float normX = i * baseStepX;

        if (normX < 0.0f || normX > 1.0f)
            continue;

        auto p = normalizedToPixel({ normX, 0.0f });

        g.drawLine(p.x,
            viewArea.getY(),
            p.x,
            viewArea.getBottom());
    }

    // --- Horizontal lines ---
    int firstLineY = std::floor(startY / baseStepY);
    int lastLineY = std::ceil(endY / baseStepY);

    for (int i = firstLineY; i <= lastLineY; ++i)
    {
        float normY = i * baseStepY;

        if (normY < 0.0f || normY > 1.0f)
            continue;

        auto p = normalizedToPixel({ 0.0f, normY });

        g.drawLine(viewArea.getX(),
            p.y,
            viewArea.getRight(),
            p.y);
    }
}

void GridSection::rebuildPointComponents()
{
    removeAllChildren();
    pointComponents.clear();
    anchorComponents.clear();

    if (!envelope)
        return;

    auto& points = envelope->points;

    // === Build points ===
    for (int i = 0; i < (int)points.size(); ++i)
    {
        auto comp = std::make_unique<PointComponent>(*this, i);

        comp->onDragStart = [this](int index)
            {
                if (!envelope) return;

                dragStartStates[index] = envelope->points[index];

                undoManager->beginNewTransaction("Move Envelope Point");
            };

        comp->onDragMove = [this](int index,
            juce::Point<float> pos,
            bool snapMode)
            {
                if (!envelope) return;

                auto& pts = envelope->points;

                pos.x = juce::jlimit(0.0f, 1.0f, pos.x);
                pos.y = juce::jlimit(0.0f, 1.0f, pos.y);

                if (snapMode)
                {
                    float snapStep = 1.0f / gridLines;
                    pos.x = snapValue(pos.x, snapStep);
                    pos.y = snapValue(pos.y, snapStep);
                }

                const int lastIndex = (int)pts.size() - 1;

                if (index == 0)
                {
                    pts[index].x = 0.0f;
                    pts[index].y = pos.y;
                }
                else if (index == lastIndex)
                {
                    pts[index].x = 1.0f;
                    pts[index].y = pos.y;
                }
                else
                {
                    float leftLimit = pts[index - 1].x + 0.001f;
                    float rightLimit = pts[index + 1].x - 0.001f;

                    pos.x = juce::jlimit(leftLimit, rightLimit, pos.x);

                    pts[index].x = pos.x;
                    pts[index].y = pos.y;
                }

                updatePointPositions();
                repaint();
            };

        comp->onDragEnd = [this](int index)
            {
                if (!envelope) return;

                auto newState = envelope->points[index];
                auto oldState = dragStartStates[index];

                if (!juce::approximatelyEqual(oldState.x, newState.x) ||
                    !juce::approximatelyEqual(oldState.y, newState.y))
                {
                    undoManager->perform(
                        new MovePointAction(*envelope,
                            index,
                            oldState,
                            newState));
                }

                dragStartStates.erase(index);
            };


        addAndMakeVisible(comp.get());
        pointComponents.push_back(std::move(comp));
    }

    // === Build anchors (ONE PER SEGMENT) ===
    for (int i = 0; i < (int)points.size() - 1; ++i)
    {
        auto anchor = std::make_unique<AnchorComponent>(*this, i);

        anchor->onDragStart = [this](int segmentIndex)
            {
                if (!envelope) return;

                curveDragStartStates[segmentIndex] =
                    envelope->points[segmentIndex].curve;

                if (undoManager)
                    undoManager->beginNewTransaction("Move Curve");
            };

        anchor->onDragMove = [this](int segmentIndex, float newCurve)
            {
                if (!envelope) return;

                envelope->points[segmentIndex].curve =
                    juce::jlimit(-1.0f, 1.0f, newCurve);

                updatePointPositions();
                repaint();
            };

        anchor->onDragEnd = [this](int segmentIndex)
            {
                if (!envelope || !undoManager) return;

                auto it = curveDragStartStates.find(segmentIndex);
                if (it == curveDragStartStates.end())
                    return;

                float oldCurve = it->second;
                float newCurve = envelope->points[segmentIndex].curve;

                if (!juce::approximatelyEqual(oldCurve, newCurve))
                {
                    undoManager->perform(
                        new MoveCurveAction(*envelope,
                            segmentIndex,
                            oldCurve,
                            newCurve));
                }

                curveDragStartStates.erase(segmentIndex);
            };

        addAndMakeVisible(anchor.get());
        anchorComponents.push_back(std::move(anchor));
    }

    updatePointPositions();
}


void GridSection::updatePointPositions()
{
    if (!envelope)
        return;

    for (int i = 0; i < (int)pointComponents.size(); ++i)
    {
        auto& p = envelope->points[i];
        pointComponents[i]->setNormalizedPosition({ p.x, p.y });
    }

    // Update anchors
    for (int i = 0; i < (int)anchorComponents.size(); ++i)
    {
        auto& p1 = envelope->points[i];
        auto& p2 = envelope->points[i + 1];

        float t = 0.5f;

        float shapedT = applyCurve(t, p1.curve);

        float x = juce::jmap(t, p1.x, p2.x);
        float y = juce::jmap(shapedT, p1.y, p2.y);

        anchorComponents[i]->setNormalizedPosition({ x, y });
    }
}

void GridSection::changeListenerCallback(juce::ChangeBroadcaster*)
{
    rebuildPointComponents();
    repaint();
}
