#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GridBackground.h"
#include "../../dsp/EnvelopeCurves.h"
#include "../../dsp/EnvelopeSmoothing.h"

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
        cacheDirty = true;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        if (cacheDirty)
            rebuildCache();

        if (!cacheValid)
            return;

        juce::ColourGradient grad(T_COL(envelopeFillTop), 0, (float)state.viewArea.getY(),
                                  T_COL(envelopeFillBot), 0, (float)state.viewArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillPath(cachedFillPath);

        if (hasSmoothOverlay)
        {
            g.setColour(T_COL(accent).withAlpha(0.35f));
            g.strokePath(cachedSmoothPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
        }

        g.setColour(T_COL(envelopeLine));
        g.strokePath(cachedCurvePath, juce::PathStrokeType(2.0f));
    }

private:
    void rebuildCache()
    {
        cacheDirty = false;
        cacheValid = false;
        hasSmoothOverlay = false;
        cachedCurvePath.clear();
        cachedFillPath.clear();
        cachedSmoothPath.clear();

        if (!envelope.isValid())
            return;

        auto points = envelope.getChildWithName("POINTS");
        if (!points.isValid())
            return;

        const int numPoints = points.getNumChildren();
        if (numPoints < 2)
            return;

        auto getTrueX = [&](juce::ValueTree p) -> float {
            return (p == activeDragNode) ? activeDragPosition.x : (float)p["x"];
        };
        auto getTrueY = [&](juce::ValueTree p) -> float {
            return (p == activeDragNode) ? activeDragPosition.y : (float)p["y"];
        };

        auto segments = envelope.getChildWithName("SEGMENTS");
        auto firstNode = points.getChild(0);
        float firstX = getTrueX(firstNode);
        float firstY = getTrueY(firstNode);

        cachedCurvePath.startNewSubPath(state.normalizedToPixel({ firstX, firstY }));

        for (int i = 0; i < numPoints - 1; ++i)
        {
            auto p1 = points.getChild(i);
            auto p2 = points.getChild(i + 1);

            float x1 = getTrueX(p1);
            float y1 = getTrueY(p1);
            float x2 = getTrueX(p2);
            float y2 = getTrueY(p2);

            float curve = 0.5f;
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
                cachedCurvePath.lineTo(state.normalizedToPixel({ x, y }));
            }
        }

        cachedFillPath = cachedCurvePath;
        auto lastNode = points.getChild(numPoints - 1);
        cachedFillPath.lineTo(state.normalizedToPixel({ getTrueX(lastNode), 0.0f }));
        cachedFillPath.lineTo(state.normalizedToPixel({ firstX, 0.0f }));
        cachedFillPath.closeSubPath();

        float smoothValue = (float)envelope.getProperty("smooth", 0.0) / 500.0f;
        if (smoothValue <= 0.001f)
        {
            cacheValid = true;
            return;
        }

        auto evaluateY = [&](float xPos) -> float {
            if (xPos <= getTrueX(points.getChild(0))) return getTrueY(points.getChild(0));
            if (xPos >= getTrueX(points.getChild(numPoints - 1))) return getTrueY(points.getChild(numPoints - 1));

            for (int i = 0; i < numPoints - 1; ++i)
            {
                auto p1 = points.getChild(i);
                auto p2 = points.getChild(i + 1);
                float x1 = getTrueX(p1);
                float x2 = getTrueX(p2);

                if (xPos >= x1 && xPos <= x2)
                {
                    float y1 = getTrueY(p1);
                    float y2 = getTrueY(p2);

                    float curve = 0.5f;
                    CurveType type = CurveType::Exponential;
                    if (segments.isValid() && i < segments.getNumChildren())
                    {
                        auto sNode = segments.getChild(i);
                        curve = (sNode == activeAnchorNode) ? activeDragCurve : (float)sNode["curve"];
                        type = (CurveType)(int)sNode["type"];
                    }

                    float t = (x2 > x1) ? (xPos - x1) / (x2 - x1) : 0.0f;
                    float shapedT = EnvelopeCurves::applyCurve(t, curve, type);
                    return juce::jmap(shapedT, y1, y2);
                }
            }
            return 0.0f;
        };

        float rawRate = (float)envelope.getProperty("rate", 20.0);
        bool isFreq = (bool)envelope.getProperty("rateIsFrequencyMode", true);
        float smoothMs = (float)envelope.getProperty("smooth", 0.0);
        double effectiveRate = EnvelopeSmoothing::effectiveCyclesPerSecond(rawRate, isFreq);

        // Use the exact same steady-state smoothing the DSP uses, so the drawn
        // line and the audio gain are identical.
        const int smoothSteps = 400;
        std::vector<float> smoothed;
        std::function<float(float)> eval = evaluateY;
        EnvelopeSmoothing::computeSteadyStateSmoothing(eval, smoothMs, effectiveRate, smoothed, smoothSteps);

        cachedSmoothPath.startNewSubPath(state.normalizedToPixel({ 0.0f, smoothed[0] }));
        for (int i = 1; i <= smoothSteps; ++i)
        {
            float x = (float)i / smoothSteps;
            cachedSmoothPath.lineTo(state.normalizedToPixel({ x, smoothed[(size_t)i] }));
        }

        hasSmoothOverlay = true;
        cacheValid = true;
    }

    GridViewState state;
    juce::ValueTree envelope;
    juce::ValueTree activeDragNode;
    juce::Point<float> activeDragPosition;
    juce::ValueTree activeAnchorNode;
    float activeDragCurve = 0.0f;
    bool cacheDirty = true;
    bool cacheValid = false;
    bool hasSmoothOverlay = false;
    juce::Path cachedCurvePath;
    juce::Path cachedFillPath;
    juce::Path cachedSmoothPath;
};
