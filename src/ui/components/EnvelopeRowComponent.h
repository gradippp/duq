#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class EnvelopeRowComponent : public juce::Component
{
public:
    EnvelopeRowComponent(const juce::String& name);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    std::function<void()> onDeleteRequested;
    std::function<void()> onSelected;


private:
    juce::String envelopeName;
    bool isActive = false;
    bool isHovered = false;

    juce::DrawableButton saveButton{ "save", juce::DrawableButton::ImageFitted };
    juce::DrawableButton replaceButton{ "replace", juce::DrawableButton::ImageFitted };
    juce::DrawableButton deleteButton{ "delete", juce::DrawableButton::ImageFitted };

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeRowComponent)
};
