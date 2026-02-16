#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"

class EnvelopeGraphComponent : public juce::Component
{
public:
    EnvelopeGraphComponent();

    void setEnvelope(EnvelopeData* data);

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:
    void drawGrid(juce::Graphics& g, juce::Rectangle<int> area);
    void drawEnvelope(juce::Graphics& g, juce::Rectangle<int> area);

    juce::Point<float> toPixel(const EnvelopePoint& p,
        juce::Rectangle<int> area) const;

    EnvelopePoint toNormalized(juce::Point<float> pos,
        juce::Rectangle<int> area) const;

    juce::Point<float> getHandlePosition(size_t index,
        juce::Rectangle<int> area) const;

private:
    EnvelopeData* currentEnvelope = nullptr;

    int draggedPointIndex = -1;
    int draggedCurveIndex = -1;
    float dragStartCurveValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGraphComponent)
};
