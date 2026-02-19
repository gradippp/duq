/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "model/EnvelopeData.h"
#include "dsp/EnvelopeEvaluator.h"

static juce::ValueTree createDefaultEnvelope(const juce::String& name, int note)
{
    juce::ValueTree env("ENVELOPE");

    env.setProperty("name", name, nullptr);
    env.setProperty("triggerNote", note, nullptr);
    env.setProperty("rate", 2.0, nullptr); // 2Hz or 1/4 note (index 2)
    env.setProperty("depth", 100.0, nullptr);
    env.setProperty("smooth", 0.0, nullptr);
    env.setProperty("rateIsFrequencyMode", true, nullptr);

    juce::ValueTree points("POINTS");

    juce::ValueTree p1("POINT");
    p1.setProperty("x", 0.0f, nullptr);
    p1.setProperty("y", 1.0f, nullptr);

    juce::ValueTree p2("POINT");
    p2.setProperty("x", 0.5f, nullptr);
    p2.setProperty("y", 0.0f, nullptr);

    juce::ValueTree p3("POINT");
    p3.setProperty("x", 1.0f, nullptr);
    p3.setProperty("y", 1.0f, nullptr);

    points.addChild(p1, -1, nullptr);
    points.addChild(p2, -1, nullptr);
    points.addChild(p3, -1, nullptr);

    juce::ValueTree segments("SEGMENTS");
    
    juce::ValueTree s1("SEGMENT");
    s1.setProperty("curve", 0.5f, nullptr);
    s1.setProperty("type", (int)CurveType::Exponential, nullptr);
    segments.addChild(s1, -1, nullptr);

    juce::ValueTree s2("SEGMENT");
    s2.setProperty("curve", 0.5f, nullptr);
    s2.setProperty("type", (int)CurveType::Exponential, nullptr);
    segments.addChild(s2, -1, nullptr);

    env.addChild(points, -1, nullptr);
    env.addChild(segments, -1, nullptr);

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
    envelopes.addListener(this);

    if (envelopes.getNumChildren() == 0)
    {
        addEnvelope("Env 1", 36);
    }

    voices.resize(maxVoices);
    for (auto& v : voices) v.isActive = false;

    syncToDSP();
}

DuqAudioProcessor::~DuqAudioProcessor()
{
    getEnvelopesTree().removeListener(this);
}

void DuqAudioProcessor::syncToDSP()
{
    std::vector<DSPEnvelope> newEnvelopes;
    auto envelopesTree = getEnvelopesTree();

    for (int i = 0; i < envelopesTree.getNumChildren(); ++i)
    {
        auto envVT = envelopesTree.getChild(i);
        DSPEnvelope de;
        
        float rawRate = envVT.getProperty("rate", 20.0);
        bool isFreq = (bool)envVT.getProperty("rateIsFrequencyMode", true);

        if (isFreq)
        {
            de.rate = (double)rawRate;
        }
        else
        {
            // rateDivisions: 1/1, 1/2, 1/4, 1/8, 1/16, 1/32
            // We want cycles per BEAT.
            // 1/1 (whole bar) = 0.25 cycles per beat
            // 1/2 (half note) = 0.5 cycles per beat
            // 1/4 (quarter note) = 1.0 cycles per beat
            // ...
            static const double cycleMultipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            int idx = juce::jlimit(0, 5, (int)rawRate);
            de.rate = cycleMultipliers[idx];
        }

        de.depth = (float)envVT.getProperty("depth", 100.0) / 100.0f;
        de.smooth = (float)envVT.getProperty("smooth", 0.0) / 100.0f;
        de.triggerNote = envVT.getProperty("triggerNote", 60);
        de.isFrequencyMode = (bool)envVT.getProperty("rateIsFrequencyMode", true);
        de.isDisabled = (bool)envVT.getProperty("disabled", false);

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
            for (int j = 0; j < segmentsVT.getNumChildren(); ++j)
            {
                auto sVT = segmentsVT.getChild(j);
                de.segments.push_back({ (float)sVT.getProperty("curve", 0.5f), 
                                        (CurveType)(int)sVT.getProperty("type", (int)CurveType::Exponential) });
            }
        }

        newEnvelopes.push_back(std::move(de));
    }

    {
        const juce::ScopedLock sl(dspLock);
        dspState.envelopes = std::move(newEnvelopes);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DuqAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Add real parameters here later.
    // For now, empty layout is valid.

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
                        if (v.isActive && v.envelopeIndex == i && v.noteNumber == note)
                        {
                            v.currentPhase = 0.0;
                            v.lastSegmentIndex = 0;
                            // Do not reset currentGain, let it smooth to the new start value
                            foundVoice = true;
                            break;
                        }
                    }

                    if (!foundVoice)
                    {
                        for (auto& v : voices)
                        {
                            if (!v.isActive)
                            {
                                v.envelopeIndex = i;
                                v.currentPhase = 0.0;
                                v.currentGain = 1.0f;
                                v.lastSegmentIndex = 0;
                                v.isActive = true;
                                v.noteNumber = note;
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

    for (int i = 0; i < numSamples; ++i)
    {
        float aggregateGain = 1.0f;

        for (auto& v : voices)
        {
            if (v.isActive && v.envelopeIndex >= 0 && v.envelopeIndex < (int)dspState.envelopes.size())
            {
                const auto& env = dspState.envelopes[v.envelopeIndex];
                
                float envVal = EnvelopeEvaluator::evaluate(env, v.currentPhase, v.lastSegmentIndex);
                float targetGain = 1.0f - (1.0f - envVal) * env.depth;

                // Smoothing (One-pole LPF)
                float alpha = 1.0f;
                if (env.smooth > 0.001f)
                {
                    double tc = 0.001 + (double)env.smooth * 0.5; // up to 500ms
                    alpha = (float)(1.0 / (tc * sampleRate + 1.0));
                }
                v.currentGain += alpha * (targetGain - v.currentGain);
                
                aggregateGain *= v.currentGain;

                // Advance phase
                double currentRate = env.rate;
                
                if (!env.isFrequencyMode)
                {
                    if (auto* playhead = getPlayHead())
                    {
                        if (auto opt = playhead->getPosition())
                        {
                            if (auto bpm = opt->getBpm())
                            {
                                double beatsPerSecond = *bpm / 60.0;
                                currentRate = env.rate * beatsPerSecond;
                            }
                        }
                    }
                }

                v.currentPhase += currentRate / sampleRate;
                
                // One-Shot Deactivation
                if (v.currentPhase >= 1.0)
                {
                    v.isActive = false;
                }
            }
        }

        maxReduction = std::max(maxReduction, 1.0f - aggregateGain);

        masterGain.setTargetValue(aggregateGain);
        float currentSmoothedGain = masterGain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            float s = channelData[i];
            
            // Peak input (before modulation)
            if (ch == 0) inputPeak = std::max(inputPeak, std::abs(s));

            s *= currentSmoothedGain;
            channelData[i] = s;
            
            // Peak output
            outputPeak = std::max(outputPeak, std::abs(s));
        }

        // Write processed (ducked) sample into monitor buffer
        float monitorSample = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            monitorSample += buffer.getReadPointer(ch)[i];
        monitorSample /= (float)numChannels;

        monSamples[writeIndex] = monitorSample;

        writeIndex++;
        if (writeIndex >= monitorBufferSize)
            writeIndex = 0;
    }

    monpos.store(writeIndex, std::memory_order_relaxed);

    inputMeterLevel.store(inputPeak, std::memory_order_relaxed);
    outputMeterLevel.store(outputPeak, std::memory_order_relaxed);
    reductionMeterLevel.store(maxReduction, std::memory_order_relaxed);
}

juce::UndoManager& DuqAudioProcessor::getUndoManager()
{
    return undoManager;
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
    masterGain.reset(sampleRate, 0.002); // Faster 2ms smoothing for aggregate gain
    masterGain.setCurrentAndTargetValue(1.0f);
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
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DuqAudioProcessor();
}
