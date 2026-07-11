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
    // One LookAndFeel shared across all editor instances (so two DUQ instances
    // in a host don't clobber each other's default LnF). The process-wide
    // default is set by the first editor and cleared by the last, via a refcount.
    juce::SharedResourcePointer<GlobalLookAndFeel> globalLookAndFeel;
    static inline int defaultLnfRefCount = 0;

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

    // Single shared 60Hz animation timer that drives the grid/waveform/meters,
    // replacing five independent per-component Timers. (The editor's own base
    // Timer stays at 10Hz for undo-state/MIDI-activity polling.)
    struct FrameTimer : public juce::Timer
    {
        std::function<void()> onTick;
        void timerCallback() override { if (onTick) onTick(); }
    };
    FrameTimer frameTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessorEditor)
};
