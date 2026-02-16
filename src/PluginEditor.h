/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/sections/HeaderSection.h"
#include "ui/sections/EnvelopeListSection.h"
#include "ui/sections/ControlSection.h"
#include "ui/utils/FlatKnobLookAndFeel.h"

//==============================================================================
/**
*/
class DuqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DuqAudioProcessorEditor (DuqAudioProcessor&);
    ~DuqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    std::unique_ptr<FlatKnobLookAndFeel> knobLookAndFeel;

    DuqAudioProcessor& audioProcessor;
    HeaderSection header;
    EnvelopeListSection envelopeListSection;
    ControlSection controlSection;
    juce::TooltipWindow tooltipWindow{ this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessorEditor)
};
