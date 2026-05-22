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

MeterComponent::~MeterComponent()
{
    stopTimer();
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

    // Peak Logic
    if (normalized >= peakLevel)
    {
        peakLevel = normalized;
        peakHoldCount = clipHoldFrames;
    }
    else if (peakHoldCount > 0)
    {
        peakHoldCount--;
    }
    else
    {
        peakLevel *= 0.95f; // Faster decay for peak when not held
    }

    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // ----- Layout -----
    const float labelWidth = 100.0f;
    auto labelArea = bounds.removeFromLeft(labelWidth).reduced(8.0f, 0.0f);
    auto meterBounds = bounds.reduced(8.0f, 6.0f); // More vertical padding for a slimmer, more modern look

    // ----- Draw Label -----
    if (labelText.isNotEmpty())
    {
        g.setColour(T_COL(textLabel));
        g.setFont(FontManager::getBarlowBold(14.0f));
        g.drawFittedText(labelText.toUpperCase(),
            labelArea.toNearestInt(),
            juce::Justification::centredLeft,
            1);
    }

    // ----- Meter Track (Background) -----
    g.setColour(T_COL(sectionBackground));
    g.fillRoundedRectangle(meterBounds, 2.0f);
    
    g.setColour(T_COL(border).withAlpha(0.3f));
    g.drawRoundedRectangle(meterBounds, 2.0f, 1.0f);

    // ----- Filled Meter -----
    if (smoothedLevel > 0.001f || peakLevel > 0.001f)
    {
        auto fillArea = meterBounds.reduced(2.0f); // Leave a small gap from border
        float maxAvailableWidth = fillArea.getWidth();
        float levelWidth = maxAvailableWidth * smoothedLevel;
        float peakX = fillArea.getX() + (maxAvailableWidth * peakLevel);
        
        if (meterDirection == Direction::RightToLeft)
        {
            peakX = fillArea.getRight() - (maxAvailableWidth * peakLevel);
        }

        // --- Main Fill ---
        if (smoothedLevel > 0.001f)
        {
            auto currentFill = fillArea;
            if (meterDirection == Direction::LeftToRight)
            {
                currentFill = fillArea.withWidth(levelWidth);
            }
            else
            {
                currentFill = fillArea.withLeft(fillArea.getRight() - levelWidth);
            }

            juce::Colour baseColor = (mode == MeterMode::Envelope) ? 
                                    T_COL(meterReduction) : 
                                    T_COL(meterFill);

            juce::ColourGradient grad(baseColor.withAlpha(0.6f), fillArea.getX(), 0,
                                      baseColor, fillArea.getRight(), 0, false);
            
            if (mode == MeterMode::AudioLevel)
            {
                grad.addColour(0.7, baseColor);
                grad.addColour(0.9, T_COL(meterReduction));
            }

            g.setGradientFill(grad);
            g.fillRoundedRectangle(currentFill, 1.5f);

            // Subtle Glow at the tip
            g.setColour(baseColor.withAlpha(0.3f));
            if (meterDirection == Direction::LeftToRight)
                g.fillRoundedRectangle(currentFill.withLeft(currentFill.getRight() - 2.0f).expanded(1.0f, 2.0f), 1.0f);
            else
                g.fillRoundedRectangle(currentFill.withWidth(2.0f).expanded(1.0f, 2.0f), 1.0f);
        }

        // --- Peak Indicator ---
        if (peakLevel > 0.001f)
        {
            g.setColour(T_COL(accent).withAlpha(0.8f));
            g.fillRect(peakX - 1.0f, fillArea.getY(), 2.0f, fillArea.getHeight());
        }
    }

    // ----- Modern Tick Marks -----
    g.setColour(T_COL(border).withAlpha(0.5f));
    const int numTicks = 10;
    for (int i = 1; i < numTicks; ++i)
    {
        float x = meterBounds.getX() + (meterBounds.getWidth() * (float)i / (float)numTicks);
        g.drawVerticalLine(juce::roundToInt(x), meterBounds.getBottom() - 3.0f, meterBounds.getBottom());
        g.drawVerticalLine(juce::roundToInt(x), meterBounds.getY(), meterBounds.getY() + 3.0f);
    }
}


