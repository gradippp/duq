#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

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
