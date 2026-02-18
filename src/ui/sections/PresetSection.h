#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PresetSection : public juce::Component
{
public:
    PresetSection();
    ~PresetSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTargetEnvelope(juce::ValueTree envelope);

    std::function<void()> onClose;

private:
    juce::ValueTree targetEnvelope;

    juce::DrawableButton closeButton{ "close", juce::DrawableButton::ImageFitted };
    juce::Label titleLabel{ "title", "SELECT PRESET" };

    // Placeholder for preset list
    juce::ListBox presetList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSection)
};
