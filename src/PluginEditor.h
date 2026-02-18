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
#include "ui/sections/GridSection.h"
#include "ui/sections/MeterSection.h"
#include "ui/sections/PresetSection.h"

//==============================================================================
/**
*/
class DuqAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    DuqAudioProcessorEditor (DuqAudioProcessor&);
    ~DuqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::UndoManager& undoManager;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    std::unique_ptr<FlatKnobLookAndFeel> knobLookAndFeel;

    DuqAudioProcessor& audioProcessor;
    HeaderSection header;
    EnvelopeListSection envelopeListSection;
    ControlSection controlSection;
    GridSection gridSection;
    MeterSection meterSection;
    PresetSection presetSection;

    juce::TooltipWindow tooltipWindow{ this };

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessorEditor)
};
