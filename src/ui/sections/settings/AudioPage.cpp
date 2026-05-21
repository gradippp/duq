#include "AudioPage.h"

AudioPage::AudioPage()
    : SettingsPageBase("AUDIO & DSP")
{
    latencyToggle.setToggleState(false, juce::dontSendNotification);
    latencyToggle.addListener(this);
    addAndMakeVisible(latencyToggle);

    globalMixSlider.setRange(0, 100, 1);
    globalMixSlider.setValue(100, juce::dontSendNotification);
    globalMixSlider.addListener(this);
    addAndMakeVisible(globalMixSlider);

    sampleRateLabel.setFont(FontManager::getJetBrainsMono(11.0f));
    sampleRateLabel.setColour(juce::Label::textColourId, T_COL(textDimmed));
    sampleRateLabel.setText("Host Sample Rate: 44.1 kHz", juce::dontSendNotification);
    addAndMakeVisible(sampleRateLabel);
}

AudioPage::~AudioPage() {}

void AudioPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    drawControlLabel(g, "Global Dry/Wet Mix", globalMixSlider.getBounds());
}

void AudioPage::resized()
{
    const int startY = 80;
    const int rowHeight = 30;
    const int spacingY = 40;

    auto area = getLocalBounds().withTrimmedTop(startY).reduced(20, 0);

    // Row 1: Latency Mode
    latencyToggle.setBounds(area.removeFromTop(rowHeight));
    
    area.removeFromTop(spacingY);

    // Row 2: Global Mix
    globalMixSlider.setBounds(area.removeFromTop(rowHeight).withWidth(200));
    
    area.removeFromTop(spacingY + 5);

    // Row 3: Sample Rate
    sampleRateLabel.setBounds(area.removeFromTop(20).withWidth(200));
}

void AudioPage::lookAndFeelChanged()
{
    latencyToggle.setColour(juce::ToggleButton::textColourId, T_COL(textMain));
    sampleRateLabel.setColour(juce::Label::textColourId, T_COL(textDimmed));
    repaint();
}

void AudioPage::buttonClicked(juce::Button* b) {}
void AudioPage::sliderValueChanged(juce::Slider* s) {}
