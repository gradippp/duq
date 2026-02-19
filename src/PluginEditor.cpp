/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/utils/PresetManager.h"
#include "model/EnvelopeData.h"

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
    controlSection.setProcessor(&p);
    presetSection.setUndoManager(undoManager);

    envelopeListSection.onReplaceRequested = [this](juce::ValueTree env)
        {
            presetSection.setMode(PresetSection::Mode::Envelope);
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

    presetSection.onProjectLoaded = [this](juce::String name)
        {
            header.setPresetName(name);
        };

    header.setAboutCallback([this]
        {
            aboutSection.setVisible(true);
            gridSection.setVisible(false);
            presetSection.setVisible(false);
            resized();
        });

    header.setupAttachments(audioProcessor.parameters);

    aboutSection.setVisible(false);
    aboutSection.onClose = [this]
        {
            aboutSection.setVisible(false);
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

    header.setSaveProjectCallback([this]
        {
            auto initialFile = PresetManager::getProjectDirectory()
                .getChildFile("Project");

            auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

            auto chooser = std::make_shared<juce::FileChooser>("Save Project Preset",
                initialFile,
                "*" + PresetManager::projectExtension);

                            chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
                            {
                                auto file = fc.getResult();
                                if (file == juce::File())
                                    return;
            
                                if (PresetManager::saveProject(audioProcessor.getEnvelopesTree(), file))
                                {
                                    header.setPresetName(file.getFileNameWithoutExtension());
                                }
                            });
            
        });

    header.setLoadProjectCallback([this]
        {
            presetSection.setMode(PresetSection::Mode::Project);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
        });

    header.setInitPresetCallback([this]
        {
            undoManager.beginNewTransaction("Init Project");
            auto envelopes = audioProcessor.getEnvelopesTree();
            envelopes.removeAllChildren(&undoManager);
            
            // Add a single default envelope
            juce::ValueTree env("ENVELOPE");
            env.setProperty("name", "Env 1", nullptr);
            env.setProperty("triggerNote", 36, nullptr);
            env.setProperty("rate", 2.0, nullptr);
            env.setProperty("depth", 100.0, nullptr);
            env.setProperty("smooth", 0.0, nullptr);
            env.setProperty("rateIsFrequencyMode", true, nullptr);

            juce::ValueTree points("POINTS");
            juce::ValueTree p1("POINT"); p1.setProperty("x", 0.0f, nullptr); p1.setProperty("y", 1.0f, nullptr);
            juce::ValueTree p2("POINT"); p2.setProperty("x", 0.5f, nullptr); p2.setProperty("y", 0.0f, nullptr);
            juce::ValueTree p3("POINT"); p3.setProperty("x", 1.0f, nullptr); p3.setProperty("y", 1.0f, nullptr);
            points.addChild(p1, -1, nullptr);
            points.addChild(p2, -1, nullptr);
            points.addChild(p3, -1, nullptr);
            env.addChild(points, -1, nullptr);

            juce::ValueTree segments("SEGMENTS");
            juce::ValueTree s1("SEGMENT");
            s1.setProperty("curve", 0.5f, nullptr);
            s1.setProperty("type", (int)CurveType::Exponential, nullptr);
            segments.addChild(s1, -1, nullptr);

            juce::ValueTree s2("SEGMENT");
            s2.setProperty("curve", 0.5f, nullptr);
            s2.setProperty("type", (int)CurveType::Exponential, nullptr);
            segments.addChild(s2, -1, nullptr);

            env.addChild(segments, -1, nullptr);

            envelopes.addChild(env, -1, &undoManager);
            header.setPresetName("Default Project");
        });

    startTimerHz(10);

    controlSection.getRateKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getDepthKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());
    controlSection.getSmoothKnob().getSlider().setLookAndFeel(knobLookAndFeel.get());

    gridSection.setSampleBuffer(
        &audioProcessor.getMonitorWritePosition(),
        audioProcessor.getMonitorSamples(),
        audioProcessor.getMonitorBufferSize());

    gridSection.setProcessor(&audioProcessor);

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
    addAndMakeVisible(aboutSection);
    aboutSection.toFront(false);
    aboutSection.setVisible(false);
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
    aboutSection.setBounds(getLocalBounds());
    meterSection.setBounds(meterArea);
}


