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

        juce::Path fillPath = path;
        auto lastNode = points.getChild(numPoints - 1);
        float fillLastX = (lastNode == activeDragNode) ? activeDragPosition.x : (float)lastNode["x"];
        float fillFirstX = (firstNode == activeDragNode) ? activeDragPosition.x : (float)firstNode["x"];

        fillPath.lineTo(state.normalizedToPixel({ fillLastX, 0.0f }));
        fillPath.lineTo(state.normalizedToPixel({ fillFirstX, 0.0f }));
        fillPath.closeSubPath();

        juce::ColourGradient grad(T_COL(envelopeFillTop), 0, (float)state.viewArea.getY(),
                                  T_COL(envelopeFillBot), 0, (float)state.viewArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);

        // --- Smooth Overlay ---
        float smoothValue = (float)envelope.getProperty("smooth", 0.0) / 100.0f;
        if (smoothValue > 0.001f)
        {
            auto getTrueX = [&](juce::ValueTree p) -> float {
                return (p == activeDragNode) ? activeDragPosition.x : (float)p["x"];
            };
            auto getTrueY = [&](juce::ValueTree p) -> float {
                return (p == activeDragNode) ? activeDragPosition.y : (float)p["y"];
            };

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
            double currentRate = rawRate;
            if (!isFreq)
            {
                static const double cycleMultipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
                int idx = juce::jlimit(0, 5, (int)(rawRate / 16.66f));
                currentRate = cycleMultipliers[idx] * 2.0; // Assume 120 bpm
            }
            if (currentRate <= 0.0) currentRate = 1.0;
            double durationSec = 1.0 / currentRate;

            const int smoothSteps = 400; 
            float smoothTimeSec = smoothValue * 0.1f; // 100ms max
            
            double visualSrate = smoothSteps / durationSec;
            float coeff = 1.0f - std::exp(-1.0f / (smoothTimeSec * (float)visualSrate));

            // Run simulation for multiple cycles to reach steady-state
            float currentSmoothY = 1.0f;
            
            // Pass 1: Settle (Run for 10 cycles to handle heavy smoothing at fast rates)
            for (int cycle = 0; cycle < 10; ++cycle)
            {
                for (int i = 0; i <= smoothSteps; ++i)
                {
                    float x = (float)i / smoothSteps;
                    currentSmoothY += (evaluateY(x) - currentSmoothY) * coeff;
                }
            }

            // Pass 2: Draw (The final cycle)
            juce::Path smoothPath;
            smoothPath.startNewSubPath(state.normalizedToPixel({ 0.0f, currentSmoothY }));

            for (int i = 1; i <= smoothSteps; ++i)
            {
                float x = (float)i / smoothSteps;
                float targetY = evaluateY(x);
                currentSmoothY += (targetY - currentSmoothY) * coeff;
                smoothPath.lineTo(state.normalizedToPixel({ x, currentSmoothY }));
            }

            g.setColour(T_COL(accent).withAlpha(0.35f));
            g.strokePath(smoothPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
        }

        g.setColour(T_COL(envelopeLine));
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

private:
    GridViewState state;
    juce::ValueTree envelope;
    juce::ValueTree activeDragNode;
    juce::Point<float> activeDragPosition;
    juce::ValueTree activeAnchorNode;
    float activeDragCurve = 0.0f;
};
