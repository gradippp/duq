#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "FontManager.h"
#include "IconFactory.h"

class GlobalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlobalLookAndFeel()
    {
        refreshColours();
    }

    void refreshColours()
    {
        // ==============================================================================
        // CORE UI COLOURS
        // ==============================================================================
        
        // --- Buttons ---
        setColour(juce::TextButton::buttonColourId, T_COL(sectionBackground));
        setColour(juce::TextButton::buttonOnColourId, T_COL(uiSelected));
        setColour(juce::TextButton::textColourOffId, T_COL(textMain));
        setColour(juce::TextButton::textColourOnId, T_COL(accent));

        // --- Combo Boxes ---
        setColour(juce::ComboBox::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::ComboBox::outlineColourId, T_COL(widgetOutline));
        setColour(juce::ComboBox::textColourId, T_COL(widgetText));
        setColour(juce::ComboBox::arrowColourId, T_COL(accent).withAlpha(0.8f));
        setColour(juce::ComboBox::focusedOutlineColourId, T_COL(accent).withAlpha(0.5f));

        // --- Popup Menus ---
        setColour(juce::PopupMenu::backgroundColourId, T_COL(contextMenuBackground));
        setColour(juce::PopupMenu::textColourId, T_COL(contextMenuText));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, T_COL(contextMenuHighlight));
        setColour(juce::PopupMenu::highlightedTextColourId, T_COL(textMain));

        // --- Toggles ---
        setColour(juce::ToggleButton::textColourId, T_COL(textMain));
        setColour(juce::ToggleButton::tickColourId, T_COL(widgetTick));

        // --- Sliders ---
        setColour(juce::Slider::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::Slider::thumbColourId, T_COL(accent));
        setColour(juce::Slider::trackColourId, T_COL(accent).withAlpha(0.4f));
        setColour(juce::Slider::textBoxBackgroundColourId, T_COL(widgetBackground));
        setColour(juce::Slider::textBoxTextColourId, T_COL(textMain));
        setColour(juce::Slider::textBoxOutlineColourId, T_COL(widgetOutline));

        // --- Text Editors ---
        setColour(juce::TextEditor::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::TextEditor::textColourId, T_COL(textMain));
        setColour(juce::TextEditor::outlineColourId, T_COL(widgetOutline));
        setColour(juce::TextEditor::focusedOutlineColourId, T_COL(accent).withAlpha(0.6f));
        setColour(juce::TextEditor::highlightColourId, T_COL(accent).withAlpha(0.3f));
    }

    // ==============================================================================
    // BUTTONS
    // ==============================================================================

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

        g.setColour(T_COL(border).withAlpha(isMouseOverButton ? 0.6f : 0.3f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
    {
        g.setFont(FontManager::getBarlowBold(12.0f));
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId 
                                                             : juce::TextButton::textColourOffId)
                          .withAlpha(button.isEnabled() ? 1.0f : 0.5f));

        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(4, 0), juce::Justification::centred, 2);
    }

    // ==============================================================================
    // TOGGLES
    // ==============================================================================

    void drawTickBox(juce::Graphics& g, juce::Component& component,
                     float x, float y, float w, float h,
                     const bool ticked, const bool isEnabled,
                     const bool isMouseOverButton, const bool isButtonDown) override
    {
        juce::ignoreUnused(isEnabled, isButtonDown, component);
        auto boxRect = juce::Rectangle<float>(x, y, w, h).reduced(1.0f);

        // Background
        g.setColour(T_COL(widgetBackground));
        g.fillRoundedRectangle(boxRect, 2.0f);

        // Outline
        g.setColour(T_COL(widgetOutline).withAlpha(isMouseOverButton ? 0.8f : 0.4f));
        g.drawRoundedRectangle(boxRect.reduced(0.5f), 2.0f, 1.0f);

        if (ticked)
        {
            g.setColour(T_COL(widgetTick));
            auto tickPath = getTickShape(w * 0.7f);
            g.fillPath(tickPath, tickPath.getTransformToScaleToFit(x + w * 0.15f, y + h * 0.15f, w * 0.7f, h * 0.7f, true));
        }
    }

    // ==============================================================================
    // COMBO BOXES
    // ==============================================================================

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
        label.setColour(juce::Label::textColourId, box.findColour(juce::ComboBox::textColourId));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return FontManager::getInterRegular(13.0f);
    }

    // ==============================================================================
    // SLIDERS
    // ==============================================================================

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearVertical)
        {
            auto trackWidth = 4.0f;
            auto isHorizontal = style == juce::Slider::LinearHorizontal;

            juce::Rectangle<float> trackRect;
            if (isHorizontal)
                trackRect = { (float)x, y + height * 0.5f - trackWidth * 0.5f, (float)width, trackWidth };
            else
                trackRect = { x + width * 0.5f - trackWidth * 0.5f, (float)y, trackWidth, (float)height };

            // Track Background
            g.setColour(T_COL(widgetBackground));
            g.fillRoundedRectangle(trackRect, trackWidth * 0.5f);

            // Active Track
            juce::Rectangle<float> activeTrack;
            if (isHorizontal)
                activeTrack = { trackRect.getX(), trackRect.getY(), sliderPos - trackRect.getX(), trackWidth };
            else
                activeTrack = { trackRect.getX(), sliderPos, trackWidth, trackRect.getBottom() - sliderPos };

            g.setColour(T_COL(accent));
            g.fillRoundedRectangle(activeTrack, trackWidth * 0.5f);

            // Thumb
            float thumbSize = 12.0f;
            juce::Rectangle<float> thumbRect;
            if (isHorizontal)
                thumbRect = { sliderPos - thumbSize * 0.5f, y + height * 0.5f - thumbSize * 0.5f, thumbSize, thumbSize };
            else
                thumbRect = { x + width * 0.5f - thumbSize * 0.5f, sliderPos - thumbSize * 0.5f, thumbSize, thumbSize };

            g.setColour(T_COL(textMain));
            g.fillEllipse(thumbRect);
            
            g.setColour(T_COL(widgetOutline));
            g.drawEllipse(thumbRect, 1.0f);
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // --- Outer Ring (Shadow/Glow) ---
        g.setColour(T_COL(knobShadow).withAlpha(0.2f));
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        // --- Base Circle Gradient ---
        juce::ColourGradient baseGrad(T_COL(sectionBackground).brighter(0.05f), centreX, centreY - radius,
                                      T_COL(sectionBackground).darker(0.1f), centreX, centreY + radius, false);
        g.setGradientFill(baseGrad);
        g.fillEllipse(centreX - radius + 1.0f, centreY - radius + 1.0f, (radius - 1.0f) * 2.0f, (radius - 1.0f) * 2.0f);

        // Inner bevel highlight
        g.setColour(T_COL(accent).withAlpha(0.05f));
        g.drawEllipse(centreX - radius + 2.0f, centreY - radius + 2.0f, (radius - 2.0f) * 2.0f, (radius - 2.0f) * 2.0f, 1.0f);

        // --- Arcs Area ---
        float arcRadius = radius - 3.5f;
        float thickness = 3.0f;

        // Background arc (Track)
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(T_COL(knobTrack));
        g.strokePath(bgArc, juce::PathStrokeType(thickness + 0.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc (Active)
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
        
        auto accentColour = T_COL(knobAccent);
        g.setColour(accentColour);
        g.strokePath(valueArc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Subtle Glow on value arc
        g.setColour(accentColour.withAlpha(0.15f));
        g.strokePath(valueArc, juce::PathStrokeType(thickness + 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // --- Modern Indicator (Dot) ---
        float dotRadius = 1.8f;
        float dotDist = radius - 8.5f;
        
        float cosA = std::cos(angle - juce::MathConstants<float>::halfPi);
        float sinA = std::sin(angle - juce::MathConstants<float>::halfPi);

        juce::Point<float> dotPos (centreX + dotDist * cosA, centreY + dotDist * sinA);

        g.setColour(T_COL(knobIndicator));
        g.fillEllipse(dotPos.x - dotRadius, dotPos.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }

    // ==============================================================================
    // POPUP MENUS
    // ==============================================================================

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

        auto textRect = itemArea.reduced(10, 0);
        g.drawFittedText(text, textRect.toNearestInt(), juce::Justification::centredLeft, 1);

        if (isTicked)
        {
            const float tickSize = 6.0f;
            auto tickArea = itemArea.removeFromRight(20).withSizeKeepingCentre(tickSize, tickSize);
            g.setColour(T_COL(accent));
            g.fillEllipse(tickArea);
        }
    }
};
