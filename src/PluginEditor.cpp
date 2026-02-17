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
    undoManager(p.getUndoManager())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    knobLookAndFeel = std::make_unique<FlatKnobLookAndFeel>();

    envelopeListSection.setUndoManager(undoManager);
    controlSection.setUndoManager(undoManager);
    gridSection.setUndoManager(undoManager);

    header.setUndoCallback([this]
        {
            if (undoManager.canUndo())
            {
                undoManager.undo();

                if (auto* env = envelopeListSection.getSelectedEnvelope())
                {
                    controlSection.loadEnvelope(*env);
                    gridSection.setEnvelope(env);
                }

                gridSection.repaint();
            }
        });

    header.setRedoCallback([this]
        {
            if (undoManager.canRedo())
            {
                undoManager.redo();

                if (auto* env = envelopeListSection.getSelectedEnvelope())
                {
                    controlSection.loadEnvelope(*env);
                    gridSection.setEnvelope(env);
                }

                gridSection.repaint();
            }
        });

    startTimerHz(10);

    controlSection.getRateKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getDepthKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getSmoothKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    envelopeListSection.onEnvelopeSelected = [this](EnvelopeData* env)
        {
            if (env)
                controlSection.loadEnvelope(*env);
            else
                controlSection.clearEnvelope();
        };

    controlSection.onEnvelopeChanged =
        [this](const EnvelopeData& data)
        {
            envelopeListSection.updateSelectedEnvelope(data);
            gridSection.repaint();
        };

    addAndMakeVisible(gridSection);

    setSize(900, 600);
    setResizable(true, true);
    setResizeLimits(900, 600, 1200, 900);

    tooltipWindow.setMillisecondsBeforeTipAppears(500);

    addAndMakeVisible(header);
    addAndMakeVisible(envelopeListSection);
    addAndMakeVisible(controlSection);

    addAndMakeVisible(meterSection);
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
    constexpr int meterHeight = 70;

    auto meterArea = rightArea.removeFromBottom(meterHeight);

    gridSection.setBounds(rightArea);
    meterSection.setBounds(meterArea);
}


