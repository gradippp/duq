#pragma once

#include "ContextMenuLookAndFeel.h"

class ComboBoxLookAndFeel : public ContextMenuLookAndFeel
{
public:
    ComboBoxLookAndFeel()
    {
        refreshColours();
    }

    void refreshColours()
    {
        ContextMenuLookAndFeel::refreshColours();
        setColour(juce::ComboBox::backgroundColourId, T_COL(sectionBackground));
        setColour(juce::ComboBox::outlineColourId, T_COL(border).withAlpha(0.5f));
        setColour(juce::ComboBox::textColourId, T_COL(textMain));
        setColour(juce::ComboBox::arrowColourId, T_COL(accent).withAlpha(0.8f));
        setColour(juce::ComboBox::focusedOutlineColourId, T_COL(accent).withAlpha(0.5f));
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(width, height).toFloat();

        // Background
        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(bounds, 2.0f);

        // Outline
        g.setColour(box.findColour(box.hasKeyboardFocus(true) ? juce::ComboBox::focusedOutlineColourId 
                                                             : juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 2.0f, 1.0f);

        // Arrow
        auto arrowArea = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH).toFloat();
        float arrowSize = 0.3f * juce::jmin(arrowArea.getWidth(), arrowArea.getHeight());
        auto centre = arrowArea.getCentre();

        juce::Path p;
        p.addTriangle(centre.x - arrowSize, centre.y - arrowSize * 0.5f,
                      centre.x + arrowSize, centre.y - arrowSize * 0.5f,
                      centre.x, centre.y + arrowSize * 0.5f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId));
        g.fillPath(p);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(1, 1, box.getWidth() - 30, box.getHeight() - 2);
        label.setFont(getComboBoxFont(box));
        label.setJustificationType(juce::Justification::centredLeft);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return FontManager::getInterRegular(13.0f);
    }
};
