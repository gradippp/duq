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
}

GeneralPage::~GeneralPage() 
{
    waveformQualityCombo.setLookAndFeel(nullptr);
}

void GeneralPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    drawControlLabel(g, "Waveform Quality", waveformQualityCombo.getBounds());
}

void GeneralPage::resized()
{
    const int startY = 80;
    const int rowHeight = 30;
    const int spacingY = 40;

    auto area = getLocalBounds().withTrimmedTop(startY).reduced(20, 0);
    
    // Row 1: Waveform Quality
    auto row1 = area.removeFromTop(rowHeight);
    waveformQualityCombo.setBounds(row1.removeFromLeft(150));

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
