/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
*/
class DuqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DuqAudioProcessor();
    ~DuqAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::UndoManager& getUndoManager();

    std::atomic<float>& getInputMeterLevel() { return inputMeterLevel; }
    std::atomic<float>& getReductionMeterLevel() { return reductionMeterLevel; }
    std::atomic<float>& getOutputMeterLevel() { return outputMeterLevel; }

    int getMonitorBufferSize() const noexcept { return monitorBufferSize; }
    const float* getMonitorSamples() const noexcept { return monSamples; }
    const std::atomic<int>& getMonitorWritePosition() const noexcept { return monpos; }

    bool DuqAudioProcessor::isNoteActive(int note) const
    {
        return activeNotes[note].load(std::memory_order_relaxed);
    }

private:
    juce::UndoManager undoManager { 200 };

    std::atomic<float> inputMeterLevel{ 0.0f };
    std::atomic<float> reductionMeterLevel{ 0.0f };
    std::atomic<float> outputMeterLevel{ 0.0f };

    static constexpr int monitorBufferSize = 2048;
    float monSamples[monitorBufferSize];
    std::atomic<int> monpos{ 0 };

    std::array<std::atomic<bool>, 128> activeNotes;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuqAudioProcessor)
};
