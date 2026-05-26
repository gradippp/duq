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
#include "ui/utils/GlobalLookAndFeel.h"
#include "ui/sections/GridSection.h"
#include "ui/sections/MeterSection.h"
#include "ui/sections/PresetSection.h"
#include "ui/sections/AboutSection.h"
#include "ui/sections/SettingsSection.h"

//==============================================================================
/**
*/
class DuqAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                public juce::Timer,
                                public juce::ChangeListener
{
public:
    DuqAudioProcessorEditor (DuqAudioProcessor&);
    ~DuqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    enum class Section { Grid, Presets, Settings, About };
    void showSection(Section section);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    std::unique_ptr<GlobalLookAndFeel> globalLookAndFeel;

    DuqAudioProcessor& audioProcessor;
    juce::UndoManager& undoManager;
    HeaderSection header;
    EnvelopeListSection envelopeListSection;
    ControlSection controlSection;
    GridSection gridSection;
    MeterSection meterSection;
    PresetSection presetSection{ audioProcessor };
    AboutSection aboutSection;
    SettingsSection settingsSection;

    juce::SharedResourcePointer<ConfigManager> config;
    juce::TooltipWindow tooltipWindow{ this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessorEditor)
};
