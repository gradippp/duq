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
    addAndMakeVisible(waveform);
    waveform.toBack();
    setOpaque(false);
}

GridSection::~GridSection()
{
    if (envelope.isValid())
        envelope.removeListener(this);
}

void GridSection::valueTreePropertyChanged(
    juce::ValueTree&,
    const juce::Identifier&)
{
    // If the user is actively zooming, ignore external property changes to
    // avoid the rubber-banding feedback loop where tree writes overwrite
    // the active UI change.
    if (isUserZooming)
        return;

    if (!isDraggingPoint && !isDraggingAnchor)
    {
        // A property change (e.g. curve) in the ValueTree should update
        // the anchor/point positions so undo/redo immediately reflects in UI.
        updatePointPositions();
        repaint();
    }
}

void GridSection::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    rebuildPointComponents();
    repaint();
}

void GridSection::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    rebuildPointComponents();
    repaint();
}


void GridSection::setSampleBuffer(
    const std::atomic<int>* writePos,
    const float* sampleData,
    int bufferSize)
{
    waveform.setSampleBuffer(writePos, sampleData, bufferSize);
}

void GridSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
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

void GridSection::setEnvelope(juce::ValueTree newEnvelope)
{
    if (envelope.isValid())
        envelope.removeListener(this);

    envelope = newEnvelope;

    if (envelope.isValid())
    {
        envelope.addListener(this);

        // Load view state from tree
        zoomX = (float)envelope.getProperty("zoomX", 1.0f);
        zoomY = (float)envelope.getProperty("zoomY", 1.0f);
        uniformZoom = (float)envelope.getProperty("uniformZoom", 1.0f);
        offsetX = (float)envelope.getProperty("offsetX", 0.0f);
        offsetY = (float)envelope.getProperty("offsetY", 0.0f);
        gridPower = (int)envelope.getProperty("gridPower", 4);
    }

    rebuildPointComponents();
    repaint();
}

void GridSection::deletePoint(juce::ValueTree pointNode)
{
    if (!envelope.isValid() || !pointNode.isValid())
        return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid())
        return;

    int index = points.indexOf(pointNode);
    if (index < 0)
        return;

    const int lastIndex = points.getNumChildren() - 1;

    // Protect endpoints
    if (index == 0 || index == lastIndex)
        return;

    if (undoManager)
        undoManager->beginNewTransaction("Delete Envelope Point");

    points.removeChild(pointNode, undoManager);
}

void GridSection::resized()
{
    viewArea = getLocalBounds().reduced(20);
    waveform.setBounds(viewArea);
    updatePointPositions();
}

float GridSection::getCurveForSegment(int index) const
{
    if (!envelope.isValid())
        return 0.0f;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid() || index >= points.getNumChildren())
        return 0.0f;

    return (float)points.getChild(index)["curve"];
}

void GridSection::mouseMove(const juce::MouseEvent&)
{
    updatePanCursor();
}

// Note: GridSection intentionally does not implement MouseListener
// overrides directly. Panning is started from child components by calling
// `beginPanningAtScreenPosition` when Alt+drag is detected in child
// components. This avoids duplicate definitions of existing mouse handlers.

void GridSection::mouseDown(const juce::MouseEvent& e)
{
    if (!envelope.isValid())
        return;

    bool altDown =
        juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown();

    bool canPan = (uniformZoom > 1.0f);

    if (altDown && canPan)
    {
        isPanning = true;
        panStartMouse = e.getEventRelativeTo(this).getPosition();

        panStartOffsetX = offsetX;
        panStartOffsetY = offsetY;

        updatePanCursor();
        {
            juce::String msg = "GridSection::mouseDown startPan alt=";
            msg += (altDown ? "true" : "false");
            msg += " panStartMouse=(";
            msg += juce::String(panStartMouse.x);
            msg += ",";
            msg += juce::String(panStartMouse.y);
            msg += ") panStartOffset=(";
            msg += juce::String(panStartOffsetX);
            msg += ",";
            msg += juce::String(panStartOffsetY);
            msg += ")";
            juce::Logger::outputDebugString(msg);
        }
    }
}

void GridSection::mouseDrag(const juce::MouseEvent& e)
{
    // If the user didn't start panning on mouseDown (e.g. clicked a child
    // component), allow panning to start on the first mouseDrag when Alt is
    // held and zoom is active.
    bool altDown = juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown();

    if (!isPanning && altDown && envelope.isValid() && (uniformZoom > 1.0f)
        && !isDraggingPoint && !isDraggingAnchor)
    {
        isPanning = true;
        panStartMouse = e.getEventRelativeTo(this).getPosition();
        panStartOffsetX = offsetX;
        panStartOffsetY = offsetY;
        updatePanCursor();
        {
            juce::String msg = "GridSection::mouseDrag autoStartPan alt=";
            msg += (altDown ? "true" : "false");
            msg += " panStartMouse=(";
            msg += juce::String(panStartMouse.x);
            msg += ",";
            msg += juce::String(panStartMouse.y);
            msg += ") panStartOffset=(";
            msg += juce::String(panStartOffsetX);
            msg += ",";
            msg += juce::String(panStartOffsetY);
            msg += ")";
            juce::Logger::outputDebugString(msg);
        }
    }

    if (!isPanning || !envelope.isValid() || (uniformZoom <= 1.0f))
        return;

    auto currentPos = e.getEventRelativeTo(this).position;
    auto deltaF = currentPos - juce::Point<float>((float)panStartMouse.x, (float)panStartMouse.y);
    juce::Point<int> delta = { (int)std::round(deltaF.x), (int)std::round(deltaF.y) };

    {
        juce::String msg = "GridSection::mouseDrag delta=(";
        msg += juce::String(delta.x);
        msg += ",";
        msg += juce::String(delta.y);
        msg += ") currentPos=(";
        msg += juce::String(currentPos.x);
        msg += ",";
        msg += juce::String(currentPos.y);
        msg += ") offsetBefore=(";
        msg += juce::String(offsetX);
        msg += ",";
        msg += juce::String(offsetY);
        msg += ")";
        juce::Logger::outputDebugString(msg);
    }

    float visibleWidth = 1.0f / uniformZoom;
    float visibleHeight = 1.0f / uniformZoom;

    float dx = (float)delta.x / viewArea.getWidth() * visibleWidth;
    float dy = (float)delta.y / viewArea.getHeight() * visibleHeight;

    offsetX = panStartOffsetX - dx;
    offsetY = panStartOffsetY + dy;

    offsetX = juce::jlimit(0.0f, 1.0f - visibleWidth, offsetX);
    offsetY = juce::jlimit(0.0f, 1.0f - visibleHeight, offsetY);

    if (persistZoomToTree)
    {
        envelope.setProperty("offsetX", offsetX, nullptr);
        envelope.setProperty("offsetY", offsetY, nullptr);
    }

    waveform.setViewState(uniformZoom, offsetX);

    updatePanCursor();
    updatePointPositions();
    repaint();
}

void GridSection::mouseUp(const juce::MouseEvent&)
{
    isPanning = false;
    updatePanCursor();
}

void GridSection::beginPanningAtScreenPosition(juce::Point<int> screenPos)
{
    if (!envelope.isValid() || uniformZoom <= 1.0f)
        return;

    isPanning = true;
    // Convert screen pos to local
    auto local = getLocalPoint(nullptr, screenPos);
    panStartMouse = local;
    panStartOffsetX = offsetX;
    panStartOffsetY = offsetY;
    updatePanCursor();
}

void GridSection::updatePanCursor()
{
    bool altDown =
        juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown();

    // Use uniformZoom to decide if panning is possible (strict pinch uses
    // uniformZoom as the single zoom state).
    bool canPan = (uniformZoom > 1.0f);

    if (!altDown || !canPan)
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        return;
    }

    if (isPanning)
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    else
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void GridSection::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!envelope.isValid())
        return;

    if (!e.mods.isLeftButtonDown())
        return;

    if (e.eventComponent != this)
        return;

    auto normalized = pixelToNormalized(e.position);

    if (normalized.x <= 0.0f || normalized.x >= 1.0f)
        return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid())
        return;

    juce::ValueTree newPoint("POINT");
    newPoint.setProperty("x", normalized.x, nullptr);
    newPoint.setProperty("y", normalized.y, nullptr);
    newPoint.setProperty("curve", 0.0f, nullptr);

    int insertIndex = 0;

    for (int i = 0; i < points.getNumChildren(); ++i)
    {
        if ((float)points.getChild(i)["x"] > normalized.x)
            break;

        insertIndex = i + 1;
    }

    if (undoManager)
        undoManager->beginNewTransaction("Add Envelope Point");

    points.addChild(newPoint, insertIndex, undoManager);
}

juce::Point<float> GridSection::normalizedToPixel(juce::Point<float> p) const
{
    float visibleWidth = 1.0f / uniformZoom;
    float visibleHeight = 1.0f / uniformZoom;

    float nx = (p.x - offsetX) / visibleWidth;
    float ny = (p.y - offsetY) / visibleHeight;

    return {
        viewArea.getX() + nx * viewArea.getWidth(),
        viewArea.getY() + (1.0f - ny) * viewArea.getHeight()
    };
}

juce::Point<float> GridSection::pixelToNormalized(juce::Point<float> p) const
{
    float visibleWidth = 1.0f / uniformZoom;
    float visibleHeight = 1.0f / uniformZoom;

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
    if (!envelope.isValid())
        return;

    // --- CTRL + Scroll -> change grid resolution ---
    bool ctrlDown =
        juce::ModifierKeys::getCurrentModifiersRealtime().isCtrlDown();
    
    if (ctrlDown && !wheel.isSmooth)
    {
        if (wheel.deltaY > 0)
            gridPower = juce::jlimit(minGridPower, maxGridPower, gridPower + 1);
        else if (wheel.deltaY < 0)
            gridPower = juce::jlimit(minGridPower, maxGridPower, gridPower - 1);

        envelope.setProperty("gridPower", gridPower, nullptr);

        repaint();
        return;
    }

    // --- Normal scroll -> zoom ---
    // Map wheel delta directly to an exponential scale factor for smooth
    // pinch-like behaviour. Apply per-event to keep interaction responsive.
    float sensitivity = 0.2f;
    float zoomFactor = std::exp(wheel.deltaY * sensitivity);

    // Save old values
    float oldZoomX = zoomX;
    float oldZoomY = zoomY;
    float oldOffsetX = offsetX; 
    float oldOffsetY = offsetY; 

    // Use the uniformZoom (base) to compute the mouse-normalized anchor
    // so the zoom is anchored consistently when we switch to strict pinch.
    float oldUniform = uniformZoom;
    float visibleWidthOld = 1.0f / oldUniform;
    float visibleHeightOld = 1.0f / oldUniform;

    auto localPos = e.getEventRelativeTo(this).position;

    float nx = (localPos.x - viewArea.getX()) / viewArea.getWidth();
    float ny = 1.0f - ((localPos.y - viewArea.getY()) / viewArea.getHeight());

    float mouseNormX = oldOffsetX + nx * visibleWidthOld;
    float mouseNormY = oldOffsetY + ny * visibleHeightOld;
    // Strict pinch: compute a single base zoom. Use the larger of the two
    // current zooms as the base so the dominant axis doesn't shrink when
    // you start pinching — this prevents the "revert" behaviour you saw.
    float baseZoom = uniformZoom;
    float newBase = juce::jlimit(minZoom, maxZoom, baseZoom * zoomFactor);

    uniformZoom = newBase;
    zoomX = uniformZoom;
    zoomY = uniformZoom;

    float visibleWidthNew = 1.0f / zoomX;
    float visibleHeightNew = 1.0f / zoomY;

    // Recalculate offset so mouse stays anchored
    offsetX = mouseNormX - nx * visibleWidthNew;
    offsetY = mouseNormY - ny * visibleHeightNew;

    // Clamp
    offsetX = juce::jlimit(0.0f, 1.0f - visibleWidthNew, offsetX);
    offsetY = juce::jlimit(0.0f, 1.0f - visibleHeightNew, offsetY);

    // Update visuals immediately, but defer writing to the ValueTree to
    // avoid feedback from other listeners that can cause rubber-banding.
    waveform.setViewState(uniformZoom, offsetX);
    updatePointPositions();
    repaint();

    // Debounce writing to the ValueTree to avoid rapid feedback loops.
    // The actual properties will be committed by timerCallback().
    pendingZoomWrite = true;
    isUserZooming = true;
    startTimer(100); // 100ms
}

void GridSection::timerCallback()
{
    stopTimer();

    if (!pendingZoomWrite || !envelope.isValid())
        return;
    if (persistZoomToTree)
    {
        envelope.setProperty("zoomX", zoomX, nullptr);
        envelope.setProperty("zoomY", zoomY, nullptr);
        envelope.setProperty("uniformZoom", uniformZoom, nullptr);
        envelope.setProperty("offsetX", offsetX, nullptr);
        envelope.setProperty("offsetY", offsetY, nullptr);
    }

    pendingZoomWrite = false;
    isUserZooming = false;
}

void GridSection::paint(juce::Graphics& g)
{
}

void GridSection::paintOverChildren(juce::Graphics& g)
{
    drawGrid(g);

    if (!envelope.isValid())
        return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid())
        return;

    const int numPoints = points.getNumChildren();
    if (numPoints < 2)
        return;

    juce::Path path;

    // ---- First point ----
    auto firstNode = points.getChild(0);

    float firstX = (firstNode == activeDragNode)
        ? activeDragPosition.x
        : (float)firstNode["x"];

    float firstY = (firstNode == activeDragNode)
        ? activeDragPosition.y
        : (float)firstNode["y"];

    path.startNewSubPath(
        normalizedToPixel({ firstX, firstY })
    );

    // ---- Segments ----
    for (int i = 0; i < numPoints - 1; ++i)
    {
        auto p1 = points.getChild(i);
        auto p2 = points.getChild(i + 1);

        float x1 = (p1 == activeDragNode)
            ? activeDragPosition.x
            : (float)p1["x"];

        float y1 = (p1 == activeDragNode)
            ? activeDragPosition.y
            : (float)p1["y"];

        float x2 = (p2 == activeDragNode)
            ? activeDragPosition.x
            : (float)p2["x"];

        float y2 = (p2 == activeDragNode)
            ? activeDragPosition.y
            : (float)p2["y"];

        float curve = (p1 == activeAnchorNode)
            ? activeDragCurve
            : (float)p1["curve"];

        const int resolution = 40;

        for (int s = 1; s <= resolution; ++s)
        {
            float t = (float)s / resolution;
            float shapedT = applyCurve(t, curve);

            float x = juce::jmap(t, x1, x2);
            float y = juce::jmap(shapedT, y1, y2);

            path.lineTo(normalizedToPixel({ x, y }));
        }
    }

    g.setColour(juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}


void GridSection::drawGrid(juce::Graphics& g)
{
    int divisions = 1 << gridPower;      // 2^gridPower
    float baseStep = 1.0f / divisions;

    float visibleWidth = 1.0f / uniformZoom;
    float visibleHeight = 1.0f / uniformZoom;

    float startX = offsetX;
    float endX = offsetX + visibleWidth;

    float startY = offsetY;
    float endY = offsetY + visibleHeight;

    // Adaptive density
    float pxPerGridX = viewArea.getWidth() * (baseStep / visibleWidth);
    while (pxPerGridX < 8.0f)
    {
        baseStep *= 2.0f;
        pxPerGridX *= 2.0f;
    }

    float pxPerGridY = viewArea.getHeight() * (baseStep / visibleHeight);
    while (pxPerGridY < 8.0f)
    {
        baseStep *= 2.0f;
        pxPerGridY *= 2.0f;
    }

    int firstX = std::floor(startX / baseStep);
    int lastX = std::ceil(endX / baseStep);

    for (int i = firstX; i <= lastX; ++i)
    {
        float normX = i * baseStep;

        if (normX < 0.0f || normX > 1.0f)
            continue;

        auto p = normalizedToPixel({ normX, 0.0f });

        bool isMajor = (i % 4 == 0);

        g.setColour(isMajor
            ? juce::Colours::white.withAlpha(0.15f)
            : juce::Colours::white.withAlpha(0.05f));

        g.drawLine(p.x,
            viewArea.getY(),
            p.x,
            viewArea.getBottom());
    }

    int firstY = std::floor(startY / baseStep);
    int lastY = std::ceil(endY / baseStep);

    for (int i = firstY; i <= lastY; ++i)
    {
        float normY = i * baseStep;

        if (normY < 0.0f || normY > 1.0f)
            continue;

        auto p = normalizedToPixel({ 0.0f, normY });

        bool isMajor = (i % 4 == 0);

        g.setColour(isMajor
            ? juce::Colours::white.withAlpha(0.15f)
            : juce::Colours::white.withAlpha(0.05f));

        g.drawLine(viewArea.getX(),
            p.y,
            viewArea.getRight(),
            p.y);
    }
}

void GridSection::rebuildPointComponents()
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

    // ============================
    // POINT COMPONENTS
    // ============================

    for (int i = 0; i < numPoints; ++i)
    {
        auto node = points.getChild(i);
        auto comp = std::make_unique<PointComponent>(*this, node);

        juce::Component::SafePointer<GridSection> safeThis(this);

        comp->onDragStart = [safeThis](juce::ValueTree node)
        {
            if (safeThis == nullptr)
                return;

            auto& grid = *safeThis;

            grid.isDraggingPoint = true;
            grid.activeDragNode = node;

            if (grid.undoManager)
                grid.undoManager->beginNewTransaction("Move Envelope Point");
        };

        comp->onDragMove = [safeThis, compPtr = comp.get()](juce::ValueTree node,
            juce::Point<float> pos,
            bool snapMode)
        {
            if (safeThis == nullptr)
                return;

            if (!node.isValid())
                return;

            auto& grid = *safeThis;

            pos.x = juce::jlimit(0.0f, 1.0f, pos.x);
            pos.y = juce::jlimit(0.0f, 1.0f, pos.y);

            if (snapMode)
            {
                float snapStep = 1.0f / (1 << grid.gridPower);
                pos.x = grid.snapValue(pos.x, snapStep);
                pos.y = grid.snapValue(pos.y, snapStep);
            }

            grid.activeDragNode = node;
            grid.activeDragPosition = pos;

            // Move dragged point visually
            compPtr->setNormalizedPosition(pos);

            // ---- LIVE ANCHOR UPDATE ----
            auto points = grid.envelope.getChildWithName("POINTS");
            if (!points.isValid())
                return;

            int index = points.indexOf(node);
            if (index < 0)
                return;

            const int numPoints = points.getNumChildren();

            // Update anchor BEFORE this point
            if (index > 0 && (size_t)(index - 1) < grid.anchorComponents.size())
            {
                auto prevNode = points.getChild(index - 1);

                float x1 = (prevNode == grid.activeDragNode)
                    ? grid.activeDragPosition.x
                    : (float)prevNode["x"];

                float y1 = (prevNode == grid.activeDragNode)
                    ? grid.activeDragPosition.y
                    : (float)prevNode["y"];

                float x2 = pos.x;
                float y2 = pos.y;

                float curve = (float)prevNode["curve"];

                float t = 0.5f;
                float shapedT = applyCurve(t, curve);

                float ax = juce::jmap(t, x1, x2);
                float ay = juce::jmap(shapedT, y1, y2);

                grid.anchorComponents[index - 1]->setNormalizedPosition({ ax, ay });
            }

            // Update anchor AFTER this point
            if (index < numPoints - 1 && (size_t)index < grid.anchorComponents.size())
            {
                auto nextNode = points.getChild(index + 1);

                float x1 = pos.x;
                float y1 = pos.y;

                float x2 = (nextNode == grid.activeDragNode)
                    ? grid.activeDragPosition.x
                    : (float)nextNode["x"];

                float y2 = (nextNode == grid.activeDragNode)
                    ? grid.activeDragPosition.y
                    : (float)nextNode["y"];

                float curve = (float)node["curve"];

                float t = 0.5f;
                float shapedT = applyCurve(t, curve);

                float ax = juce::jmap(t, x1, x2);
                float ay = juce::jmap(shapedT, y1, y2);

                grid.anchorComponents[index]->setNormalizedPosition({ ax, ay });
            }

            grid.repaint();
        };

        comp->onDragEnd = [safeThis](juce::ValueTree node)
            {
                if (safeThis == nullptr)
                    return;

                auto& grid = *safeThis;

                if (node.isValid())
                {
                    node.setProperty("x", grid.activeDragPosition.x, grid.undoManager);
                    node.setProperty("y", grid.activeDragPosition.y, grid.undoManager);
                }

                // Transaction started on drag start is ended implicitly when a
                // new transaction is started elsewhere. Do not start an empty
                // transaction here.

                grid.activeDragNode = {};
                grid.isDraggingPoint = false;
            };

        addAndMakeVisible(comp.get());
        pointComponents.push_back(std::move(comp));
    }

    // ============================
    // ANCHOR COMPONENTS
    // ============================

    for (int i = 0; i < numPoints - 1; ++i)
    {
        auto node = points.getChild(i);
        auto anchor = std::make_unique<AnchorComponent>(*this, node);

        juce::Component::SafePointer<GridSection> safeThisAnchor(this);

        anchor->onDragStart =
            [safeThisAnchor, i](juce::ValueTree node)
            {
                if (safeThisAnchor == nullptr)
                    return;

                auto& grid = *safeThisAnchor;

                grid.isDraggingAnchor = true;
                grid.activeAnchorNode = node;
                grid.activeDragCurve = (float)node["curve"];
                grid.activeAnchorIndex = i;

                if (grid.undoManager)
                    grid.undoManager->beginNewTransaction("Move Curve");
            };

        anchor->onDragMove =
            [safeThisAnchor, i](juce::ValueTree node, float newCurve)
            {
                if (safeThisAnchor == nullptr)
                    return;

                if (!node.isValid())
                    return;

                auto& grid = *safeThisAnchor;

                grid.activeAnchorNode = node;
                grid.activeAnchorIndex = i;
                grid.activeDragCurve = juce::jlimit(-1.0f, 1.0f, newCurve);

                grid.updatePointPositions();
                grid.repaint();
            };

        anchor->onDragEnd =
            [safeThisAnchor](juce::ValueTree node)
            {
                if (safeThisAnchor == nullptr)
                    return;

                auto& grid = *safeThisAnchor;

                if (!node.isValid())
                    return;

                // Commit the edited curve to the exact node that was dragged.
                node.setProperty("curve", grid.activeDragCurve, grid.undoManager);

                grid.activeAnchorNode = {};
                grid.activeAnchorIndex = -1;
                grid.isDraggingAnchor = false;
            };

        addAndMakeVisible(anchor.get());
        anchorComponents.push_back(std::move(anchor));
    }

    updatePointPositions();
}



void GridSection::updatePointPositions()
{
    if (!envelope.isValid())
        return;

    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid())
        return;

    const int numPoints = points.getNumChildren();

    for (int i = 0; i < numPoints && i < (int)pointComponents.size(); ++i)
    {
        auto point = points.getChild(i);

        float x = (float)point["x"];
        float y = (float)point["y"];

        pointComponents[i]->setNormalizedPosition({ x, y });
    }

    for (int i = 0; i < numPoints - 1 && i < (int)anchorComponents.size(); ++i)
    {
        auto p1 = points.getChild(i);
        auto p2 = points.getChild(i + 1);

        float x1 = (float)p1["x"];
        float y1 = (float)p1["y"];
        float x2 = (float)p2["x"];
        float y2 = (float)p2["y"];
        float curve = (p1 == activeAnchorNode)
            ? activeDragCurve
            : (float)p1["curve"];


        float t = 0.5f;
        float shapedT = applyCurve(t, curve);

        float x = juce::jmap(t, x1, x2);
        float y = juce::jmap(shapedT, y1, y2);

        anchorComponents[i]->setNormalizedPosition({ x, y });
    }
}
