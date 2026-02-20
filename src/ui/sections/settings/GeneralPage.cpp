#include "GeneralPage.h"

GeneralPage::GeneralPage()
    : SettingsPageBase("UI & VISUALS")
{
    waveformQualityCombo.setLookAndFeel(&comboBoxLNF);
    waveformQualityCombo.addItemList({"Low", "Medium", "High"}, 1);
    waveformQualityCombo.setSelectedItemIndex(config->getWaveformQuality());
    waveformQualityCombo.addListener(this);
    addAndMakeVisible(waveformQualityCombo);

    tooltipsToggle.setToggleState(config->getShowTooltips(), juce::dontSendNotification);
    tooltipsToggle.addListener(this);
    addAndMakeVisible(tooltipsToggle);

    themeCombo.setLookAndFeel(&comboBoxLNF);
    themeCombo.addItemList({"Default Dark", "Slate", "Steel", "Obsidian"}, 1);
    themeCombo.setSelectedItemIndex(0);
    themeCombo.addListener(this);
    addAndMakeVisible(themeCombo);
}

GeneralPage::~GeneralPage() 
{
    waveformQualityCombo.setLookAndFeel(nullptr);
    themeCombo.setLookAndFeel(nullptr);
}

void GeneralPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    drawControlLabel(g, "Waveform Quality", waveformQualityCombo.getBounds());
    drawControlLabel(g, "UI Theme", themeCombo.getBounds());
}

void GeneralPage::resized()
{
    auto area = getContentArea();
    
    waveformQualityCombo.setBounds(area.removeFromTop(30).withWidth(150));
    area.removeFromTop(40);
    tooltipsToggle.setBounds(area.removeFromTop(30));
    area.removeFromTop(40);
    themeCombo.setBounds(area.removeFromTop(30).withWidth(150));
}

void GeneralPage::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &waveformQualityCombo)
        config->setWaveformQuality(waveformQualityCombo.getSelectedItemIndex());
}

void GeneralPage::buttonClicked(juce::Button* b)
{
    if (b == &tooltipsToggle)
    {
        config->setShowTooltips(tooltipsToggle.getToggleState());
    }
}
