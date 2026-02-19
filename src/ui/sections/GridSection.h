#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/PointComponent.h"
#include "../components/AnchorComponent.h"
#include "../components/WaveformComponent.h"
#include "../../model/EnvelopeData.h"

class GridSection : public juce::Component,
    private juce::ValueTree::Listener,
    private juce::Timer
{
public:
    GridSection();
    ~GridSection() override;

    // Envelope
    void setEnvelope(juce::ValueTree newEnvelope);
    void deletePoint(juce::ValueTree pointNode);

    void setDraggingAnchor(bool b) { isDraggingAnchor = b; }
    void setDraggingPoint(bool b) { isDraggingPoint = b; }
    juce::Rectangle<int> getViewArea() const { return viewArea; }
    float getZoomX() const { return zoomX; }
    float getZoomY() const { return zoomY; }
    float getUniformZoom() const { return uniformZoom; }
    juce::ValueTree activeDragNode;
    juce::Point<float> activeDragPosition;

    // Undo
    void setUndoManager(juce::UndoManager& um);
    juce::UndoManager& getUndoManager();

    // Coordinate mapping
    juce::Point<float> normalizedToPixel(juce::Point<float>) const;
    juce::Point<float> pixelToNormalized(juce::Point<float>) const;

    // Rendering
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    float getCurveForSegment(int index) const;

    // Waveform
    void setSampleBuffer(const std::atomic<int>* writePos,
        const float* sampleData,
        int bufferSize);

    // Mouse interaction
    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

    // Allow child components to begin panning when Alt+drag starts on them
    void beginPanningAtScreenPosition(juce::Point<int> screenPos);
    void mouseWheelMove(const juce::MouseEvent&,
        const juce::MouseWheelDetails&) override;

private:
    // ValueTree Listener
    void valueTreePropertyChanged(juce::ValueTree&,
        const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&,
        juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&,
        juce::ValueTree&,
        int) override;

    // Undo
    juce::UndoManager* undoManager = nullptr;

    bool isDraggingPoint = false;
    bool isDraggingAnchor = false;
    juce::ValueTree activeAnchorNode;
    float activeDragCurve = 0.0f;
    int activeAnchorIndex = -1;

    // View State
    int gridPower = 4;

    static constexpr int minGridPower = 2;
    static constexpr int maxGridPower = 6;

    float zoomX = 1.0f;
    float zoomY = 1.0f;

    // When strict pinch is enabled we drive both axes from this uniform zoom
    float uniformZoom = 1.0f;

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    static constexpr float minZoom = 1.0f;
    static constexpr float maxZoom = 10.0f;

    // (strict pinch uses `uniformZoom`) 

    // Panning
    void updatePanCursor();

    // Timer for debounced writing to ValueTree
    void timerCallback() override;

    bool isPanning = false;
    juce::Point<int> panStartMouse;
    float panStartOffsetX = 0.0f;
    float panStartOffsetY = 0.0f;

    // Rendering helpers
    void drawGrid(juce::Graphics&);
    void rebuildPointComponents();
    void updatePointPositions();
    static float snapValue(float value, float step);

    // Layout
    juce::Rectangle<int> viewArea;
    WaveformComponent waveform;

    // Data
    juce::ValueTree envelope;

    bool pendingZoomWrite = false;
    bool isUserZooming = false;
    bool persistZoomToTree = false; // set true to re-enable writing zoom to ValueTree

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSection)
};
