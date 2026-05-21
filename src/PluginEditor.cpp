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

    globalLookAndFeel = std::make_unique<GlobalLookAndFeel>();
    
    juce::LookAndFeel::setDefaultLookAndFeel(globalLookAndFeel.get());
    setLookAndFeel(globalLookAndFeel.get());

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
            showSection(Section::Presets);
        };

    envelopeListSection.onSaveRequested = [this](juce::ValueTree env)
        {
            presetSection.setMode(PresetSection::Mode::Envelope);
            presetSection.setTargetEnvelope(env);
            showSection(Section::Presets);
            presetSection.startSavingProcess();
        };

    envelopeListSection.onImportRequested = [this]()
        {
            presetSection.setMode(PresetSection::Mode::Import);
            showSection(Section::Presets);
        };

    presetSection.onClose = [this]()
        {
            bool returningToSettings = false;
            if (auto* workflow = settingsSection.getWorkflowPage())
            {
                if (workflow->isWaitingForImport)
                {
                    workflow->isWaitingForImport = false;
                    returningToSettings = true;
                }
            }

            if (returningToSettings) {
                showSection(Section::Settings);
            } else {
                showSection(Section::Grid);
            }
        };

    presetSection.onEnvelopeImported = [this](int newIndex)
        {
            bool handledAsDefault = false;
            if (auto* workflow = settingsSection.getWorkflowPage())
            {
                if (workflow->isWaitingForImport)
                {
                    handledAsDefault = true;
                    workflow->isWaitingForImport = false;
                }
            }

            if (handledAsDefault)
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
                    
                    // Refresh the list in the UI
                    if (auto* workflow = settingsSection.getWorkflowPage())
                    {
                        workflow->updateEnvelopeList();
                        workflow->updateShapePreview();
                    }

                    // Remove the temporary imported envelope
                    envelopes.removeChild(env, &undoManager);
                }
                
                showSection(Section::Settings);
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

    header.setAboutCallback([this] { showSection(Section::About); });
    header.setSettingsCallback([this] { showSection(Section::Settings); });
    
    settingsSection.onClose = [this] { showSection(Section::Grid); };

    settingsSection.setProcessor(&p);
    if (auto* workflow = settingsSection.getWorkflowPage())
    {
        workflow->onImportFromBrowser = [this, workflow]
        {
            workflow->isWaitingForImport = true;
            presetSection.setMode(PresetSection::Mode::Import);
            showSection(Section::Presets);
        };
    }

    aboutSection.onClose = [this] { showSection(Section::Grid); };

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
            showSection(Section::Presets);
            presetSection.startSavingProcess();
        });

    header.setLoadProjectCallback([this]
        {
            presetSection.setMode(PresetSection::Mode::Project);
            showSection(Section::Presets);
        });

    header.setInitPresetCallback([this]
        {
            undoManager.beginNewTransaction("Init Project");
            
            // Reset global parameters
            auto& vts = audioProcessor.parameters;
            if (auto* p = vts.getParameter("mix")) p->setValueNotifyingHost(p->getDefaultValue());
            if (auto* p = vts.getParameter("lookahead")) p->setValueNotifyingHost(vts.getParameterRange("lookahead").convertTo0to1(Defaults::lookahead));

            auto envelopes = audioProcessor.getEnvelopesTree();
            envelopes.removeAllChildren(&undoManager);
            
            // Add a single default envelope (uses global config defaults)
            audioProcessor.triggerEnvelope(-1); // reset trigger state
            audioProcessor.addEnvelope(Defaults::envelopeName + " 1", -1); // -1 means use config default note
            
            header.setPresetName("Default Project");
        });

    startTimerHz(10);

    gridSection.setSampleBuffers(
        &audioProcessor.getMonitorWritePosition(),
        audioProcessor.getMonitorSamplesPre(),
        audioProcessor.getMonitorSamplesPost(),
        audioProcessor.getMonitorSamplesSidechain(),
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

    showSection(Section::Grid);

    ThemeManager::getInstance().addChangeListener(this);

    undoManager.clearUndoHistory();
}

DuqAudioProcessorEditor::~DuqAudioProcessorEditor()
{
    ThemeManager::getInstance().removeChangeListener(this);
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    setLookAndFeel(nullptr);

    stopTimer();
}

void DuqAudioProcessorEditor::timerCallback()
{
    header.updateUndoState(
        undoManager.canUndo(),
        undoManager.canRedo());

    envelopeListSection.updateMidiActivity(audioProcessor);
}

void DuqAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        // 1. Refresh global LookAndFeel
        if (globalLookAndFeel) 
        {
            globalLookAndFeel->refreshColours();
            globalLookAndFeel->setDefaultSansSerifTypeface(FontManager::getJetBrainsMono(12.0f).getTypefacePtr());
        }

        // 2. Trigger LookAndFeelChanged recursively
        sendLookAndFeelChange();

        // 3. Manual refresh for sections
        header.refreshTheme();
        
        // 4. Force repaint
        repaint();
    }
}

void DuqAudioProcessorEditor::showSection(Section section)
{
    gridSection.setVisible(section == Section::Grid);
    presetSection.setVisible(section == Section::Presets);
    settingsSection.setVisible(section == Section::Settings);
    aboutSection.setVisible(section == Section::About);
    
    // Always show left panel and meters unless it's About section
    bool showMainUI = (section != Section::About);
    envelopeListSection.setVisible(showMainUI);
    controlSection.setVisible(showMainUI);
    meterSection.setVisible(showMainUI);
    
    resized();
}

//==============================================================================
void DuqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(T_COL(background));
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


