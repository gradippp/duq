/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DuqAudioProcessor::DuqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for (auto& n : activeNotes)
        n.store(false);
}

DuqAudioProcessor::~DuqAudioProcessor()
{
}

void DuqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
            activeNotes[msg.getNoteNumber()].store(true, std::memory_order_relaxed);

        else if (msg.isNoteOff())
            activeNotes[msg.getNoteNumber()].store(false, std::memory_order_relaxed);
    }

    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    float inputPeak = 0.0f;

    // --- Get write position ---
    int writeIndex = monpos.load(std::memory_order_relaxed);

    for (int i = 0; i < numSamples; ++i)
    {
        float mixedSample = 0.0f;

        // Mix all channels to mono
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s = buffer.getReadPointer(ch)[i];
            mixedSample += s;
            inputPeak = std::max(inputPeak, std::abs(s));
        }

        mixedSample /= (float)numChannels;

        // Write into circular monitor buffer
        monSamples[writeIndex] = mixedSample;

        writeIndex++;
        if (writeIndex >= monitorBufferSize)
            writeIndex = 0;
    }

    monpos.store(writeIndex, std::memory_order_relaxed);

    inputMeterLevel.store(inputPeak, std::memory_order_relaxed);
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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
void DuqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DuqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DuqAudioProcessor();
}
