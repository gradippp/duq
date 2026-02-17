#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>
#include "../components/MeterComponent.h"

class MeterSection : public juce::Component
{
public:
    MeterSection(std::atomic<float>& inputSource,
        std::atomic<float>& reductionSource,
        std::atomic<float>& outputSource);

    void resized() override;

private:
    // ===== Meter Components =====
    MeterComponent inputMeter;
    MeterComponent reductionMeter;
    MeterComponent outputMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterSection)
};
