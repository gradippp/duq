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

    void mouseDoubleClick(const juce::MouseEvent&) override;

    EnvelopeData* getEnvelope() { return envelope; }
    juce::UndoManager& getUndoManager();

private:
    juce::UndoManager* undoManager = nullptr;

    int gridLines = 8;

    void drawGrid(juce::Graphics&);
    void rebuildPointComponents();
    void updatePointPositions();

    juce::Rectangle<int> viewArea;

    static float snapValue(float value, float step);

    EnvelopeData* envelope = nullptr;

    std::vector<std::unique_ptr<PointComponent>> pointComponents;
    std::vector<std::unique_ptr<AnchorComponent>> anchorComponents;
    std::unordered_map<int, EnvelopePoint> dragStartStates;

    void changeListenerCallback(juce::ChangeBroadcaster*);
};