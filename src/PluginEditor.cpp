/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DuqAudioProcessorEditor::DuqAudioProcessorEditor(DuqAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    undoManager(p.getUndoManager()),
    meterSection(
        audioProcessor.getInputMeterLevel(),
        audioProcessor.getReductionMeterLevel(),
        audioProcessor.getOutputMeterLevel())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    knobLookAndFeel = std::make_unique<FlatKnobLookAndFeel>();

    envelopeListSection.setUndoManager(undoManager);

    envelopeListSection.onEnvelopeSelected =
        [this](juce::ValueTree env)
        {
            if (env.isValid())
            {
                controlSection.setEnvelope(env);
                gridSection.setEnvelope(env);
            }
            else
            {
                controlSection.clearEnvelope();
                gridSection.setEnvelope({});
            }
        };

    envelopeListSection.setProcessor(p);

    envelopeListSection.onReplaceRequested = [this](juce::ValueTree env)
        {
            presetSection.setTargetEnvelope(env);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
        };

    presetSection.setVisible(false);
    presetSection.onClose = [this]()
        {
            presetSection.setVisible(false);
            gridSection.setVisible(true);
            resized();
        };

    controlSection.setUndoManager(undoManager);
    gridSection.setUndoManager(undoManager);


    header.setUndoCallback([this]
        {
            if (undoManager.canUndo())
                undoManager.undo();
        });

    header.setRedoCallback([this]
        {
            if (undoManager.canRedo())
                undoManager.redo();
        });

    startTimerHz(10);

    controlSection.getRateKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getDepthKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getSmoothKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());

    gridSection.setSampleBuffer(
        &audioProcessor.getMonitorWritePosition(),
        audioProcessor.getMonitorSamples(),
        audioProcessor.getMonitorBufferSize());

    addAndMakeVisible(gridSection);

    setSize(900, 600);
    setResizable(true, true);
    setResizeLimits(900, 600, 1200, 900);

    tooltipWindow.setMillisecondsBeforeTipAppears(500);

    addAndMakeVisible(header);
    addAndMakeVisible(envelopeListSection);
    addAndMakeVisible(controlSection);

    addAndMakeVisible(meterSection);
    addAndMakeVisible(presetSection);
    presetSection.setVisible(false); // <--- ENSURE IT IS HIDDEN AFTER ADDING

    undoManager.clearUndoHistory();
}

DuqAudioProcessorEditor::~DuqAudioProcessorEditor()
{
    controlSection.getRateKnob().getSlider().setLookAndFeel(nullptr);
    controlSection.getDepthKnob().getSlider().setLookAndFeel(nullptr);
    controlSection.getSmoothKnob().getSlider().setLookAndFeel(nullptr);

    stopTimer();
}

void DuqAudioProcessorEditor::timerCallback()
{
    header.updateUndoState(
        undoManager.canUndo(),
        undoManager.canRedo());

    envelopeListSection.updateMidiActivity(audioProcessor);
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

    // ===== Split left / right =====
    constexpr int leftPanelWidth = 260;
    auto leftArea = bounds.removeFromLeft(leftPanelWidth);
    auto rightArea = bounds;

    // ===== Left Panel =====
    constexpr int controlHeight = 250;

    controlSection.setBounds(
        leftArea.removeFromBottom(controlHeight));

    envelopeListSection.setBounds(leftArea);

    // ===== Right Side =====
    constexpr int meterHeight = 100;

    auto meterArea = rightArea.removeFromBottom(meterHeight);

    gridSection.setBounds(rightArea);
    presetSection.setBounds(rightArea);
    meterSection.setBounds(meterArea);
}


