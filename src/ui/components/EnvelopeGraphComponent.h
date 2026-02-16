#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"

class EnvelopeGraphComponent : public juce::Component
{
public:
    EnvelopeGraphComponent();

    void paint(juce::Graphics& g) override;

    void setEnvelope(EnvelopeData* data);

    // Mouse interaction
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    EnvelopeData* currentEnvelope = nullptr;

    int draggedPointIndex = -1;
    int draggedCurveIndex = -1;

    void drawGrid(juce::Graphics& g, juce::Rectangle<int> area);
    void drawEnvelope(juce::Graphics& g, juce::Rectangle<int> area);

    juce::Point<float> toPixel(
        const EnvelopePoint& p,
        juce::Rectangle<int> area) const;

    EnvelopePoint toNormalized(
        juce::Point<float> pos,
        juce::Rectangle<int> area) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGraphComponent)
};
