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
    sampleRateLabel.setColour(juce::Label::textColourId, Theme::Colours::textDimmed);
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
    auto area = getContentArea();
    latencyToggle.setBounds(area.removeFromTop(30));
    area.removeFromTop(40);
    globalMixSlider.setBounds(area.removeFromTop(30).withWidth(200));
    area.removeFromTop(45);
    sampleRateLabel.setBounds(area.removeFromTop(20).withWidth(200));
}

void AudioPage::buttonClicked(juce::Button* b) {}
void AudioPage::sliderValueChanged(juce::Slider* s) {}
