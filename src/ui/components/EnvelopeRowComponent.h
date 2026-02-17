#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../model/EnvelopeData.h"

class SelectableLabel : public juce::Label
{
public:
    std::function<void()> onSingleClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (onSingleClick)
            onSingleClick();

        juce::Label::mouseDown(e);
    }
};

class EnvelopeRowComponent : public juce::Component
{
public:
    EnvelopeRowComponent(EnvelopeData& dataRef);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    std::function<void()> onDeleteRequested;
    std::function<void()> onSelected;
    void setSelected(bool shouldBeSelected);

    void setTriggerNote(int note);
    std::function<void(int)> onNoteChanged;
    std::function<void(const juce::String&)> onNameChanged;
    
    void setName(const juce::String& name);


private:
    SelectableLabel nameLabel;

    EnvelopeData& data;

    bool isActive = false;
    bool isHovered = false;
    bool isSelected = false;

    juce::TextButton noteButton{ "-" };
    juce::DrawableButton saveButton{ "save", juce::DrawableButton::ImageFitted };
    juce::DrawableButton replaceButton{ "replace", juce::DrawableButton::ImageFitted };
    juce::DrawableButton deleteButton{ "delete", juce::DrawableButton::ImageFitted };

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeRowComponent)
};
