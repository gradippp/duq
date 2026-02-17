#include "MeterComponent.h"

MeterComponent::MeterComponent(std::atomic<float>& source,
    Direction dir,
    const juce::String& label)
    : inputLevel(source),
    meterDirection(dir),
    labelText(label),
    gradient(juce::Colours::green, 0.0f, 0.0f,
        juce::Colours::red, 100.0f, 0.0f,
        false)
{
    startTimerHz(60);
}

void MeterComponent::setGradient(const juce::ColourGradient& newGradient)
{
    gradient = newGradient;
    repaint();
}

void MeterComponent::setLabel(const juce::String& newLabel)
{
    labelText = newLabel;
    repaint();
}

void MeterComponent::timerCallback()
{
    float target = juce::jlimit(0.0f, 1.0f, inputLevel.load());

    smoothedLevel += (target - smoothedLevel) * 0.15f;

    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.fillAll(juce::Colours::black);

    // ----- Layout -----
    const float labelWidth = 90.0f; // adjust to taste

    auto labelArea = bounds.removeFromLeft(labelWidth);
    auto meterBounds = bounds;

    // ----- Draw Label -----
    if (labelText.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawFittedText(labelText,
            labelArea.toNearestInt(),
            juce::Justification::centredLeft,
            1);
    }

    // ----- Draw Meter -----
    float width = meterBounds.getWidth();
    float fillWidth = width * smoothedLevel;

    juce::Rectangle<float> meterArea;

    if (meterDirection == Direction::LeftToRight)
    {
        meterArea = { meterBounds.getX(),
                      meterBounds.getY(),
                      fillWidth,
                      meterBounds.getHeight() };
    }
    else
    {
        meterArea = { meterBounds.getRight() - fillWidth,
                      meterBounds.getY(),
                      fillWidth,
                      meterBounds.getHeight() };
    }

    gradient.point1 = { meterBounds.getX(), 0.0f };
    gradient.point2 = { meterBounds.getRight(), 0.0f };

    g.setGradientFill(gradient);
    g.fillRect(meterArea);

    g.setColour(juce::Colours::grey);
    g.drawRect(meterBounds, 1.0f);
}
