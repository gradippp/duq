#include "MeterComponent.h"
#include "../utils/FontManager.h"
#include "../../Globals.h"

MeterComponent::MeterComponent(std::atomic<float>& source,
    Direction dir,
    const juce::String& label)
    : inputLevel(source),
    meterDirection(dir),
    labelText(label)
{
    startTimerHz(60);
}

void MeterComponent::setMode(MeterMode newMode)
{
    mode = newMode;
}

void MeterComponent::setLabel(const juce::String& newLabel)
{
    labelText = newLabel;
    repaint();
}

void MeterComponent::timerCallback()
{
    float value = juce::jlimit(0.0f, 1.0f, inputLevel.load());

    float normalized = value;

    if (mode == MeterMode::AudioLevel)
    {
        float linear = juce::jlimit(0.000001f, 1.0f, value);
        float db = juce::Decibels::gainToDecibels(linear, -60.0f);
        normalized = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (mode == MeterMode::Envelope)
    {
        // Direct linear mapping for reduction
        normalized = value;
    }

    normalized = juce::jlimit(0.0f, 1.0f, normalized);

    // Faster smoothing to catch blips
    smoothedLevel += (normalized - smoothedLevel) * 0.4f;

    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // ----- Layout -----
    const float labelWidth = 100.0f;
    auto labelArea = bounds.removeFromLeft(labelWidth).reduced(8.0f, 0.0f);
    // Taller meter: reduce the vertical padding (from 6.0f to 2.0f)
    auto meterBounds = bounds.reduced(8.0f, 2.0f);

    // ----- Draw Label -----
    if (labelText.isNotEmpty())
    {
        g.setColour(Theme::Colours::textLabel);
        g.setFont(FontManager::getBarlowBold(15.0f));
        g.drawFittedText(labelText.toUpperCase(),
            labelArea.toNearestInt(),
            juce::Justification::centredLeft,
            1);
    }

    // ----- Meter Track (Background) -----
    // Very dark background for the bar track to provide high contrast
    g.setColour(Theme::Colours::meterBackground.withAlpha(0.6f));
    g.fillRoundedRectangle(meterBounds, 2.0f);

    // ----- Filled Meter -----
    if (smoothedLevel > 0.001f)
    {
        auto fillArea = meterBounds;
        float levelWidth = meterBounds.getWidth() * smoothedLevel;
        
        if (meterDirection == Direction::LeftToRight)
        {
            fillArea = meterBounds.withWidth(levelWidth);
        }
        else
        {
            fillArea = meterBounds.withLeft(meterBounds.getRight() - levelWidth);
        }

        juce::ColourGradient grad;
        if (mode == MeterMode::Envelope)
        {
            // Modern Amber/Orange for envelope/reduction
            grad = juce::ColourGradient(Theme::Colours::meterReduction.withMultipliedSaturation(0.8f), meterBounds.getX(), 0,
                                        Theme::Colours::meterReduction, meterBounds.getRight(), 0, false);
        }
        else
        {
            // Modern Cyan -> Green -> Red for audio levels
            grad = juce::ColourGradient(juce::Colour(0xff2ec4b6), meterBounds.getX(), 0,
                                        Theme::Colours::meterFill, meterBounds.getRight(), 0, false);
            grad.addColour(0.6, juce::Colour(0xffcbf3f0));
            grad.addColour(0.8, Theme::Colours::meterReduction);
        }

        g.setGradientFill(grad);
        g.fillRoundedRectangle(fillArea, 2.0f);

        // Subtle Glow
        g.setColour(grad.getColourAtPosition(smoothedLevel).withAlpha(0.15f));
        g.fillRoundedRectangle(fillArea.expanded(1.0f), 2.0f);
    }

    // ----- Modern LED Overlay (Grid) -----
    // This gives it a "digital hardware" feel without being too chunky
    g.setColour(Theme::Colours::background.withAlpha(0.8f));
    constexpr int gridCount = 40;
    float gridStep = meterBounds.getWidth() / gridCount;
    for (int i = 1; i < gridCount; ++i)
    {
        float x = meterBounds.getX() + i * gridStep;
        g.drawVerticalLine((int)x, meterBounds.getY(), meterBounds.getBottom());
    }

    // ----- Glass Highlight -----
    auto highlightArea = meterBounds.withHeight(meterBounds.getHeight() * 0.4f);
    g.setGradientFill(juce::ColourGradient(Theme::Colours::accent.withAlpha(0.05f), 0, highlightArea.getY(),
                                           juce::Colours::transparentWhite, 0, highlightArea.getBottom(), false));
    g.fillRoundedRectangle(highlightArea, 2.0f);

    // ----- Border -----
    g.setColour(Theme::Colours::border.withAlpha(0.4f));
    g.drawRoundedRectangle(meterBounds, 2.0f, 1.0f);
}

