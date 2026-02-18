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
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        auto angle = rotaryStartAngle +
            sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // --- Outer Ring (Shadow/Glow) ---
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        // --- Base Circle Gradient ---
        juce::ColourGradient baseGrad(juce::Colour(0xff2a2a2e), centreX, centreY - radius,
                                      juce::Colour(0xff121214), centreX, centreY + radius, false);
        g.setGradientFill(baseGrad);
        g.fillEllipse(centreX - radius + 1.0f, centreY - radius + 1.0f, (radius - 1.0f) * 2.0f, (radius - 1.0f) * 2.0f);

        // Inner bevel highlight
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawEllipse(centreX - radius + 2.0f, centreY - radius + 2.0f, (radius - 2.0f) * 2.0f, (radius - 2.0f) * 2.0f, 1.0f);

        // --- Arcs Area ---
        float arcRadius = radius - 3.5f;
        float thickness = 2.5f;

        // Background arc (Track)
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff0a0a0c));
        g.strokePath(bgArc, juce::PathStrokeType(thickness + 0.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc (Active)
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
        
        auto accentColour = juce::Colour(0xff4cc9f0); // Cyan/Blue accent
        g.setColour(accentColour);
        g.strokePath(valueArc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Subtle Glow on value arc
        g.setColour(accentColour.withAlpha(0.15f));
        g.strokePath(valueArc, juce::PathStrokeType(thickness + 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // --- Modern Indicator (Dot) ---
        float dotRadius = 1.5f;
        float dotDist = radius - 8.0f;
        
        juce::Point<float> dotPos (
            centreX + dotDist * std::cos(angle - juce::MathConstants<float>::halfPi),
            centreY + dotDist * std::sin(angle - juce::MathConstants<float>::halfPi)
        );

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(dotPos.x - dotRadius, dotPos.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
};
