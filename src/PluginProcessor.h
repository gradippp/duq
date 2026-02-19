#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/EnvelopeProcessor.h"

//==============================================================================
class DuqAudioProcessor : public juce::AudioProcessor,
    private juce::ValueTree::Listener
{
public:
    //==============================================================================
    DuqAudioProcessor();
    ~DuqAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // APVTS
    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // Envelope tree access
    juce::ValueTree getEnvelopesTree();
    juce::UndoManager& getUndoManager();

    //==============================================================================
    // Meters
    std::atomic<float>& getInputMeterLevel() { return inputMeterLevel; }
    std::atomic<float>& getReductionMeterLevel() { return reductionMeterLevel; }
    std::atomic<float>& getOutputMeterLevel() { return outputMeterLevel; }

    //==============================================================================
    // Monitor buffer
    int getMonitorBufferSize() const noexcept { return monitorBufferSize; }
    const float* getMonitorSamples() const noexcept { return monSamples; }
    const std::atomic<int>& getMonitorWritePosition() const noexcept { return monpos; }

    //==============================================================================
    bool isNoteActive(int note) const
    {
        return activeNotes[note].load(std::memory_order_relaxed);
    }

private:
    //==============================================================================
    void syncToDSP();
    void processMidi(juce::MidiBuffer& midi);

    // ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override { syncToDSP(); }
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override { syncToDSP(); }
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override { syncToDSP(); }
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override { syncToDSP(); }

    //==============================================================================
    juce::UndoManager undoManager{ 200 };
    void addEnvelope(const juce::String& name, int note);

    // DSP State
    struct InternalDSPState
    {
        std::vector<DSPEnvelope> envelopes;
    } dspState;

    juce::CriticalSection dspLock;
    std::vector<EnvelopeVoice> voices;
    static constexpr int maxVoices = 32;

    juce::LinearSmoothedValue<float> masterGain{ 1.0f };

    std::atomic<float> inputMeterLevel{ 0.0f };
    std::atomic<float> reductionMeterLevel{ 0.0f };
    std::atomic<float> outputMeterLevel{ 0.0f };

    //==============================================================================
    static constexpr int monitorBufferSize = 2048;
    float monSamples[monitorBufferSize]{};
    std::atomic<int> monpos{ 0 };

    std::array<std::atomic<bool>, 128> activeNotes{};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DuqAudioProcessor)
};
