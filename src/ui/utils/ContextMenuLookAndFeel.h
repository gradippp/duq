#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "FontManager.h"

class ContextMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ContextMenuLookAndFeel()
    {
        refreshColours();
    }

    void refreshColours()
    {
        setColour(juce::PopupMenu::backgroundColourId, T_COL(contextMenuBackground));
        setColour(juce::PopupMenu::textColourId, T_COL(contextMenuText));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, T_COL(contextMenuHighlight));
        setColour(juce::PopupMenu::highlightedTextColourId, T_COL(textMain));
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto area = juce::Rectangle<int>(width, height).toFloat();
        g.setColour(T_COL(contextMenuBackground));
        g.fillRoundedRectangle(area, 4.0f);

        g.setColour(T_COL(contextMenuBorder));
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
            g.setColour(T_COL(border).withAlpha(0.5f));
            g.drawLine((float)r.getX(), (float)r.getCentreY(), (float)r.getRight(), (float)r.getCentreY());
            return;
        }

        auto itemArea = area.toFloat();

        if (isHighlighted && isActive)
        {
            g.setColour(T_COL(contextMenuHighlight));
            g.fillRoundedRectangle(itemArea.reduced(2.0f, 1.0f), 3.0f);
        }

        g.setColour(textColourToUse != nullptr ? *textColourToUse : T_COL(contextMenuText));
        g.setFont(FontManager::getInterRegular(13.0f));

        // Text area leaves space for potential icon on left and tick on right
        auto textRect = itemArea.reduced(10, 0);
        g.drawFittedText(text, textRect.toNearestInt(), juce::Justification::centredLeft, 1);

        if (isTicked)
        {
            // Move tick mark to the RIGHT side
            const float tickSize = 6.0f;
            auto tickArea = itemArea.removeFromRight(20).withSizeKeepingCentre(tickSize, tickSize);
            g.setColour(T_COL(accent));
            g.fillEllipse(tickArea);
        }
    }
};
