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
    const int startY = 80;
    const int rowHeight = 30;
    const int spacingY = 40;

    auto area = getLocalBounds().withTrimmedTop(startY).reduced(20, 0);
    
    // Row 1: Waveform Quality and UI Theme
    auto row1 = area.removeFromTop(rowHeight);
    waveformQualityCombo.setBounds(row1.removeFromLeft(150));
    row1.removeFromLeft(40);
    themeCombo.setBounds(row1.removeFromLeft(150));

    area.removeFromTop(spacingY);

    // Row 2: Tooltips
    tooltipsToggle.setBounds(area.removeFromTop(rowHeight));
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
