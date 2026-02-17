#include "MeterComponent.h"

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

    normalized = juce::jlimit(0.0f, 1.0f, normalized);

    smoothedLevel += (normalized - smoothedLevel) * 0.15f;

    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colours::black);

    // ----- Layout -----
    const float labelWidth = 90.0f;

    auto labelArea = bounds.removeFromLeft(labelWidth);
    auto meterBounds = bounds.reduced(4.0f);

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

    // ----- LED Meter -----
    constexpr int ledCount = 20;
    constexpr float ledGap = 2.0f;

    float meterWidth = meterBounds.getWidth();
    float meterHeight = meterBounds.getHeight();

    float ledWidth =
        (meterWidth - (ledGap * (ledCount - 1))) / ledCount;

    for (int i = 0; i < ledCount; ++i)
    {
        float ledStartX =
            meterBounds.getX() +
            i * (ledWidth + ledGap);

        float ledThreshold =
            (float)(i + 1) / ledCount;

        bool isLit = smoothedLevel >= ledThreshold;

        juce::Colour ledColour;

        // Envelope mode = single colour
        if (mode == MeterMode::Envelope)
        {
            ledColour = juce::Colours::orange;
        }
        else
        {
            // Audio level zones
            if (ledThreshold > 0.8f)
                ledColour = juce::Colours::red;
            else if (ledThreshold > 0.6f)
                ledColour = juce::Colours::yellow;
            else
                ledColour = juce::Colours::green;
        }

        if (!isLit)
            ledColour = ledColour.darker(0.8f);

        g.setColour(ledColour);

        g.fillRoundedRectangle(
            ledStartX,
            meterBounds.getY(),
            ledWidth,
            meterHeight,
            2.0f);
    }

    // ----- Border -----
    g.setColour(juce::Colours::grey);
    g.drawRect(meterBounds, 1.0f);
}

