#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"

struct GridViewState
{
    float zoomX = 1.0f;
    float zoomY = 1.0f;
    float uniformZoom = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    int gridPower = 4;
    juce::Rectangle<int> viewArea;

    juce::Point<float> normalizedToPixel(juce::Point<float> p) const
    {
        float visibleWidth = 1.0f / uniformZoom;
        float visibleHeight = 1.0f / uniformZoom;

        float nx = (p.x - offsetX) / visibleWidth;
        float ny = (p.y - offsetY) / visibleHeight;

        return {
            viewArea.getX() + nx * viewArea.getWidth(),
            viewArea.getY() + (1.0f - ny) * viewArea.getHeight()
        };
    }

    juce::Point<float> pixelToNormalized(juce::Point<float> p) const
    {
        float visibleWidth = 1.0f / uniformZoom;
        float visibleHeight = 1.0f / uniformZoom;

        float nx = (p.x - viewArea.getX()) / viewArea.getWidth();
        float ny = 1.0f - ((p.y - viewArea.getY()) / viewArea.getHeight());

        float realX = offsetX + nx * visibleWidth;
        float realY = offsetY + ny * visibleHeight;

        return {
            juce::jlimit(0.0f, 1.0f, realX),
            juce::jlimit(0.0f, 1.0f, realY)
        };
    }
};

class GridBackground : public juce::Component
{
public:
    GridBackground()
    {
        setInterceptsMouseClicks(false, false);
    }

    void setViewState(const GridViewState& newState)
    {
        state = newState;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(T_COL(background));

        int divisions = 1 << state.gridPower;
        float baseStep = 1.0f / divisions;

        float visibleWidth = 1.0f / state.uniformZoom;
        float visibleHeight = 1.0f / state.uniformZoom;

        float startX = state.offsetX;
        float endX = state.offsetX + visibleWidth;

        float startY = state.offsetY;
        float endY = state.offsetY + visibleHeight;

        auto viewArea = state.viewArea;

        // Adaptive density
        float pxPerGridX = viewArea.getWidth() * (baseStep / visibleWidth);
        while (pxPerGridX < 8.0f)
        {
            baseStep *= 2.0f;
            pxPerGridX *= 2.0f;
        }

        float pxPerGridY = viewArea.getHeight() * (baseStep / visibleHeight);
        while (pxPerGridY < 8.0f)
        {
            baseStep *= 2.0f;
            pxPerGridY *= 2.0f;
        }

        int firstX = static_cast<int>(std::floor(startX / baseStep));
        int lastX = static_cast<int>(std::ceil(endX / baseStep));

        for (int i = firstX; i <= lastX; ++i)
        {
            float normX = i * baseStep;
            if (normX < 0.0f || normX > 1.0f) continue;

            auto p = state.normalizedToPixel({ normX, 0.0f });
            bool isMajor = (i % 4 == 0);
            g.setColour(isMajor ? T_COL(gridMajor) : T_COL(gridMinor));
            g.drawLine(p.x, (float)viewArea.getY(), p.x, (float)viewArea.getBottom());
        }

        int firstY = static_cast<int>(std::floor(startY / baseStep));
        int lastY = static_cast<int>(std::ceil(endY / baseStep));

        for (int i = firstY; i <= lastY; ++i)
        {
            float normY = i * baseStep;
            if (normY < 0.0f || normY > 1.0f) continue;

            auto p = state.normalizedToPixel({ 0.0f, normY });
            bool isMajor = (i % 4 == 0);
            g.setColour(isMajor ? T_COL(gridMajor) : T_COL(gridMinor));
            g.drawLine((float)viewArea.getX(), p.y, (float)viewArea.getRight(), p.y);
        }
    }

private:
    GridViewState state;
};
