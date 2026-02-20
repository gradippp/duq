#pragma once

#include "SettingsPageBase.h"
#include "../../utils/ComboBoxLookAndFeel.h"

class GeneralPage : public SettingsPageBase,
                    private juce::ComboBox::Listener,
                    private juce::Button::Listener
{
public:
    GeneralPage();
    ~GeneralPage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void comboBoxChanged(juce::ComboBox* cb) override;
    void buttonClicked(juce::Button* b) override;

    juce::ComboBox waveformQualityCombo;
    juce::ToggleButton tooltipsToggle{ "Show Tooltips" };
    juce::ComboBox themeCombo;

    ComboBoxLookAndFeel comboBoxLNF;
    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneralPage)
};
