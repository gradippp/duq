#include "PresetSection.h"
#include "../utils/IconFactory.h"

PresetSection::PresetSection()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font("Segoe UI", 20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));

    auto normal = Icons::load("close", juce::Colours::white.withAlpha(0.6f));
    auto over = Icons::load("close", juce::Colours::white);
    auto down = Icons::load("close", juce::Colours::white.withAlpha(0.4f));
    closeButton.setImages(normal.get(), over.get(), down.get());

    addAndMakeVisible(closeButton);
    closeButton.setTooltip("Close Preset Selection");
    
    closeButton.onClick = [this]() {
        if (onClose)
            onClose();
    };

    setOpaque(true);
}

PresetSection::~PresetSection()
{
}

void PresetSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Modern dark background
    g.setColour(juce::Colour(0xff121212));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle inner shadow / border
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);

    // Placeholder text for preset selection
    g.setColour(juce::Colours::grey.withAlpha(0.4f));
    g.setFont(juce::Font("Segoe UI", 16.0f, juce::Font::plain));
    g.drawFittedText("PRESET BROWSER COMING SOON", 
                      getLocalBounds(), 
                      juce::Justification::centred, 
                      1);
}

void PresetSection::resized()
{
    auto area = getLocalBounds();
    
    auto headerArea = area.removeFromTop(60);
    
    // Close button (X) in the top right
    const int xSize = 24;
    closeButton.setBounds(headerArea.removeFromRight(50).withSizeKeepingCentre(xSize, xSize));
    
    // Title is centered in the header area
    titleLabel.setBounds(headerArea.withLeft(50)); // compensate for the button space on the right
}

void PresetSection::setTargetEnvelope(juce::ValueTree envelope)
{
    targetEnvelope = envelope;
}
