/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/sections/HeaderComponent.h"
#include "ui/sections/EnvelopeListSection.h"

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
    DuqAudioProcessor& audioProcessor;
    HeaderComponent header;
    EnvelopeListSection envelopeListSection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessorEditor)
};
