#include "MeterSection.h"

MeterSection::MeterSection(std::atomic<float>& inputSource,
    std::atomic<float>& reductionSource,
    std::atomic<float>& outputSource)
    : inputMeter(inputSource,
        MeterComponent::Direction::LeftToRight,
        "Input Gain"),

    reductionMeter(reductionSource,
        MeterComponent::Direction::RightToLeft,
        "Reduction"),

    outputMeter(outputSource,
        MeterComponent::Direction::LeftToRight,
        "Output Gain")
{
    inputMeter.setMode(MeterComponent::MeterMode::AudioLevel);
    outputMeter.setMode(MeterComponent::MeterMode::AudioLevel);
    reductionMeter.setMode(MeterComponent::MeterMode::Envelope);

    addAndMakeVisible(inputMeter);
    addAndMakeVisible(reductionMeter);
    addAndMakeVisible(outputMeter);
}

void MeterSection::resized()
{
    auto area = getLocalBounds().reduced(6);

    const int meterHeight = area.getHeight() / 3;

    inputMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
    reductionMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
    outputMeter.setBounds(area.removeFromTop(meterHeight).reduced(0, 4));
}