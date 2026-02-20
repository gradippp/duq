#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GridBackground.h"
#include "../../dsp/EnvelopeCurves.h"

class EnvelopePathRenderer : public juce::Component
{
public:
    EnvelopePathRenderer()
    {
        setInterceptsMouseClicks(false, false);
    }

    void update(const GridViewState& newState, juce::ValueTree newEnvelope, 
                juce::ValueTree dragNode, juce::Point<float> dragPos,
                juce::ValueTree anchorNode, float dragCurve)
    {
        state = newState;
        envelope = newEnvelope;
        activeDragNode = dragNode;
        activeDragPosition = dragPos;
        activeAnchorNode = anchorNode;
        activeDragCurve = dragCurve;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        if (!envelope.isValid()) return;

        auto points = envelope.getChildWithName("POINTS");
        if (!points.isValid()) return;

        const int numPoints = points.getNumChildren();
        if (numPoints < 2) return;

        juce::Path path;
        auto firstNode = points.getChild(0);

        float firstX = (firstNode == activeDragNode) ? activeDragPosition.x : (float)firstNode["x"];
        float firstY = (firstNode == activeDragNode) ? activeDragPosition.y : (float)firstNode["y"];

        path.startNewSubPath(state.normalizedToPixel({ firstX, firstY }));

        auto segments = envelope.getChildWithName("SEGMENTS");

        for (int i = 0; i < numPoints - 1; ++i)
        {
            auto p1 = points.getChild(i);
            auto p2 = points.getChild(i + 1);

            float x1 = (p1 == activeDragNode) ? activeDragPosition.x : (float)p1["x"];
            float y1 = (p1 == activeDragNode) ? activeDragPosition.y : (float)p1["y"];
            float x2 = (p2 == activeDragNode) ? activeDragPosition.x : (float)p2["x"];
            float y2 = (p2 == activeDragNode) ? activeDragPosition.y : (float)p2["y"];

            float curve = 0.0f;
            CurveType type = CurveType::Exponential;

            if (segments.isValid() && i < segments.getNumChildren())
            {
                auto sNode = segments.getChild(i);
                curve = (sNode == activeAnchorNode) ? activeDragCurve : (float)sNode["curve"];
                type = (CurveType)(int)sNode["type"];
            }

            const int resolution = 40;
            for (int s = 1; s <= resolution; ++s)
            {
                float t = (float)s / resolution;
                float shapedT = EnvelopeCurves::applyCurve(t, curve, type);
                float x = juce::jmap(t, x1, x2);
                float y = juce::jmap(shapedT, y1, y2);
                path.lineTo(state.normalizedToPixel({ x, y }));
            }
        }

        g.setColour(Theme::Colours::envelopeLine);
        g.strokePath(path, juce::PathStrokeType(2.0f));

        juce::Path fillPath = path;
        auto lastNode = points.getChild(numPoints - 1);
        float fillLastX = (lastNode == activeDragNode) ? activeDragPosition.x : (float)lastNode["x"];
        float fillFirstX = (firstNode == activeDragNode) ? activeDragPosition.x : (float)firstNode["x"];

        fillPath.lineTo(state.normalizedToPixel({ fillLastX, 0.0f }));
        fillPath.lineTo(state.normalizedToPixel({ fillFirstX, 0.0f }));
        fillPath.closeSubPath();

        juce::ColourGradient grad(Theme::Colours::envelopeFillTop, 0, (float)state.viewArea.getY(),
                                  Theme::Colours::envelopeFillBot, 0, (float)state.viewArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);
    }

private:
    GridViewState state;
    juce::ValueTree envelope;
    juce::ValueTree activeDragNode;
    juce::Point<float> activeDragPosition;
    juce::ValueTree activeAnchorNode;
    float activeDragCurve = 0.0f;
};
