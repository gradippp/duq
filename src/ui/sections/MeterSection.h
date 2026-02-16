#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>
#include "../components/MeterComponent.h"

class MeterSection : public juce::Component
{
public:
    MeterSection();

    void resized() override;

    // expose level setters (temporary until processor wiring)
    void setInputLevel(float value);
    void setReductionLevel(float value);
    void setOutputLevel(float value);

private:
    // ===== Atomic Level Sources =====
    std::atomic<float> inputLevel{ 0.0f };
    std::atomic<float> reductionLevel{ 0.0f };
    std::atomic<float> outputLevel{ 0.0f };

    // ===== Meter Components =====
    MeterComponent inputMeter;
    MeterComponent reductionMeter;
    MeterComponent outputMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterSection)
};
