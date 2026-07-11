#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/WaveformComponent.h"
#include "../../model/EnvelopeData.h"
#include "GridBackground.h"
#include "EnvelopePathRenderer.h"
#include "PlayheadOverlay.h"
#include "PointsContainer.h"

class GridSection : public juce::Component,
    private juce::ValueTree::Listener
{
public:
    GridSection();
    ~GridSection() override;

    // Driven by the editor's shared 60Hz frame timer; also ticks the waveform.
    void onFrameTick();

    // Envelope
    void setEnvelope(juce::ValueTree newEnvelope);
    void setProcessor(class DuqAudioProcessor* p) { processor = p; }
    void deletePoint(juce::ValueTree pointNode);

    void setDraggingAnchor(bool b) { isDraggingAnchor = b; }
    void setDraggingPoint(bool b) { isDraggingPoint = b; }
    
    juce::Rectangle<int> getViewArea() const { return viewArea; }
    float getZoomX() const { return zoomX; }
    float getZoomY() const { return zoomY; }
    float getUniformZoom() const { return uniformZoom; }
    int getGridPower() const { return gridPower; }
    juce::ValueTree getEnvelope() const { return envelope; }

    juce::ValueTree activeDragNode;
    juce::Point<float> activeDragPosition;
    juce::ValueTree activeAnchorNode;
    float activeDragCurve = 0.0f;

    // Undo
    void setUndoManager(juce::UndoManager& um);
    juce::UndoManager& getUndoManager();
    juce::UndoManager* getUndoManagerPtr() { return undoManager; }

    // Coordinate mapping
    juce::Point<float> normalizedToPixel(juce::Point<float> p) const { return state.normalizedToPixel(p); }
    juce::Point<float> pixelToNormalized(juce::Point<float> p) const { return state.pixelToNormalized(p); }

    // Rendering
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    float getCurveForSegment(int index) const;

    // Waveform
    void setSampleBuffers(const std::atomic<int>* writePos,
        std::span<const float> preData,
        std::span<const float> postData,
        std::span<const float> sidechainData);

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

    void updatePointPositions();
    void updatePathRenderer();
    static float snapValue(float value, float step);

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
    int activeAnchorIndex = -1;

    // View State
    GridViewState state;
    int gridPower = 4;
    static constexpr int minGridPower = 2;
    static constexpr int maxGridPower = 6;
    float zoomX = 1.0f;
    float zoomY = 1.0f;
    float uniformZoom = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    static constexpr float minZoom = 1.0f;
    static constexpr float maxZoom = 10.0f;

    void updatePanCursor();
    void updateViewState();

    bool isPanning = false;
    juce::Point<int> panStartMouse;
    float panStartOffsetX = 0.0f;
    float panStartOffsetY = 0.0f;

    // Layout
    juce::Rectangle<int> viewArea;
    
    // Components
    GridBackground gridBackground;
    WaveformComponent waveform;
    EnvelopePathRenderer pathRenderer;
    PointsContainer pointsContainer;
    PlayheadOverlay playheadOverlay;

    // Data
    juce::ValueTree envelope;
    class DuqAudioProcessor* processor = nullptr;
    int currentEnvelopeIndex = -1;

    bool pendingZoomWrite = false;
    int zoomWriteCounter = 0;
    bool isUserZooming = false;
    bool persistZoomToTree = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSection)
};
