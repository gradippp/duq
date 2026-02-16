#include "ControlSliderComponent.h"

ControlSliderComponent::ControlSliderComponent(const juce::String& label,
    const juce::String& unitSuffix)
    : labelText(label), unit(unitSuffix)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(0.0, 100.0, 0.01);
    slider.setValue(0.0);

    slider.onValueChange = [this]()
        {
            updateValueLabel();
        };

    addAndMakeVisible(slider);
    addAndMakeVisible(valueLabel);

    valueLabel.setJustificationType(juce::Justification::centredRight);
    valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    updateValueLabel();
}

void ControlSliderComponent::updateValueLabel()
{
    auto value = slider.getValue();

    juce::String text;

    if (unit == "%")
        text = juce::String((int)value) + "%";
    else if (unit == "Hz")
        text = juce::String((int)value) + "Hz";
    else
        text = juce::String(value, 2) + unit;

    valueLabel.setText(text, juce::dontSendNotification);
}

void ControlSliderComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));

    constexpr int labelWidth = 90;
    constexpr int leftPadding = 12;

    g.drawText(labelText,
        leftPadding, 0,
        labelWidth - leftPadding,
        getHeight(),
        juce::Justification::centredLeft);
}

void ControlSliderComponent::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromLeft(80); // label space

    valueLabel.setBounds(bounds.removeFromRight(60));

    slider.setBounds(bounds.reduced(6, 8));
}
