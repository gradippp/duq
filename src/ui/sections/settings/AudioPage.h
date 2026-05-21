#pragma once

#include "SettingsPageBase.h"

class AudioPage : public SettingsPageBase,
                  private juce::Button::Listener,
                  private juce::Slider::Listener
{
public:
    AudioPage();
    ~AudioPage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;

private:
    void buttonClicked(juce::Button* b) override;
    void sliderValueChanged(juce::Slider* s) override;

    juce::ToggleButton latencyToggle{ "High-Precision Latency Mode (High CPU)" };
    juce::Slider globalMixSlider;
    juce::Label sampleRateLabel;

    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPage)
};
