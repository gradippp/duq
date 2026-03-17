#pragma once

#include "SettingsPageBase.h"
#include "../../utils/ThemeManager.h"
#include "../../utils/ThemeEditor.h"
#include "../../utils/ComboBoxLookAndFeel.h"
#include "../../utils/TextButtonLookAndFeel.h"

class ThemePage : public SettingsPageBase,
                  private juce::ComboBox::Listener,
                  private juce::Button::Listener,
                  private juce::ChangeListener
{
public:
    ThemePage();
    ~ThemePage() override;

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

    juce::ComboBox themeCombo;
    juce::TextButton importButton{ "IMPORT THEME" };
    juce::TextButton exportButton{ "EXPORT CURRENT" };
    juce::TextButton resetButton{ "RESET TO DEFAULT" };

    ThemeEditor editor;

    ComboBoxLookAndFeel comboLNF;
    TextButtonLookAndFeel buttonLNF;
    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemePage)
};
