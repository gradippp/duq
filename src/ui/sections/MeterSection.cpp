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
    auto area = getLocalBounds().reduced(4);

    constexpr int gap = 8; // reduced gap to give more room to the bars
    const int totalGap = gap * 2;

    const int meterHeight =
        (area.getHeight() - totalGap) / 3;

    inputMeter.setBounds(
        area.removeFromTop(meterHeight));

    area.removeFromTop(gap);

    reductionMeter.setBounds(
        area.removeFromTop(meterHeight));

    area.removeFromTop(gap);

    outputMeter.setBounds(
        area.removeFromTop(meterHeight));
}
