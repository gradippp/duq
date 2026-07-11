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

        juce::ColourGradient grad(Theme::get(ThemeManager::envelopeFillTop), 0, (float)state.viewArea.getY(),
                                  Theme::get(ThemeManager::envelopeFillBot), 0, (float)state.viewArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillPath(cachedFillPath);

        if (hasSmoothOverlay)
        {
            g.setColour(Theme::get(ThemeManager::accent).withAlpha(0.35f));
            g.strokePath(cachedSmoothPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
        }

        g.setColour(Theme::get(ThemeManager::envelopeLine));
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

        // Snapshot points/segments into POD arrays once (folding in the active
        // drag/anchor overrides). The smoothing pass below calls evaluateY ~4400
        // times per rebuild, i.e. on every drag mouse-move; reading plain floats
        // here avoids that many ValueTree var-conversions.
        struct PtSnap { float x, y; };
        struct SegSnap { float curve; CurveType type; };

        std::vector<PtSnap> pts;
        pts.reserve((size_t)numPoints);
        for (int i = 0; i < numPoints; ++i)
        {
            auto p = points.getChild(i);
            float x = (p == activeDragNode) ? activeDragPosition.x : (float)p["x"];
            float y = (p == activeDragNode) ? activeDragPosition.y : (float)p["y"];
            pts.push_back({ x, y });
        }

        auto segments = envelope.getChildWithName("SEGMENTS");
        std::vector<SegSnap> segs;
        segs.reserve((size_t)std::max(0, numPoints - 1));
        for (int i = 0; i < numPoints - 1; ++i)
        {
            float curve = 0.5f;
            CurveType type = CurveType::Exponential;
            if (segments.isValid() && i < segments.getNumChildren())
            {
                auto sNode = segments.getChild(i);
                curve = (sNode == activeAnchorNode) ? activeDragCurve : (float)sNode["curve"];
                type = (CurveType)(int)sNode["type"];
            }
            segs.push_back({ curve, type });
        }

        float firstX = pts[0].x;
        float firstY = pts[0].y;

        cachedCurvePath.startNewSubPath(state.normalizedToPixel({ firstX, firstY }));

        for (int i = 0; i < numPoints - 1; ++i)
        {
            float x1 = pts[(size_t)i].x, y1 = pts[(size_t)i].y;
            float x2 = pts[(size_t)i + 1].x, y2 = pts[(size_t)i + 1].y;
            float curve = segs[(size_t)i].curve;
            CurveType type = segs[(size_t)i].type;

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
        cachedFillPath.lineTo(state.normalizedToPixel({ pts.back().x, 0.0f }));
        cachedFillPath.lineTo(state.normalizedToPixel({ firstX, 0.0f }));
        cachedFillPath.closeSubPath();

        float smoothValue = (float)envelope.getProperty("smooth", 0.0) / 500.0f;
        if (smoothValue <= 0.001f)
        {
            cacheValid = true;
            return;
        }

        auto evaluateY = [&](float xPos) -> float {
            if (xPos <= pts.front().x) return pts.front().y;
            if (xPos >= pts.back().x) return pts.back().y;

            for (int i = 0; i < numPoints - 1; ++i)
            {
                float x1 = pts[(size_t)i].x;
                float x2 = pts[(size_t)i + 1].x;

                if (xPos >= x1 && xPos <= x2)
                {
                    float y1 = pts[(size_t)i].y;
                    float y2 = pts[(size_t)i + 1].y;
                    float t = (x2 > x1) ? (xPos - x1) / (x2 - x1) : 0.0f;
                    float shapedT = EnvelopeCurves::applyCurve(t, segs[(size_t)i].curve, segs[(size_t)i].type);
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
