#include "EnvelopeRowComponent.h"

EnvelopeRowComponent::EnvelopeRowComponent(const juce::String& name)
    : envelopeName(name)
{
    for (auto* b : { &duplicateButton, &editButton, &deleteButton })
    {
        b->setColour(juce::TextButton::buttonColourId,
            juce::Colours::darkgrey.withAlpha(0.4f));

        b->setColour(juce::TextButton::textColourOffId,
            juce::Colours::white);

        b->setClickingTogglesState(false);
        addAndMakeVisible(*b);
    }
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

    duplicateButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    editButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
}
