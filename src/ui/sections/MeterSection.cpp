#include "MeterSection.h"

#include "MeterSection.h"

MeterSection::MeterSection()
    : inputMeter(inputLevel,
        MeterComponent::Direction::LeftToRight,
        "Input Gain"),

    reductionMeter(reductionLevel,
        MeterComponent::Direction::RightToLeft,
        "Reduction"),

    outputMeter(outputLevel,
        MeterComponent::Direction::LeftToRight,
        "Output Gain")
{
    inputMeter.setMode(MeterComponent::MeterMode::AudioLevel);
    outputMeter.setMode(MeterComponent::MeterMode::AudioLevel);

    reductionMeter.setMode(MeterComponent::MeterMode::Envelope);

    addAndMakeVisible(inputMeter);
    addAndMakeVisible(reductionMeter);
    addAndMakeVisible(outputMeter);

    // Optional gradient setup
    //inputMeter.setGradient(
    //    juce::ColourGradient(juce::Colours::green, 0, 0,
    //        juce::Colours::red, 100, 0,
    //        false));

    //reductionMeter.setGradient(
    //    juce::ColourGradient(juce::Colours::cyan, 0, 0,
    //        juce::Colours::blue, 100, 0,
    //        false));

    //outputMeter.setGradient(
    //    juce::ColourGradient(juce::Colours::green, 0, 0,
    //        juce::Colours::red, 100, 0,
    //        false));
}

void MeterSection::resized()
{
    auto area = getLocalBounds().reduced(6);

    const int meterHeight = area.getHeight() / 3;

    inputMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
    reductionMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
    outputMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
}

void MeterSection::setInputLevel(float value)
{
    inputLevel.store(value);
}

void MeterSection::setReductionLevel(float value)
{
    reductionLevel.store(value);
}

void MeterSection::setOutputLevel(float value)
{
    outputLevel.store(value);
}