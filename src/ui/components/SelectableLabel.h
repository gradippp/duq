#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SelectableLabel : public juce::Label
{
public:
    std::function<void()> onSingleClick;
    std::function<void()> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (onRightClick)
                onRightClick();
        }
        else
        {
            if (onSingleClick)
                onSingleClick();
        }

        juce::Label::mouseDown(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (auto* parent = getParentComponent())
            parent->mouseDrag(e.getEventRelativeTo(parent));
    }
};
