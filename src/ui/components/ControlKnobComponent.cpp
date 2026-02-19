#include "ControlKnobComponent.h"
#include "../utils/FontManager.h"
#include "../../Globals.h"

ControlKnobComponent::ControlKnobComponent(const juce::String& name, const float initialValue,
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
    knob.setValue(initialValue);

    knob.onValueChange = [this]()
        {
            updateValueLabel();

            if (onValueChanged)
                onValueChanged(knob.getValue());
        };


    knob.rightClickHandler = [this](const juce::MouseEvent&)
        {
            showContextMenu();
        };

    addAndMakeVisible(knob);
    addAndMakeVisible(valueLabel);

    valueLabel.setColour(juce::Label::textColourId, Theme::Colours::textMain);
    valueLabel.setJustificationType(juce::Justification::centredLeft);
    valueLabel.setFont(FontManager::getJetBrainsMono(13.0f));

    updateValueLabel();
}



void ControlKnobComponent::setLabel(const juce::String& text)
{
    labelText = text;
    repaint();
}

void ControlKnobComponent::refreshValueLabel() {
    updateValueLabel();
}

void ControlKnobComponent::updateValueLabel()
{
    auto value = knob.getValue();

    juce::String text;

    if (valueFormatter)
    {
        text = valueFormatter(value);
    }
    else
    {
        text = juce::String(value, 2) + unit;
    }

    valueLabel.setText("[" + text + "]",
        juce::dontSendNotification);
}

void ControlKnobComponent::showContextMenu()
{
    juce::PopupMenu menu;

    menu.addItem(1, "Enter value");

    if (extendContextMenu)
    {
        menu.addSeparator();
        extendContextMenu(menu);
    }

    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(this),
        [this](int result)
        {
            if (result == 1)
                showValueEntryDialog();
            else
                handleCustomMenuResult(result);
        });
}

void ControlKnobComponent::showValueEntryDialog()
{
    auto* window = new juce::AlertWindow(
        "Enter Value",
        "Type a new value:",
        juce::AlertWindow::NoIcon);

    window->addTextEditor("value",
        juce::String(knob.getValue()));

    window->addButton("OK", 1);
    window->addButton("Cancel", 0);

    window->enterModalState(
        true,
        juce::ModalCallbackFunction::create(
            [this, window](int result)
            {
                if (result == 1)
                {
                    auto text =
                        window->getTextEditor("value")->getText();

                    auto newValue = text.getDoubleValue();

                    knob.setValue(newValue,
                        juce::sendNotification);
                }

                delete window;
            }),
        true);
}

void ControlKnobComponent::handleCustomMenuResult(int result)
{
    if (onCustomMenuResult)
        onCustomMenuResult(result);
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

    g.setColour(Theme::Colours::textLabel);
    g.setFont(FontManager::getInterMedium(14.0f));

    g.drawText(labelText,
        bounds,
        juce::Justification::centredLeft);
}

