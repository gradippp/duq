#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "FontManager.h"

class TextButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TextButtonLookAndFeel()
    {
        setColour(juce::TextButton::buttonColourId, Theme::Colours::sectionBackground);
        setColour(juce::TextButton::buttonOnColourId, Theme::Colours::uiSelected);
        setColour(juce::TextButton::textColourOffId, Theme::Colours::textMain);
        setColour(juce::TextButton::textColourOnId, Theme::Colours::accent);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 2.0f;

        auto baseColour = backgroundColour;
        if (isButtonDown)
            baseColour = baseColour.darker(0.1f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.05f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(Theme::Colours::border.withAlpha(isMouseOverButton ? 0.6f : 0.3f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
    {
        auto font = FontManager::getBarlowBold(12.0f);
        g.setFont(font);
        
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId 
                                                             : juce::TextButton::textColourOffId)
                          .withAlpha(button.isEnabled() ? 1.0f : 0.5f));

        auto yIndent = juce::jmin(4.0f, button.getHeight() * 0.1f);
        auto cornerSize = juce::jmin(button.getWidth(), button.getHeight()) * 0.01f;

        g.drawFittedText(button.getButtonText(),
                         button.getLocalBounds().reduced(4, 0),
                         juce::Justification::centred, 2);
    }
};
