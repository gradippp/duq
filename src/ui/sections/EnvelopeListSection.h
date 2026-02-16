#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../components/EnvelopeRowComponent.h"
#include "../../model/EnvelopeData.h"

class EnvelopeListComponent;

class EnvelopeListSection : public juce::Component
{
public:
    EnvelopeListSection();
    ~EnvelopeListSection() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void selectEnvelope(int i);
    std::function<void(EnvelopeData&)> onEnvelopeSelected;
    void updateSelectedEnvelope(const EnvelopeData& data);

private:
    juce::Viewport viewport;
    juce::Component rowContainer;
    juce::TextButton addButton;
    juce::OwnedArray<EnvelopeRowComponent> rows;
    void removeRow(EnvelopeRowComponent* row);

    std::vector<EnvelopeData> envelopes;
    int selectedIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeListSection)
};
