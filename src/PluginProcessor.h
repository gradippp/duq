#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/EnvelopeProcessor.h"
#include "utils/ConfigManager.h"

//==============================================================================
class DuqAudioProcessor : public juce::AudioProcessor,
    private juce::ValueTree::Listener,
    private juce::AudioProcessorValueTreeState::Listener,
    private juce::Timer,
    public juce::ChangeListener,
    public juce::AudioProcessorParameter::Listener
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
    const float* getMonitorSamplesPre() const noexcept { return monSamplesPre; }
    const float* getMonitorSamplesPost() const noexcept { return monSamplesPost; }
    const float* getMonitorSamplesSidechain() const noexcept { return monSamplesSidechain; }
    const std::atomic<int>& getMonitorWritePosition() const noexcept { return monpos; }

    //==============================================================================
    bool isNoteActive(int note) const
    {
        return activeNotes[static_cast<size_t>(note)].load(std::memory_order_relaxed);
    }

    void triggerEnvelope(int index) { manualTriggerIndex = index; }
    void resetVoices();

    void addEnvelope(const juce::String& name, int note);

    std::vector<double> getActivePhasesForEnvelope(int envelopeIndex) const;
    double getPhaseIncrement(int envelopeIndex) const;

    ConfigManager& getConfig() { return *config; }

private:
    //==============================================================================
    void syncToDSP();
    void processMidi(juce::MidiBuffer& midi);

    // ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override { requiresSync = true; }
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override { requiresSync = true; resetVoices(); }
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override { requiresSync = true; resetVoices(); }

    // APVTS::Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // AudioProcessorParameter::Listener
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    // Timer
    void timerCallback() override;

    //==============================================================================
    juce::UndoManager undoManager{ 200 };

    juce::AudioParameterInt* undoTriggerParam = nullptr;
    std::atomic<int> lastUndoTriggerValue{ 0 };
    std::atomic<bool> isInternalAction{ false };
    std::atomic<bool> isHostUndoing{ false };

    // DSP State
    struct InternalDSPState
    {
        std::vector<DSPEnvelope> envelopes;
        int lookaheadSamples = 0;
        float mixPercent = 100.0f;
    } dspState;

    juce::CriticalSection dspLock;
    std::atomic<bool> requiresSync{ true };
    std::atomic<int> manualTriggerIndex{ -1 };

    std::vector<EnvelopeVoice> voices;
    static constexpr int maxVoices = 32;

    juce::AudioBuffer<float> delayBuffer;
    int delayWritePos = 0;

    juce::LinearSmoothedValue<float> masterGain{ 1.0f };
    float reductionPeak = 0.0f;

    std::atomic<float> inputMeterLevel{ 0.0f };
    std::atomic<float> reductionMeterLevel{ 0.0f };
    std::atomic<float> outputMeterLevel{ 0.0f };

    //==============================================================================
    static constexpr int monitorBufferSize = 2048;
    float monSamplesPre[monitorBufferSize]{};
    float monSamplesPost[monitorBufferSize]{};
    float monSamplesSidechain[monitorBufferSize]{};
    std::atomic<int> monpos{ 0 };

    std::array<std::atomic<bool>, 128> activeNotes{};

    juce::SharedResourcePointer<ConfigManager> config;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DuqAudioProcessor)
};
