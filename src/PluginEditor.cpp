/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DuqAudioProcessorEditor::DuqAudioProcessorEditor (DuqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(600, 400);
    setResizable(true, true);

    tooltipWindow.setMillisecondsBeforeTipAppears(500);

    addAndMakeVisible(header);
    addAndMakeVisible(envelopeListSection);
}

DuqAudioProcessorEditor::~DuqAudioProcessorEditor()
{
}

//==============================================================================
void DuqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void DuqAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // ===== Header =====
    constexpr int headerHeight = 60;
    header.setBounds(bounds.removeFromTop(headerHeight));

    // ===== Main Area =====
    constexpr int leftPanelWidth = 260;
    auto leftArea = bounds.removeFromLeft(leftPanelWidth);

    // Envelope section size
    constexpr int envelopeHeight = 203;

    envelopeListSection.setBounds(
        leftArea.removeFromTop(envelopeHeight)
    );
}

