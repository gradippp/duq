#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/EnvelopeRowComponent.h"

class EnvelopeListComponent;

class EnvelopeListSection : public juce::Component
{
public:
    EnvelopeListSection();
    ~EnvelopeListSection() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Viewport viewport;
    juce::Component rowContainer;
    juce::TextButton addButton;
    juce::OwnedArray<EnvelopeRowComponent> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeListSection)
};
