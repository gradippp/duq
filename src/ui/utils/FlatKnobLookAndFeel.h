#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class FlatKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FlatKnobLookAndFeel() = default;
    ~FlatKnobLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto radius = juce::jmin(width, height) * 0.5f - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;

        auto angle = rotaryStartAngle +
            sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Base circle
        g.setColour(juce::Colour(30, 36, 42));
        g.fillEllipse(centreX - radius,
            centreY - radius,
            radius * 2.0f,
            radius * 2.0f);

        // Background arc
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY,
            radius - 4.0f,
            radius - 4.0f,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        g.setColour(juce::Colour(55, 65, 75));
        g.strokePath(bgArc, juce::PathStrokeType(2.0f));

        // Value arc
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY,
            radius - 4.0f,
            radius - 4.0f,
            0.0f,
            rotaryStartAngle,
            angle,
            true);

        g.setColour(juce::Colour(60, 170, 255));
        g.strokePath(valueArc, juce::PathStrokeType(3.0f));

        // Indicator line
        juce::Path indicator;
        indicator.addRectangle(-1.5f, -radius + 6.0f,
            3.0f, radius * 0.5f);

        g.setColour(juce::Colours::white);
        g.fillPath(indicator,
            juce::AffineTransform::rotation(angle)
            .translated(centreX, centreY));
    }
};
