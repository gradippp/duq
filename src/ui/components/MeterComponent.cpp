#include "MeterComponent.h"

MeterComponent::MeterComponent(std::atomic<float>& source,
    Direction dir)
    : inputLevel(source),
    meterDirection(dir),
    gradient(juce::Colours::green, 0.0f, 0.0f,
        juce::Colours::red, 100.0f, 0.0f,
        false)
{
    startTimerHz(60); // 60 FPS repaint
}

void MeterComponent::setGradient(const juce::ColourGradient& newGradient)
{
    gradient = newGradient;
    repaint();
}

void MeterComponent::timerCallback()
{
    float target = inputLevel.load();

    // clamp
    target = juce::jlimit(0.0f, 1.0f, target);

    // simple smoothing
    smoothedLevel += (target - smoothedLevel) * 0.15f;

    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colours::black);
    g.fillRect(bounds);

    float width = bounds.getWidth();
    float fillWidth = width * smoothedLevel;

    juce::Rectangle<float> meterArea;

    if (meterDirection == Direction::LeftToRight)
    {
        meterArea = { bounds.getX(),
                      bounds.getY(),
                      fillWidth,
                      bounds.getHeight() };
    }
    else
    {
        meterArea = { bounds.getRight() - fillWidth,
                      bounds.getY(),
                      fillWidth,
                      bounds.getHeight() };
    }

    gradient.point1 = { meterArea.getX(), 0.0f };
    gradient.point2 = { meterArea.getRight(), 0.0f };

    g.setGradientFill(gradient);
    g.fillRect(meterArea);

    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1.0f);
}
