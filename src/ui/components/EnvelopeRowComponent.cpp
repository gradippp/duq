#include "EnvelopeRowComponent.h"
#include "../utils/IconFactory.h"


EnvelopeRowComponent::EnvelopeRowComponent(const juce::String& name)
    : envelopeName(name)
{
    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                juce::Colours::darkgrey.withAlpha(0.2f));

            auto normal = Icons::load(iconName, juce::Colours::white);
            auto over = Icons::load(iconName, juce::Colours::white.withAlpha(0.85f));
            auto down = Icons::load(iconName, juce::Colours::white.withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);

            button.setTooltip(iconName);
        };

    setupIconButton(saveButton, "save");
    setupIconButton(replaceButton, "replace");
    setupIconButton(deleteButton, "delete");

    saveButton.setTooltip("Save this envelope as a preset, including the controls.");
    replaceButton.setTooltip("Replace this envelope from a preset");
    deleteButton.setTooltip("Delete this envelope");

    deleteButton.onClick = [this]()
        {
            if (onDeleteRequested)
                onDeleteRequested();
        };


    addAndMakeVisible(saveButton);
    addAndMakeVisible(replaceButton);
    addAndMakeVisible(deleteButton);
}

void EnvelopeRowComponent::setActive(bool shouldBeActive)
{
    isActive = shouldBeActive;
    repaint();
}

void EnvelopeRowComponent::mouseEnter(const juce::MouseEvent&)
{
    isHovered = true;
    repaint();
}

void EnvelopeRowComponent::mouseExit(const juce::MouseEvent&)
{
    isHovered = false;
    repaint();
}

void EnvelopeRowComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background strip
    auto bgColour = juce::Colours::darkgrey.withAlpha(0.2f);

    if (isHovered)
        bgColour = juce::Colours::darkgrey.withAlpha(0.35f);

    g.setColour(bgColour);
    g.fillRect(bounds);

    // Envelope name
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));

    g.drawText(envelopeName,
        bounds.removeFromLeft(80).reduced(10, 0),
        juce::Justification::centredLeft);

    // Active indicator dot (right side)
    if (isActive)
    {
        const int dotSize = 6;
        auto dotArea = getLocalBounds()
            .removeFromRight(20)
            .withSizeKeepingCentre(dotSize, dotSize);

        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(dotArea.toFloat());
    }

    // Bottom separator line
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    g.drawLine(0.0f, (float)getHeight() - 1.0f,
        (float)getWidth(), (float)getHeight() - 1.0f);
}

void EnvelopeRowComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    // Reserve right side for buttons
    auto buttonArea = bounds.removeFromRight(90);

    constexpr int buttonWidth = 24;

    saveButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    replaceButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
}
