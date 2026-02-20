#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "FontManager.h"

class ContextMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ContextMenuLookAndFeel()
    {
        setColour(juce::PopupMenu::backgroundColourId, Theme::Colours::contextMenuBackground);
        setColour(juce::PopupMenu::textColourId, Theme::Colours::contextMenuText);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::Colours::contextMenuHighlight);
        setColour(juce::PopupMenu::highlightedTextColourId, Theme::Colours::textMain);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto area = juce::Rectangle<int>(width, height).toFloat();
        g.setColour(Theme::Colours::contextMenuBackground);
        g.fillRoundedRectangle(area, 4.0f);

        g.setColour(Theme::Colours::contextMenuBorder);
        g.drawRoundedRectangle(area.reduced(0.5f), 4.0f, 1.0f);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColourToUse) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            g.setColour(Theme::Colours::border.withAlpha(0.5f));
            g.drawLine((float)r.getX(), (float)r.getCentreY(), (float)r.getRight(), (float)r.getCentreY());
            return;
        }

        auto itemArea = area.toFloat();

        if (isHighlighted && isActive)
        {
            g.setColour(Theme::Colours::contextMenuHighlight);
            g.fillRoundedRectangle(itemArea.reduced(2.0f, 1.0f), 3.0f);
        }

        g.setColour(textColourToUse != nullptr ? *textColourToUse : Theme::Colours::contextMenuText);
        g.setFont(FontManager::getInterRegular(13.0f));

        // Text area leaves space for potential icon on left and tick on right
        auto textRect = itemArea.reduced(10, 0);
        g.drawFittedText(text, textRect.toNearestInt(), juce::Justification::centredLeft, 1);

        if (isTicked)
        {
            // Move tick mark to the RIGHT side
            const float tickSize = 6.0f;
            auto tickArea = itemArea.removeFromRight(20).withSizeKeepingCentre(tickSize, tickSize);
            g.setColour(Theme::Colours::accent);
            g.fillEllipse(tickArea);
        }
    }
};
