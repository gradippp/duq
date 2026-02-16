#include "ControlKnobComponent.h"

ControlKnobComponent::ControlKnobComponent(const juce::String& name,
    const juce::String& unitSuffix)
    : labelText(name), unit(unitSuffix)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    knob.setRotaryParameters(
        juce::degreesToRadians(135.0f),
        juce::degreesToRadians(405.0f),
        true
    );

    knob.setRange(0.0, 100.0, 0.01);
    knob.setValue(50.0);

    knob.onValueChange = [this]()
        {
            updateValueLabel();
        };

    addAndMakeVisible(knob);
    addAndMakeVisible(valueLabel);

    valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    valueLabel.setJustificationType(juce::Justification::centredLeft);

    updateValueLabel();
}

void ControlKnobComponent::updateValueLabel()
{
    auto value = knob.getValue();

    juce::String text;

    if (unit == "%")
        text = juce::String((int)value) + "%";
    else if (unit == "Hz")
        text = juce::String((int)value) + "Hz";
    else
        text = juce::String(value, 2) + unit;

    valueLabel.setText("[" + text + "]",
        juce::dontSendNotification);
}

void ControlKnobComponent::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    constexpr int knobSize = 44;
    constexpr int spacing = 10;
    constexpr int valueWidth = 60;

    auto knobArea = bounds.removeFromLeft(knobSize);
    knob.setBounds(knobArea);

    bounds.removeFromLeft(spacing);

    valueLabel.setBounds(bounds.removeFromRight(valueWidth));
}

void ControlKnobComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(6);

    constexpr int knobSize = 46;
    constexpr int spacing = 10;

    bounds.removeFromLeft(knobSize + spacing);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f));

    g.drawText(labelText,
        bounds,
        juce::Justification::centredLeft);
}

