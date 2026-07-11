#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../../Globals.h"
#include "../../utils/FontManager.h"
#include "../../../utils/ConfigManager.h"

/**
    Base class for all settings pages to provide uniformity and reduce code repetition.
*/
class SettingsPageBase : public juce::Component
{
public:
    SettingsPageBase(const juce::String& categoryTitle)
        : title(categoryTitle)
    {}

    void paint(juce::Graphics& g) override
    {
        // Category Header
        g.setColour(Theme::get(ThemeManager::textMain));
        g.setFont(FontManager::getBarlowBold(14.0f));
        g.drawText(title, 20, 20, getWidth() - 40, 30, juce::Justification::centredLeft);

        // Underline Divider
        g.setColour(Theme::get(ThemeManager::border).withAlpha(0.3f));
        g.drawLine(20.0f, 45.0f, static_cast<float>(getWidth()) - 20.0f, 45.0f, 1.0f);
    }

    /** Helper to get a standard content area for settings controls */
    juce::Rectangle<int> getContentArea() const
    {
        return getLocalBounds().reduced(20, 80);
    }

    /** Helper to draw a label above a control consistently */
    void drawControlLabel(juce::Graphics& g, const juce::String& label, const juce::Rectangle<int>& controlBounds)
    {
        g.setFont(FontManager::getInterRegular(12.0f));
        g.setColour(Theme::get(ThemeManager::textLabel));
        g.drawText(label, controlBounds.getX(), controlBounds.getY() - 20, controlBounds.getWidth(), 20, juce::Justification::centredLeft);
    }

    /** Calculates the required height based on the bottom-most child component. */
    virtual int getRequiredHeight()
    {
        // Default implementation based on child positions
        int maxBottom = 0;
        for (auto* child : getChildren())
        {
            if (child->isVisible())
                maxBottom = juce::jmax(maxBottom, child->getBottom());
        }
        
        // Ensure a minimum height so it doesn't look squashed
        return juce::jmax(400, maxBottom + 40); 
    }

protected:
    juce::String title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageBase)
};
