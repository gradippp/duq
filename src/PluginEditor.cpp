/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/sections/settings/WorkflowPage.h"
#include "utils/PresetManager.h"
#include "model/EnvelopeData.h"
#include "Globals.h"

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
    contextMenuLookAndFeel = std::make_unique<ContextMenuLookAndFeel>();
    
    juce::LookAndFeel::setDefaultLookAndFeel(contextMenuLookAndFeel.get());
    setLookAndFeel(contextMenuLookAndFeel.get());

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

    controlSection.setProcessor(&p);
    envelopeListSection.setProcessor(p);
    presetSection.setUndoManager(undoManager);

    envelopeListSection.onReplaceRequested = [this](juce::ValueTree env)
        {
            presetSection.setMode(PresetSection::Mode::Envelope);
            presetSection.setTargetEnvelope(env);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
        };

    envelopeListSection.onSaveRequested = [this](juce::ValueTree env)
        {
            presetSection.setMode(PresetSection::Mode::Envelope);
            presetSection.setTargetEnvelope(env);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
            presetSection.startSavingProcess();
        };

    envelopeListSection.onImportRequested = [this]()
        {
            presetSection.setMode(PresetSection::Mode::Import);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
        };

    presetSection.setVisible(false);
    presetSection.onClose = [this]()
        {
            presetSection.setVisible(false);
            if (settingsSection.isVisible()) {
                // Keep settings visible
            } else {
                gridSection.setVisible(true);
            }
            resized();
        };

    presetSection.onEnvelopeImported = [this](int newIndex)
        {
            if (presetSection.getMode() == PresetSection::Mode::Import && settingsSection.isVisible())
            {
                auto envelopes = audioProcessor.getEnvelopesTree();
                auto env = envelopes.getChild(newIndex);
                if (env.isValid())
                {
                    auto points = env.getChildWithName("POINTS");
                    auto segments = env.getChildWithName("SEGMENTS");
                    
                    juce::ValueTree shape("SHAPE");
                    if (points.isValid()) shape.addChild(points.createCopy(), -1, nullptr);
                    if (segments.isValid()) shape.addChild(segments.createCopy(), -1, nullptr);

                    if (auto xml = shape.createXml())
                    {
                        juce::SharedResourcePointer<ConfigManager> config;
                        config->setDefaultPoints(xml->toString());
                    }
                    
                    // Remove the temporary imported envelope
                    envelopes.removeChild(env, &undoManager);
                }
                
                presetSection.setVisible(false);
                settingsSection.setVisible(true);
                resized();
            }
            else
            {
                envelopeListSection.selectEnvelope(newIndex);
            }
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

    header.setSettingsCallback([this]
        {
            settingsSection.setVisible(true);
            gridSection.setVisible(false);
            presetSection.setVisible(false);
            resized();
        });
    
    settingsSection.setVisible(false);
    settingsSection.onClose = [this]
        {
            settingsSection.setVisible(false);
            gridSection.setVisible(true);
            resized();
        };

    settingsSection.setProcessor(&p);
    if (auto* workflow = settingsSection.getWorkflowPage())
    {
        workflow->onImportFromBrowser = [this]
        {
            presetSection.setMode(PresetSection::Mode::Import);
            gridSection.setVisible(false);
            settingsSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
        };
    }

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
            presetSection.setMode(PresetSection::Mode::Project);
            gridSection.setVisible(false);
            presetSection.setVisible(true);
            resized();
            presetSection.startSavingProcess();
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
            
            // Reset global parameters
            auto& vts = audioProcessor.parameters;
            if (auto* p = vts.getParameter("mix")) p->setValueNotifyingHost(p->getDefaultValue());
            if (auto* p = vts.getParameter("lookahead")) p->setValueNotifyingHost(vts.getParameterRange("lookahead").convertTo0to1(Theme::Defaults::lookahead));
            if (auto* p = vts.getParameter("lookbehind")) p->setValueNotifyingHost(vts.getParameterRange("lookbehind").convertTo0to1(Theme::Defaults::lookbehind));

            auto envelopes = audioProcessor.getEnvelopesTree();
            envelopes.removeAllChildren(&undoManager);
            
            // Add a single default envelope
            juce::ValueTree env("ENVELOPE");
            env.setProperty("name", Theme::Defaults::envelopeName + " 1", nullptr);
            env.setProperty("triggerNote", Theme::Defaults::triggerNote, nullptr);
            env.setProperty("rate", Theme::Defaults::rate, nullptr);
            env.setProperty("depth", (double)Theme::Defaults::depth, nullptr);
            env.setProperty("smooth", (double)Theme::Defaults::smooth, nullptr);
            env.setProperty("rateIsFrequencyMode", Theme::Defaults::rateIsFrequencyMode, nullptr);

            juce::ValueTree points("POINTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints; ++i)
            {
                juce::ValueTree p("POINT");
                p.setProperty("x", Theme::Defaults::defaultPoints[i].x, nullptr);
                p.setProperty("y", Theme::Defaults::defaultPoints[i].y, nullptr);
                points.addChild(p, -1, nullptr);
            }
            env.addChild(points, -1, nullptr);

            juce::ValueTree segments("SEGMENTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints - 1; ++i)
            {
                juce::ValueTree s("SEGMENT");
                s.setProperty("curve", Theme::Defaults::curve, nullptr);
                s.setProperty("type", Theme::Defaults::curveType, nullptr);
                segments.addChild(s, -1, nullptr);
            }

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
    addAndMakeVisible(settingsSection);
    aboutSection.toFront(false);
    aboutSection.setVisible(false);
    settingsSection.setVisible(false);
    presetSection.setVisible(false); // <--- ENSURE IT IS HIDDEN AFTER ADDING

    undoManager.clearUndoHistory();
}

DuqAudioProcessorEditor::~DuqAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    setLookAndFeel(nullptr);

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
    g.fillAll(Theme::Colours::background);
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
    settingsSection.setBounds(rightArea);
    aboutSection.setBounds(getLocalBounds());
    meterSection.setBounds(meterArea);
}


