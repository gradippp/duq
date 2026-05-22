#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Globals.h"
#include "FontManager.h"
#include "IconFactory.h"

/**
    Central LookAndFeel for the entire application.
    Uses T_COL() directly to bypass JUCE's color ID system and ensure
    perfect theme synchronization.
*/
class GlobalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlobalLookAndFeel()
    {
        refreshColours();
    }

    void refreshColours()
    {
        // Set standard JUCE colours to match theme for components not overridden
        setColour(juce::TextButton::buttonColourId, T_COL(sectionBackground));
        setColour(juce::TextButton::buttonOnColourId, T_COL(uiSelected));
        setColour(juce::TextButton::textColourOffId, T_COL(textMain));
        setColour(juce::TextButton::textColourOnId, T_COL(accent));

        setColour(juce::ComboBox::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::ComboBox::outlineColourId, T_COL(widgetOutline));
        setColour(juce::ComboBox::textColourId, T_COL(widgetText));

        setColour(juce::Label::textColourId, T_COL(textMain));
        
        setColour(juce::Slider::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::Slider::thumbColourId, T_COL(accent));
        setColour(juce::Slider::trackColourId, T_COL(accent).withAlpha(0.5f));
        setColour(juce::Slider::textBoxTextColourId, T_COL(widgetText));
        setColour(juce::Slider::textBoxBackgroundColourId, T_COL(widgetBackground));
        setColour(juce::Slider::textBoxOutlineColourId, T_COL(widgetOutline).withAlpha(0.3f));

        setColour(juce::TextEditor::backgroundColourId, T_COL(widgetBackground));
        setColour(juce::TextEditor::textColourId, T_COL(widgetText));
        setColour(juce::TextEditor::outlineColourId, T_COL(widgetOutline));
    }

    // ==============================================================================
    // BUTTONS
    // ==============================================================================

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, [[maybe_unused]] const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 2.0f;

        auto baseColour = T_COL(sectionBackground);
        if (isButtonDown)
            baseColour = T_COL(uiSelected).withAlpha(0.4f);
        else if (isMouseOverButton)
            baseColour = T_COL(uiHover);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(T_COL(border).withAlpha(isMouseOverButton ? 0.8f : 0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isMouseOverButton, isButtonDown);
        g.setFont(FontManager::getBarlowBold(13.0f));
        
        auto textCol = button.getToggleState() ? T_COL(accent) : T_COL(textMain);
        g.setColour(textCol.withAlpha(button.isEnabled() ? 1.0f : 0.5f));

        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(4, 0), juce::Justification::centred, 2);
    }

    // ==============================================================================
    // TOGGLES (Checkboxes)
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
        g.setColour(T_COL(widgetOutline).withAlpha(isMouseOverButton ? 1.0f : 0.6f));
        g.drawRoundedRectangle(boxRect.reduced(0.5f), 2.0f, 1.2f);

        if (ticked)
        {
            g.setColour(T_COL(widgetTick));
            
            // Draw a thick, high-contrast checkmark
            juce::Path p;
            p.startNewSubPath(x + w * 0.2f, y + h * 0.5f);
            p.lineTo(x + w * 0.45f, y + h * 0.8f);
            p.lineTo(x + w * 0.85f, y + h * 0.2f);
            
            g.strokePath(p, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // ==============================================================================
    // COMBO BOXES (Dropdowns)
    // ==============================================================================

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(isButtonDown);
        auto bounds = juce::Rectangle<int>(width, height).toFloat();

        // Background
        g.setColour(T_COL(widgetBackground));
        g.fillRoundedRectangle(bounds, 2.0f);

        // Outline
        g.setColour(T_COL(widgetOutline).withAlpha(box.hasKeyboardFocus(true) ? 1.0f : 0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 2.0f, 1.2f);

        // Arrow Area
        auto arrowArea = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH).toFloat();
        float arrowSize = 0.20f * juce::jmin(arrowArea.getWidth(), arrowArea.getHeight());
        auto centre = arrowArea.getCentre();

        juce::Path p;
        p.addTriangle(centre.x - arrowSize, centre.y - arrowSize * 0.5f,
                      centre.x + arrowSize, centre.y - arrowSize * 0.5f,
                      centre.x, centre.y + arrowSize * 0.5f);

        g.setColour(T_COL(widgetText).withAlpha(0.7f));
        g.fillPath(p);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(4, 1, box.getWidth() - 34, box.getHeight() - 2);
        label.setFont(getComboBoxFont(box));
        label.setJustificationType(juce::Justification::centredLeft);
        
        // Force the label color from the theme
        label.setColour(juce::Label::textColourId, T_COL(widgetText));
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
            auto trackWidth = 5.0f;
            auto isHorizontal = style == juce::Slider::LinearHorizontal;

            juce::Rectangle<float> trackRect;
            if (isHorizontal)
                trackRect = { (float)x, y + height * 0.5f - trackWidth * 0.5f, (float)width, trackWidth };
            else
                trackRect = { x + width * 0.5f - trackWidth * 0.5f, (float)y, trackWidth, (float)height };

            // Track Background
            g.setColour(T_COL(widgetBackground).darker(0.05f));
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
            float thumbSize = 14.0f;
            juce::Rectangle<float> thumbRect;
            if (isHorizontal)
                thumbRect = { sliderPos - thumbSize * 0.5f, y + height * 0.5f - thumbSize * 0.5f, thumbSize, thumbSize };
            else
                thumbRect = { x + width * 0.5f - thumbSize * 0.5f, sliderPos - thumbSize * 0.5f, thumbSize, thumbSize };

            g.setColour(T_COL(textMain));
            g.fillEllipse(thumbRect);
            
            g.setColour(T_COL(widgetOutline));
            g.drawEllipse(thumbRect, 1.5f);
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          [[maybe_unused]] juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // --- Outer Ring (Shadow/Glow) ---
        g.setColour(T_COL(knobShadow).withAlpha(0.2f));
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        // --- Base Circle ---
        g.setColour(T_COL(sectionBackground));
        g.fillEllipse(centreX - radius + 1.0f, centreY - radius + 1.0f, (radius - 1.0f) * 2.0f, (radius - 1.0f) * 2.0f);

        // --- Arcs Area ---
        float arcRadius = radius - 4.0f;
        float thickness = 3.5f;

        // Background arc (Track)
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(T_COL(knobTrack));
        g.strokePath(bgArc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc (Active)
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(T_COL(knobAccent));
        g.strokePath(valueArc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // --- Indicator (Dot) ---
        float dotRadius = 2.0f;
        float dotDist = radius - 9.0f;
        float cosA = std::cos(angle - juce::MathConstants<float>::halfPi);
        float sinA = std::sin(angle - juce::MathConstants<float>::halfPi);
        juce::Point<float> dotPos (centreX + dotDist * cosA, centreY + dotDist * sinA);

        g.setColour(T_COL(knobIndicator));
        g.fillEllipse(dotPos.x - dotRadius, dotPos.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }

    juce::Label* createSliderTextBox(juce::Slider& slider) override
    {
        auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
        l->setColour(juce::Label::textColourId, T_COL(widgetText));
        l->setColour(juce::Label::backgroundColourId, T_COL(widgetBackground));
        l->setColour(juce::Label::outlineColourId, T_COL(widgetOutline).withAlpha(0.3f));
        return l;
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
        juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, icon);
        
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
