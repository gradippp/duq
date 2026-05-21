#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GlobalLookAndFeel.h"

class ViewportLookAndFeel : public GlobalLookAndFeel
{
public:
    ViewportLookAndFeel()
    {
        refreshColours();
        setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ScrollBar::thumbColourId, T_COL(accent).withAlpha(0.15f));
        setColour(juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
    }

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                       int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition,
                       int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        auto thumbArea = isScrollbarVertical 
            ? juce::Rectangle<int>(x + 2, thumbStartPosition, width - 4, thumbSize)
            : juce::Rectangle<int>(thumbStartPosition, y + 2, thumbSize, height - 4);

        auto alpha = isMouseDown ? 0.4f : (isMouseOver ? 0.25f : 0.15f);
        g.setColour(T_COL(accent).withAlpha(alpha));
        g.fillRoundedRectangle(thumbArea.toFloat(), thumbArea.getWidth() * 0.5f);
    }

    void drawScrollbarButton(juce::Graphics&, juce::ScrollBar&, int, int, int, bool, bool, bool) override
    {
        // Don't draw any buttons for a minimal look
    }

    bool areScrollbarButtonsVisible() override { return false; }
};
