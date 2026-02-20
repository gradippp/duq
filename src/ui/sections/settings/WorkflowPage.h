#pragma once

#include "SettingsPageBase.h"
#include "../../utils/ComboBoxLookAndFeel.h"

class WorkflowPage : public SettingsPageBase,
                     private juce::Slider::Listener,
                     private juce::ComboBox::Listener
{
public:
    WorkflowPage();
    ~WorkflowPage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void sliderValueChanged(juce::Slider* s) override;
    void comboBoxChanged(juce::ComboBox* cb) override;

    juce::Slider snapSensitivitySlider;
    juce::ComboBox defaultCurveCombo;
    juce::Slider undoLimitSlider;

    ComboBoxLookAndFeel comboBoxLNF;
    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkflowPage)
};
