#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"
#include "../components/PointComponent.h"
#include "../components/AnchorComponent.h"
#include "../../actions/GridUndoActions.h"

class GridSection : public juce::Component,
    private juce::ChangeListener
{
public:
    GridSection();
    ~GridSection();

    void setEnvelope(EnvelopeData*);
    void deletePoint(int index);

    void setUndoManager(juce::UndoManager& um);

    juce::Point<float> normalizedToPixel(juce::Point<float>) const;
    juce::Point<float> pixelToNormalized(juce::Point<float>) const;

    void paint(juce::Graphics&) override;
    void resized() override;
    float getCurveForSegment(int index) const;

    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    EnvelopeData* getEnvelope() { return envelope; }
    juce::UndoManager& getUndoManager();

private:
    juce::UndoManager* undoManager = nullptr;

    int gridPower = 4; // 2^4 = 16 → default 1/16 grid

    const int minGridPower = 2;  // 2^2 = 4   → 1/4
    const int maxGridPower = 6;  // 2^6 = 64  → 1/64

    float zoomX = 1.0f;
    float zoomY = 1.0f;

    float offsetX = 0.0f; // 0..1 visible window start
    float offsetY = 0.0f;

    const float minZoom = 1.0f;
    const float maxZoom = 10.0f;

    void updatePanCursor();

    bool isPanning = false;
    juce::Point<int> panStartMouse;
    float panStartOffsetX = 0.0f;
    float panStartOffsetY = 0.0f;

    void drawGrid(juce::Graphics&);
    void rebuildPointComponents();
    void updatePointPositions();

    juce::Rectangle<int> viewArea;

    static float snapValue(float value, float step);

    EnvelopeData* envelope = nullptr;

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;
    std::unordered_map<int, EnvelopePoint> dragStartStates;
    std::unordered_map<int, float> curveDragStartStates;

    void changeListenerCallback(juce::ChangeBroadcaster*);
};