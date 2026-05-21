#pragma once

#include "SettingsPageBase.h"
#include "../../utils/ComboBoxLookAndFeel.h"
#include "../../utils/ThemeManager.h"
#include "../../utils/ThemeEditor.h"
#include "../../utils/TextButtonLookAndFeel.h"

class GeneralPage : public SettingsPageBase,
                    private juce::ComboBox::Listener,
                    private juce::Button::Listener,
                    private juce::ChangeListener
{
public:
    GeneralPage();
    ~GeneralPage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    int getRequiredHeight() override;

private:
    void comboBoxChanged(juce::ComboBox* cb) override;
    void buttonClicked(juce::Button* b) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void refreshThemeList();
    void loadSelectedTheme();
    void importTheme();
    void exportTheme();

    juce::ComboBox waveformQualityCombo;
    juce::ToggleButton showSourceToggle{ "View Source Signal" };
    juce::ToggleButton showSidechainToggle{ "View Sidechain Signal" };
    juce::ToggleButton tooltipsToggle{ "Show Tooltips" };
    
    juce::ComboBox themeCombo;
    juce::TextButton importButton{ "IMPORT" };
    juce::TextButton exportButton{ "EXPORT" };
    juce::TextButton resetButton{ "RESET" };

    ThemeEditor editor;

    ComboBoxLookAndFeel comboBoxLNF;
    TextButtonLookAndFeel buttonLNF;
    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneralPage)
};
