#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AnchorComponent : public juce::Component
{
public:
    AnchorComponent(GridSection& g)
        : grid(g)
    {
        setSize(8, 8);
    }

    void setNormalizedPosition(juce::Point<float> p)
    {
        normalized = p;
        auto pixel = grid.normalizedToPixel(p);
        setCentrePosition((int)pixel.x, (int)pixel.y);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::orange);
        g.drawEllipse(getLocalBounds().toFloat(), 2.0f);
    }

private:
    GridSection& grid;
    juce::Point<float> normalized;
};
