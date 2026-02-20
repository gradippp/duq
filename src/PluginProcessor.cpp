/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "model/EnvelopeData.h"
#include "dsp/EnvelopeEvaluator.h"
#include "Globals.h"

static juce::ValueTree createDefaultEnvelope(const juce::String& name, int note)
{
    juce::SharedResourcePointer<ConfigManager> config;
    juce::ValueTree env("ENVELOPE");

    auto controls = config->getDefaultControls();
    auto shape = config->getDefaultShape();

    env.setProperty("name", name, nullptr);
    env.setProperty("triggerNote", note >= 0 ? note : controls.triggerNote, nullptr);
    env.setProperty("rate", controls.rate, nullptr);
    env.setProperty("depth", (double)controls.depth, nullptr);
    env.setProperty("smooth", (double)controls.smooth, nullptr);
    env.setProperty("rateIsFrequencyMode", controls.rateIsFrequencyMode, nullptr);

    shape.applyToValueTree(env);

    return env;
}

//==============================================================================
DuqAudioProcessor::DuqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, &undoManager, "PARAMETERS", createParameterLayout())
#endif
{
    for (auto& n : activeNotes)
        n.store(false);

    parameters.state.getOrCreateChildWithName("ENVELOPES", &undoManager);

    auto envelopes = getEnvelopesTree();
    
    // Listen to the ROOT state for global params
    parameters.state.addListener(this);
    // Listen to ENVELOPES for reordering and additions
    envelopes.addListener(this);

    // Listen to all automation parameters
    parameters.addParameterListener("mix", this);
    parameters.addParameterListener("lookahead", this);
    parameters.addParameterListener("lookbehind", this);
    for (int i = 0; i < 12; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        parameters.addParameterListener(prefix + "rate", this);
        parameters.addParameterListener(prefix + "depth", this);
        parameters.addParameterListener(prefix + "smooth", this);
    }

    if (envelopes.getNumChildren() == 0)
    {
        addEnvelope(Theme::Defaults::envelopeName + " 1", Theme::Defaults::triggerNote);
    }

    voices.resize(maxVoices);
    for (auto& v : voices) v.isActive = false;

    syncToDSP();
    startTimerHz(30); // Sync DSP state at 30Hz if dirty
}

DuqAudioProcessor::~DuqAudioProcessor()
{
    stopTimer();
    parameters.state.removeListener(this);
    getEnvelopesTree().removeListener(this);

    parameters.removeParameterListener("mix", this);
    parameters.removeParameterListener("lookahead", this);
    parameters.removeParameterListener("lookbehind", this);
    for (int i = 0; i < 12; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        parameters.removeParameterListener(prefix + "rate", this);
        parameters.removeParameterListener(prefix + "depth", this);
        parameters.removeParameterListener(prefix + "smooth", this);
    }
}

void DuqAudioProcessor::timerCallback()
{
    if (requiresSync)
    {
        syncToDSP();
        requiresSync = false;
    }
}

void DuqAudioProcessor::syncToDSP()
{
    std::vector<DSPEnvelope> newEnvelopes;
    auto envelopesTree = getEnvelopesTree();

    for (int i = 0; i < envelopesTree.getNumChildren(); ++i)
    {
        auto envVT = envelopesTree.getChild(i);
        DSPEnvelope de;
        
        float rawRate, rawDepth, rawSmooth;

        // Priority: Use automated parameters for first 12 slots
        if (i < 12)
        {
            juce::String prefix = "env" + juce::String(i) + "_";
            rawRate = parameters.getRawParameterValue(prefix + "rate")->load();
            rawDepth = parameters.getRawParameterValue(prefix + "depth")->load();
            rawSmooth = parameters.getRawParameterValue(prefix + "smooth")->load();
        }
        else
        {
            rawRate = envVT.getProperty("rate", 2.0);
            rawDepth = envVT.getProperty("depth", 100.0);
            rawSmooth = envVT.getProperty("smooth", 0.0);
        }

        bool isFreq = (bool)envVT.getProperty("rateIsFrequencyMode", true);

        if (isFreq)
        {
            de.rate = (double)rawRate;
        }
        else
        {
            // Sync divisions: 1/1, 1/2, 1/4, 1/8, 1/16, 1/32
            static const double cycleMultipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            
            int idx = juce::jlimit(0, 5, (int)(rawRate / 16.66f));
            de.rate = cycleMultipliers[idx];
        }

        de.depth = rawDepth / 100.0f;
        de.smooth = rawSmooth / 100.0f;
        de.triggerNote = envVT.getProperty("triggerNote", 60);
        de.isFrequencyMode = (bool)envVT.getProperty("rateIsFrequencyMode", true);
        de.isDisabled = (bool)envVT.getProperty("disabled", false);

        // Pre-calculate phase increment
        double srate = getSampleRate();
        if (srate <= 0) srate = 44100.0; // Fallback

        double currentRate = de.rate;
        if (!de.isFrequencyMode)
        {
            double bpm = 120.0;
            if (auto* playhead = getPlayHead()) {
                if (auto opt = playhead->getPosition()) {
                    if (auto b = opt->getBpm()) bpm = *b;
                }
            }
            currentRate = de.rate * (bpm / 60.0);
        }
        de.phaseIncrement = currentRate / srate;

        auto pointsVT = envVT.getChildWithName("POINTS");
        if (pointsVT.isValid())
        {
            for (int j = 0; j < pointsVT.getNumChildren(); ++j)
            {
                auto pVT = pointsVT.getChild(j);
                de.points.push_back({ (float)pVT.getProperty("x", 0.0f), 
                                      (float)pVT.getProperty("y", 0.0f) });
            }
        }

        auto segmentsVT = envVT.getChildWithName("SEGMENTS");
        if (segmentsVT.isValid())
        {
            // We expect points.size() - 1 segments.
            int numSegmentsNeeded = std::max(0, (int)de.points.size() - 1);
            for (int j = 0; j < numSegmentsNeeded; ++j)
            {
                if (j < segmentsVT.getNumChildren())
                {
                    auto sVT = segmentsVT.getChild(j);
                    de.segments.push_back({ (float)sVT.getProperty("curve", 0.5f), 
                                            (CurveType)(int)sVT.getProperty("type", (int)CurveType::Exponential) });
                }
                else
                {
                    // Fallback for missing segments
                    de.segments.push_back({ 0.5f, CurveType::Exponential });
                }
            }
        }

        newEnvelopes.push_back(std::move(de));
    }

    {
        const juce::ScopedLock sl(dspLock);
        dspState.envelopes = std::move(newEnvelopes);

        float lookaheadMs = parameters.getRawParameterValue("lookahead")->load();
        float lookbehindMs = parameters.getRawParameterValue("lookbehind")->load();
        dspState.mixPercent = parameters.getRawParameterValue("mix")->load();
        double srate = getSampleRate();

        dspState.lookaheadSamples = (int)(lookaheadMs * srate / 1000.0);
        dspState.lookbehindSamples = (int)(lookbehindMs * srate / 1000.0);

        setLatencySamples(dspState.lookaheadSamples);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DuqAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Global Parameters ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mix", 1 }, "Mix", 0.0f, 100.0f, 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lookahead", 1 },
        "Lookahead",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Theme::Defaults::lookahead,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lookbehind", 1 },
        "Lookbehind",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Theme::Defaults::lookbehind,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // --- Envelope Parameters (12 slots) ---
    for (int i = 0; i < 12; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        
        // Rate range is 0-100 (handles both Hz and Sync index)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "rate", 1 }, "Env " + juce::String(i + 1) + " Rate", 0.0f, 100.0f, 2.0f));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "depth", 1 }, "Env " + juce::String(i + 1) + " Depth", 0.0f, 100.0f, 100.0f));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "smooth", 1 }, "Env " + juce::String(i + 1) + " Smooth", 0.0f, 100.0f, 0.0f));
    }

    return { params.begin(), params.end() };
}

juce::ValueTree DuqAudioProcessor::getEnvelopesTree()
{
    return parameters.state.getOrCreateChildWithName("ENVELOPES", &undoManager);
}

void DuqAudioProcessor::addEnvelope(const juce::String& name, int note)
{
    auto envelopes = getEnvelopesTree();

    auto env = createDefaultEnvelope(name, note);

    envelopes.addChild(env, -1, &undoManager);
}

void DuqAudioProcessor::processMidi(juce::MidiBuffer& midi)
{
    const juce::ScopedLock sl(dspLock);

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            activeNotes[note].store(true, std::memory_order_relaxed);

            // Find envelopes that match this note
            for (int i = 0; i < (int)dspState.envelopes.size(); ++i)
            {
                const auto& env = dspState.envelopes[i];
                if (!env.isDisabled && env.triggerNote == note)
                {
                    // Start a new voice or retrigger
                    bool foundVoice = false;
                    for (auto& v : voices)
                    {
                        if ((v.isActive || v.isPending) && v.envelopeIndex == i && v.noteNumber == note)
                        {
                            if (dspState.lookbehindSamples > 0)
                            {
                                v.isPending = true;
                                v.isActive = false;
                                v.delaySamplesRemaining = dspState.lookbehindSamples;
                            }
                            else
                            {
                                v.isPending = false;
                                v.isActive = true;
                                v.currentPhase = 0.0;
                                v.lastSegmentIndex = 0;
                            }
                            foundVoice = true;
                            break;
                        }
                    }

                    if (!foundVoice)
                    {
                        for (auto& v : voices)
                        {
                            if (!v.isActive && !v.isPending)
                            {
                                v.envelopeIndex = i;
                                v.currentPhase = 0.0;
                                v.currentGain = 1.0f;
                                v.lastSegmentIndex = 0;
                                v.noteNumber = note;

                                if (dspState.lookbehindSamples > 0)
                                {
                                    v.isPending = true;
                                    v.isActive = false;
                                    v.delaySamplesRemaining = dspState.lookbehindSamples;
                                }
                                else
                                {
                                    v.isPending = false;
                                    v.isActive = true;
                                }
                                foundVoice = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (msg.isNoteOff())
        {
            int note = msg.getNoteNumber();
            activeNotes[note].store(false, std::memory_order_relaxed);

            // In ONE-SHOT mode, we do NOT deactivate voices on Note Off.
            // They finish when currentPhase >= 1.0.
        }
    }
}

void DuqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midi)
{
    processMidi(midi);

    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const double sampleRate = getSampleRate();

    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    float maxReduction = 0.0f;

    // --- Get write position ---
    int writeIndex = monpos.load(std::memory_order_relaxed);

    const juce::ScopedLock sl(dspLock);

    // --- Handle Manual Trigger ---
    int mTrig = manualTriggerIndex.exchange(-1);
    if (mTrig >= 0 && mTrig < (int)dspState.envelopes.size())
    {
        for (auto& v : voices)
        {
            if (!v.isActive && !v.isPending)
            {
                v.envelopeIndex = mTrig;
                v.currentPhase = 0.0;
                v.currentGain = 1.0f;
                v.lastSegmentIndex = 0;
                v.noteNumber = -1; // Manual

                if (dspState.lookbehindSamples > 0)
                {
                    v.isPending = true;
                    v.isActive = false;
                    v.delaySamplesRemaining = dspState.lookbehindSamples;
                }
                else
                {
                    v.isPending = false;
                    v.isActive = true;
                }
                break;
            }
        }
    }

    const int delaySize = delayBuffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // --- Write to Delay Buffer ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayBuffer.setSample(ch, delayWritePos, buffer.getReadPointer(ch)[i]);
        }

        // --- Read from Delay Buffer (Lookahead) ---
        int readPos = (delayWritePos - dspState.lookaheadSamples + delaySize) % delaySize;
        float sampleGain = 1.0f;

        for (auto& v : voices)
        {
            if (v.isPending)
            {
                if (--v.delaySamplesRemaining <= 0)
                {
                    v.isPending = false;
                    v.isActive = true;
                    v.currentPhase = 0.0;
                    v.lastSegmentIndex = 0;
                }
            }

            if (v.isActive && v.envelopeIndex >= 0 && v.envelopeIndex < (int)dspState.envelopes.size())
            {
                const auto& env = dspState.envelopes[v.envelopeIndex];
                
                float envVal = EnvelopeEvaluator::evaluate(env, v.currentPhase, v.lastSegmentIndex);
                
                // Map y -> y^2 for more natural volume control (logarithmic/dB-like feel)
                float mappedVal = envVal * envVal;
                
                // Apply depth: If depth is 1.0, we use mappedVal. If depth is 0.0, we use 1.0.
                float voiceGain = 1.0f - (1.0f - mappedVal) * env.depth;
                
                sampleGain *= juce::jlimit(0.0f, 1.0f, voiceGain);

                // Advance phase
                v.currentPhase += env.phaseIncrement;

                if (v.currentPhase >= 1.0)
                {
                    if (env.isFrequencyMode)
                    {
                        v.currentPhase = std::fmod(v.currentPhase, 1.0);

                        // If note is released, stop at end of cycle (noteNumber < 0 means manual trigger, which loops)
                        if (v.noteNumber >= 0 && !activeNotes[v.noteNumber].load(std::memory_order_relaxed))
                        {
                            v.isActive = false;
                            v.currentPhase = 1.0;
                        }
                    }
                    else
                    {
                        v.currentPhase = 1.0;
                        if (voiceGain >= 0.999f)
                            v.isActive = false;
                    }
                }
            }
        }

        // Capture the peak reduction within this sample loop
        maxReduction = std::max(maxReduction, 1.0f - sampleGain);

        masterGain.setTargetValue(sampleGain);
        float currentSmoothedGain = masterGain.getNextValue();

        float mixVal = dspState.mixPercent / 100.0f;

        // Write UNPROCESSED sample into monitor buffer for visualization
        float monitorSample = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            monitorSample += delayBuffer.getSample(ch, readPos);
        monitorSample /= (float)numChannels;

        monSamples[writeIndex] = monitorSample;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            float s_orig = delayBuffer.getSample(ch, readPos);
            
            // Peak input (before modulation)
            if (ch == 0) inputPeak = std::max(inputPeak, std::abs(s_orig));

            float s_wet = s_orig * currentSmoothedGain;
            float s_final = s_wet * mixVal + s_orig * (1.0f - mixVal);
            
            channelData[i] = s_final;
            
            // Peak output
            outputPeak = std::max(outputPeak, std::abs(s_final));
        }

        writeIndex++;
        if (writeIndex >= monitorBufferSize)
            writeIndex = 0;

        delayWritePos++;
        if (delayWritePos >= delaySize)
            delayWritePos = 0;
    }

    monpos.store(writeIndex, std::memory_order_relaxed);

    inputMeterLevel.store(inputPeak, std::memory_order_relaxed);
    outputMeterLevel.store(outputPeak, std::memory_order_relaxed);

    // Peak-hold release logic for meter
    if (maxReduction > reductionPeak)
        reductionPeak = maxReduction;
    else
        reductionPeak *= 0.95f; // Slower release for better visual tracking

    reductionMeterLevel.store(reductionPeak, std::memory_order_relaxed);
}

juce::UndoManager& DuqAudioProcessor::getUndoManager()
{
    return undoManager;
}

std::vector<double> DuqAudioProcessor::getActivePhasesForEnvelope(int envelopeIndex) const
{
    std::vector<double> phases;
    const juce::ScopedLock sl(dspLock);

    for (const auto& v : voices)
    {
        if (v.isActive && v.envelopeIndex == envelopeIndex)
        {
            phases.push_back(v.currentPhase);
        }
    }

    return phases;
}

double DuqAudioProcessor::getPhaseIncrement(int envelopeIndex) const
{
    const juce::ScopedLock sl(dspLock);

    if (envelopeIndex < 0 || envelopeIndex >= (int)dspState.envelopes.size())
        return 0.0;

    return dspState.envelopes[envelopeIndex].phaseIncrement;
}

void DuqAudioProcessor::resetVoices()
{
    const juce::ScopedLock sl(dspLock);
    for (auto& v : voices)
    {
        v.isActive = false;
        v.isPending = false;
        v.envelopeIndex = -1;
    }
}

//==============================================================================
const juce::String DuqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DuqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DuqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DuqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DuqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DuqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DuqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DuqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DuqAudioProcessor::getProgramName (int index)
{
    return {};
}

void DuqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DuqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    masterGain.reset(sampleRate, 0.001); // 1ms smoothing for punchy transients
    masterGain.setCurrentAndTargetValue(1.0f);

    syncToDSP();

    // Max lookahead is 100ms, size for 200ms to be safe
    int delayBufferSize = (int)(0.2 * sampleRate);
    delayBuffer.setSize(getTotalNumOutputChannels(), delayBufferSize);
    delayBuffer.clear();
    delayWritePos = 0;
}

void DuqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DuqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
bool DuqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DuqAudioProcessor::createEditor()
{
    return new DuqAudioProcessorEditor (*this);
}

//==============================================================================
void DuqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DuqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName(parameters.state.getType()))
    {
        parameters.state.removeListener(this);
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
        parameters.state.addListener(this);
        syncToDSP();
    }
}

void DuqAudioProcessor::valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& i)
{
    requiresSync = true;

    // Sync from ValueTree back to Parameter if modified externally (e.g. Preset Load)
    if (v.hasType("ENVELOPE"))
    {
        auto envelopes = getEnvelopesTree();
        int envIndex = envelopes.indexOf(v);

        if (envIndex >= 0 && envIndex < 12)
        {
            juce::String propName = i.toString();
            if (propName == "rate" || propName == "depth" || propName == "smooth")
            {
                juce::String paramID = "env" + juce::String(envIndex) + "_" + propName;
                if (auto* param = parameters.getParameter(paramID))
                {
                    float newValue = (float)(double)v.getProperty(i);
                    
                    // Only update if significantly different to avoid loops
                    if (std::abs(param->getValue() - parameters.getParameterRange(paramID).convertTo0to1(newValue)) > 0.0001f)
                    {
                        param->setValueNotifyingHost(parameters.getParameterRange(paramID).convertTo0to1(newValue));
                    }
                }
            }
        }
    }
}

//==============================================================================
void DuqAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    requiresSync = true;

    // Sync automated parameters back into the ValueTree for UI consistency
    if (parameterID.startsWith("env"))
    {
        int underscorePos = parameterID.indexOf("_");
        if (underscorePos > 3)
        {
            int envIndex = parameterID.substring(3, underscorePos).getIntValue();
            juce::String propName = parameterID.substring(underscorePos + 1);

            auto envelopes = getEnvelopesTree();
            if (envIndex >= 0 && envIndex < envelopes.getNumChildren())
            {
                auto env = envelopes.getChild(envIndex);
                env.setProperty(propName, (double)newValue, nullptr);
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DuqAudioProcessor();
}
