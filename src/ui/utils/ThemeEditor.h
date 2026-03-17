#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "ThemeManager.h"
#include "../../Globals.h"
#include "FontManager.h"

class ColorRow : public juce::Component
{
public:
    ColorRow(ThemeManager::ColourID id) : colourID(id)
    {
        colorButton.id = id;
        nameLabel.setText(ThemeManager::getInstance().getColourName(id).toUpperCase(), juce::dontSendNotification);
        nameLabel.setFont(FontManager::getJetBrainsMono(11.0f));
        nameLabel.setColour(juce::Label::textColourId, T_COL(textLabel));
        addAndMakeVisible(nameLabel);

        colorButton.setButtonText("");
        addAndMakeVisible(colorButton);
        colorButton.onClick = [this] { openPicker(); };
    }

    void paint(juce::Graphics& g) override
    {
        // Divider
        g.setColour(T_COL(border).withAlpha(0.1f));
        g.drawLine(0.0f, (float)getHeight() - 1.0f, (float)getWidth(), (float)getHeight() - 1.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(5, 0);
        nameLabel.setBounds(area.removeFromLeft(area.getWidth() - 60));
        colorButton.setBounds(area.removeFromRight(40).reduced(0, 5));
    }

private:
    class PickerWrapper : public juce::Component, private juce::ChangeListener
    {
    public:
        PickerWrapper(ThemeManager::ColourID id, ColorRow* r) 
            : colourID(id), safeRow(r)
        {
            selector.setName("Edit Color");
            selector.setCurrentColour(ThemeManager::getInstance().getColour(id));
            selector.addChangeListener(this);
            addAndMakeVisible(selector);
            setSize(300, 450);
        }

        ~PickerWrapper() override
        {
            selector.removeChangeListener(this);
        }

        void resized() override
        {
            selector.setBounds(getLocalBounds());
        }

    private:
        void changeListenerCallback(juce::ChangeBroadcaster* source) override
        {
            if (safeRow != nullptr)
            {
                ThemeManager::getInstance().setColour(colourID, selector.getCurrentColour());
                safeRow->repaint();
            }
        }

        ThemeManager::ColourID colourID;
        juce::Component::SafePointer<ColorRow> safeRow;
        juce::ColourSelector selector{ juce::ColourSelector::showAlphaChannel | juce::ColourSelector::showColourspace };
    };

    void openPicker()
    {
        auto wrapper = std::make_unique<PickerWrapper>(colourID, this);
        juce::CallOutBox::launchAsynchronously(std::move(wrapper), colorButton.getScreenBounds(), nullptr);
    }

    ThemeManager::ColourID colourID;
    juce::Label nameLabel;
    
    struct ColorButton : public juce::TextButton {
        ThemeManager::ColourID id;
        void paintButton(juce::Graphics& g, bool, bool) override {
            g.setColour(ThemeManager::getInstance().getColour(id));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 2.0f);
            g.setColour(T_COL(border));
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
        }
    } colorButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorRow)
};

class ThemeEditor : public juce::Component
{
public:
    ThemeEditor()
    {
        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&content);

        auto ids = ThemeManager::getAllIDs();
        for (auto id : ids)
        {
            auto* row = new ColorRow(id);
            rows.add(row);
            content.addAndMakeVisible(row);
        }
        
        updateLayout();
    }

    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        updateLayout();
    }

private:
    void updateLayout()
    {
        int rowHeight = 35;
        int y = 0;
        for (auto* row : rows)
        {
            row->setBounds(0, y, viewport.getWidth() - viewport.getScrollBarThickness(), rowHeight);
            y += rowHeight;
        }
        content.setSize(viewport.getWidth() - viewport.getScrollBarThickness(), y);
    }

    juce::Viewport viewport;
    juce::Component content;
    juce::OwnedArray<ColorRow> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeEditor)
};
