#include "GridSection.h"
#include "../../PluginProcessor.h"
#include "../../dsp/EnvelopeCurves.h"
#include "../../Globals.h"

GridSection::GridSection() : pointsContainer(*this)
{
    addAndMakeVisible(gridBackground);
    addAndMakeVisible(waveform);
    addAndMakeVisible(pathRenderer);
    addAndMakeVisible(pointsContainer);
    addAndMakeVisible(playheadOverlay);

    waveform.toBack();
    gridBackground.toBack();
    
    // Ensure overlays don't block interaction with GridSection or Points
    waveform.setInterceptsMouseClicks(false, false);
    pathRenderer.setInterceptsMouseClicks(false, false);
    playheadOverlay.setInterceptsMouseClicks(false, false);
    
    // Allow double-clicks to pass through PointsContainer to reach GridSection
    // when not clicking directly on a point or anchor.
    pointsContainer.setInterceptsMouseClicks(false, true);

    setOpaque(false);
    startTimerHz(60); // 60fps animation
}

GridSection::~GridSection()
{
    stopTimer();

    if (envelope.isValid())
        envelope.removeListener(this);
}

void GridSection::updateViewState()
{
    state.zoomX = zoomX;
    state.zoomY = zoomY;
    state.uniformZoom = uniformZoom;
    state.offsetX = offsetX;
    state.offsetY = offsetY;
    state.gridPower = gridPower;
    state.viewArea = viewArea;

    gridBackground.setViewState(state);
    waveform.setViewState(uniformZoom, offsetX, uniformZoom, offsetY);
    pathRenderer.update(state, envelope, activeDragNode, activeDragPosition, activeAnchorNode, activeDragCurve);
    pointsContainer.setViewState(state);
    playheadOverlay.update(state, processor, currentEnvelopeIndex);
}

void GridSection::valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& i)
{
    if (isUserZooming) return;

    if (i == juce::Identifier("gridPower"))
    {
        gridPower = (int)v.getProperty("gridPower", 4);
        updateViewState();
        return;
    }

    const auto idZoomX = juce::Identifier("zoomX");
    const auto idZoomY = juce::Identifier("zoomY");
    const auto idUniformZoom = juce::Identifier("uniformZoom");
    const auto idOffsetX = juce::Identifier("offsetX");
    const auto idOffsetY = juce::Identifier("offsetY");

    if (i == idZoomX || i == idZoomY || i == idUniformZoom || i == idOffsetX || i == idOffsetY)
    {
        zoomX = (float)v.getProperty(idZoomX, 1.0f);
        zoomY = (float)v.getProperty(idZoomY, 1.0f);
        uniformZoom = (float)v.getProperty(idUniformZoom, 1.0f);
        offsetX = (float)v.getProperty(idOffsetX, 0.0f);
        offsetY = (float)v.getProperty(idOffsetY, 0.0f);

        updateViewState();
        return;
    }

    if (!isDraggingPoint && !isDraggingAnchor)
    {
        pointsContainer.updatePointPositions();
        pathRenderer.update(state, envelope, activeDragNode, activeDragPosition, activeAnchorNode, activeDragCurve);
    }
}

void GridSection::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    pointsContainer.rebuildPointComponents();
    updateViewState();
}

void GridSection::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    pointsContainer.rebuildPointComponents();
    updateViewState();
}

void GridSection::setSampleBuffer(const std::atomic<int>* writePos, const float* sampleData, int bufferSize)
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
    if (newEnvelope == envelope) return;

    if (envelope.isValid())
        envelope.removeListener(this);

    envelope = newEnvelope;

    if (envelope.isValid())
    {
        envelope.addListener(this);
        auto parent = envelope.getParent();
        currentEnvelopeIndex = parent.isValid() ? parent.indexOf(envelope) : -1;

        // Data Integrity Check
        auto points = envelope.getOrCreateChildWithName("POINTS", nullptr);
        auto segments = envelope.getOrCreateChildWithName("SEGMENTS", nullptr);
        int numPoints = points.getNumChildren();
        if (numPoints >= 2 && segments.getNumChildren() != numPoints - 1)
        {
            segments.removeAllChildren(nullptr);
            for (int i = 0; i < numPoints - 1; ++i)
            {
                juce::ValueTree s("SEGMENT");
                s.setProperty("curve", Theme::Defaults::curve, nullptr);
                s.setProperty("type", Theme::Defaults::curveType, nullptr);
                segments.addChild(s, -1, nullptr);
            }
        }

        zoomX = (float)envelope.getProperty("zoomX", Theme::Defaults::zoom);
        zoomY = (float)envelope.getProperty("zoomY", Theme::Defaults::zoom);
        uniformZoom = (float)envelope.getProperty("uniformZoom", Theme::Defaults::zoom);
        offsetX = (float)envelope.getProperty("offsetX", Theme::Defaults::offset);
        offsetY = (float)envelope.getProperty("offsetY", Theme::Defaults::offset);
        gridPower = (int)envelope.getProperty("gridPower", Theme::Defaults::gridPower);
    }

    pointsContainer.setEnvelope(envelope);
    updateViewState();
}

void GridSection::deletePoint(juce::ValueTree pointNode)
{
    if (!envelope.isValid() || !pointNode.isValid()) return;
    auto points = envelope.getChildWithName("POINTS");
    if (!points.isValid()) return;

    int index = points.indexOf(pointNode);
    if (index <= 0 || index >= points.getNumChildren() - 1) return;

    if (undoManager) undoManager->beginNewTransaction("Delete Envelope Point");
    auto segments = envelope.getChildWithName("SEGMENTS");
    if (segments.isValid() && index < segments.getNumChildren()) segments.removeChild(index, undoManager);
    points.removeChild(pointNode, undoManager);
}

void GridSection::resized()
{
    viewArea = getLocalBounds().reduced(20);
    gridBackground.setBounds(getLocalBounds());
    waveform.setBounds(viewArea);
    pathRenderer.setBounds(getLocalBounds());
    pointsContainer.setBounds(getLocalBounds());
    playheadOverlay.setBounds(getLocalBounds());
    
    updateViewState();
}

float GridSection::getCurveForSegment(int index) const
{
    if (!envelope.isValid()) return 0.0f;
    auto segments = envelope.getChildWithName("SEGMENTS");
    if (!segments.isValid() || index >= segments.getNumChildren()) return 0.0f;
    return (float)segments.getChild(index)["curve"];
}

void GridSection::mouseMove(const juce::MouseEvent&) { updatePanCursor(); }

void GridSection::mouseDown(const juce::MouseEvent& e)
{
    if (!envelope.isValid()) return;
    if (juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown() && uniformZoom > 1.0f)
    {
        isPanning = true;
        panStartMouse = e.getEventRelativeTo(this).getPosition();
        panStartOffsetX = offsetX;
        panStartOffsetY = offsetY;
        updatePanCursor();
    }
}

void GridSection::mouseDrag(const juce::MouseEvent& e)
{
    if (!isPanning && juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown() && envelope.isValid() && uniformZoom > 1.0f 
        && !isDraggingPoint && !isDraggingAnchor)
    {
        isPanning = true;
        panStartMouse = e.getEventRelativeTo(this).getPosition();
        panStartOffsetX = offsetX;
        panStartOffsetY = offsetY;
        updatePanCursor();
    }

    if (!isPanning || !envelope.isValid() || uniformZoom <= 1.0f) return;

    auto currentPos = e.getEventRelativeTo(this).position;
    auto deltaF = currentPos - juce::Point<float>((float)panStartMouse.x, (float)panStartMouse.y);
    
    float visibleWidth = 1.0f / uniformZoom;
    float dx = deltaF.x / viewArea.getWidth() * visibleWidth;
    float dy = deltaF.y / viewArea.getHeight() * (1.0f / uniformZoom);

    offsetX = panStartOffsetX - dx;
    offsetY = panStartOffsetY + dy;

    offsetX = juce::jlimit(0.0f, 1.0f - visibleWidth, offsetX);
    offsetY = juce::jlimit(0.0f, 1.0f - (1.0f / uniformZoom), offsetY);

    if (persistZoomToTree) {
        envelope.setProperty("offsetX", offsetX, nullptr);
        envelope.setProperty("offsetY", offsetY, nullptr);
    }

    updateViewState();
    updatePanCursor();
}

void GridSection::mouseUp(const juce::MouseEvent&) { isPanning = false; updatePanCursor(); }

void GridSection::beginPanningAtScreenPosition(juce::Point<int> screenPos)
{
    if (!envelope.isValid() || uniformZoom <= 1.0f) return;
    isPanning = true;
    panStartMouse = getLocalPoint(nullptr, screenPos);
    panStartOffsetX = offsetX;
    panStartOffsetY = offsetY;
    updatePanCursor();
}

void GridSection::updatePanCursor()
{
    bool canPan = juce::ModifierKeys::getCurrentModifiersRealtime().isAltDown() && uniformZoom > 1.0f;
    if (!canPan) { setMouseCursor(juce::MouseCursor::NormalCursor); return; }
    setMouseCursor(isPanning ? juce::MouseCursor::DraggingHandCursor : juce::MouseCursor::PointingHandCursor);
}

void GridSection::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!envelope.isValid() || !e.mods.isLeftButtonDown()) return;
    
    // Use relative position to GridSection
    auto localPos = e.getEventRelativeTo(this).position;
    auto normalized = pixelToNormalized(localPos);
    
    if (normalized.x <= 0.0f || normalized.x >= 1.0f) return;

    auto points = envelope.getOrCreateChildWithName("POINTS", undoManager);
    auto segments = envelope.getOrCreateChildWithName("SEGMENTS", undoManager);

    juce::ValueTree newPoint("POINT");
    newPoint.setProperty("x", normalized.x, nullptr);
    newPoint.setProperty("y", normalized.y, nullptr);

    juce::ValueTree newSegment("SEGMENT");
    newSegment.setProperty("curve", Theme::Defaults::curve, nullptr);
    newSegment.setProperty("type", Theme::Defaults::curveType, nullptr);

    int insertIndex = 0;
    for (int i = 0; i < points.getNumChildren(); ++i) {
        if ((float)points.getChild(i)["x"] > normalized.x) break;
        insertIndex = i + 1;
    }

    if (undoManager) undoManager->beginNewTransaction("Add Envelope Point");
    points.addChild(newPoint, insertIndex, undoManager);
    segments.addChild(newSegment, std::max(0, insertIndex - 1), undoManager);
}

void GridSection::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (!envelope.isValid()) return;

    if (juce::ModifierKeys::getCurrentModifiersRealtime().isCtrlDown() && !wheel.isSmooth) {
        gridPower = juce::jlimit(minGridPower, maxGridPower, gridPower + (wheel.deltaY > 0 ? 1 : -1));
        envelope.setProperty("gridPower", gridPower, nullptr);
        updateViewState();
        return;
    }

    float zoomFactor = std::exp(wheel.deltaY * 0.2f);
    float oldUniform = uniformZoom;
    auto localPos = e.getEventRelativeTo(this).position;

    float nx = (localPos.x - viewArea.getX()) / viewArea.getWidth();
    float ny = 1.0f - ((localPos.y - viewArea.getY()) / viewArea.getHeight());

    float mouseNormX = offsetX + nx * (1.0f / oldUniform);
    float mouseNormY = offsetY + ny * (1.0f / oldUniform);

    uniformZoom = juce::jlimit(minZoom, maxZoom, uniformZoom * zoomFactor);
    zoomX = uniformZoom;
    zoomY = uniformZoom;

    float visW = 1.0f / zoomX;
    float visH = 1.0f / zoomY;

    offsetX = juce::jlimit(0.0f, 1.0f - visW, mouseNormX - nx * visW);
    offsetY = juce::jlimit(0.0f, 1.0f - visH, mouseNormY - ny * visH);

    updateViewState();
    pendingZoomWrite = true;
    zoomWriteCounter = 10;
    isUserZooming = true;
}

void GridSection::timerCallback()
{
    playheadOverlay.update(state, processor, currentEnvelopeIndex);

    if (pendingZoomWrite && --zoomWriteCounter <= 0) {
        if (envelope.isValid() && persistZoomToTree) {
            envelope.setProperty("zoomX", zoomX, nullptr);
            envelope.setProperty("zoomY", zoomY, nullptr);
            envelope.setProperty("uniformZoom", uniformZoom, nullptr);
            envelope.setProperty("offsetX", offsetX, nullptr);
            envelope.setProperty("offsetY", offsetY, nullptr);
        }
        pendingZoomWrite = false;
        isUserZooming = false;
    }
}

void GridSection::paint(juce::Graphics&) {}
void GridSection::paintOverChildren(juce::Graphics&) {}

void GridSection::updatePointPositions() { pointsContainer.updatePointPositions(); }

void GridSection::updatePathRenderer()
{
    pathRenderer.update(state, envelope, activeDragNode, activeDragPosition, activeAnchorNode, activeDragCurve);
}
