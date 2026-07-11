/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "model/EnvelopeData.h"
#include "dsp/EnvelopeEvaluator.h"
#include "dsp/EnvelopeSmoothing.h"
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
        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, &undoManager, "PARAMETERS", createParameterLayout())
#endif
{
    undoManager.setMaxNumberOfStoredUnits(30000, config->getUndoLimit());

    for (size_t i = 0; i < activeNotes.size(); ++i)
        activeNotes[i].store(false);

    parameters.state.getOrCreateChildWithName("ENVELOPES", &undoManager);

    auto envelopes = getEnvelopesTree();
    
    // Listen to the ROOT state for global params
    parameters.state.addListener(this);
    // Listen to ENVELOPES for reordering and additions
    envelopes.addListener(this);

    // Listen to all automation parameters
    parameters.addParameterListener("mix", this);
    parameters.addParameterListener("lookahead", this);
    for (int i = 0; i < Defaults::maxEnvelopeSlots; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        parameters.addParameterListener(prefix + "rate", this);
        parameters.addParameterListener(prefix + "depth", this);
        parameters.addParameterListener(prefix + "smooth", this);
    }

    // Host-undo bridge parameter. Kept automatable: hosts reliably record
    // automatable parameter changes in their own undo stack, which is what lets
    // host Ctrl-Z drive our internal UndoManager. (A non-automatable variant
    // hides it from automation lanes but silently breaks host-undo capture in
    // some hosts, so we intentionally keep it automatable.)
    undoTriggerParam = new juce::AudioParameterInt(juce::ParameterID{ "undoTrigger", 1 }, "Undo Trigger", 0, 1000000, 0);
    addParameter(undoTriggerParam);
    undoTriggerParam->addListener(this);
    lastUndoTriggerValue = undoTriggerParam->get();

    undoManager.addChangeListener(this);
    config->addChangeListener(this);

// Envelope Voices
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
    for (int i = 0; i < Defaults::maxEnvelopeSlots; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        parameters.removeParameterListener(prefix + "rate", this);
        parameters.removeParameterListener(prefix + "depth", this);
        parameters.removeParameterListener(prefix + "smooth", this);
    }

    if (undoTriggerParam)
        undoTriggerParam->removeListener(this);

    undoManager.removeChangeListener(this);
    config->removeChangeListener(this);
}

void DuqAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &config.get())
    {
        undoManager.setMaxNumberOfStoredUnits(30000, config->getUndoLimit());
    }
    else if (source == &undoManager && !isHostUndoing.load())
    {
        // An internal edit happened: bump the host-undo bridge parameter so the
        // host records it in its own undo stack. Guarded by isInternalAction so
        // the resulting parameterValueChanged echo is ignored.
        if (undoTriggerParam != nullptr)
        {
            isInternalAction.store(true);
            int nextValue = undoTriggerParam->get() + 1;
            lastUndoTriggerValue.store(nextValue);

            undoTriggerParam->beginChangeGesture();
            undoTriggerParam->setValueNotifyingHost(undoTriggerParam->convertTo0to1((float)nextValue));
            undoTriggerParam->endChangeGesture();
            isInternalAction.store(false);
        }
    }
}

void DuqAudioProcessor::parameterValueChanged(int parameterIndex, float /*newValue*/)
{
    if (undoTriggerParam == nullptr || parameterIndex != undoTriggerParam->getParameterIndex())
        return;

    int newTriggerValue = undoTriggerParam->get();
    int lastVal = lastUndoTriggerValue.load();

    // Ignore echo from our own notifications or duplicate host events.
    if (newTriggerValue == lastVal)
        return;

    if (!isInternalAction.load())
    {
        // Host moved the bridge parameter (e.g. Ctrl-Z/Ctrl-Y reverting it).
        // Defer the actual undo/redo to the message-thread timer so we never
        // touch the UndoManager (or allocate) on the audio thread.
        lastUndoTriggerValue.store(newTriggerValue);
        pendingHostUndoRedo.store(newTriggerValue < lastVal ? -1 : 1);
    }
    else
    {
        lastUndoTriggerValue.store(newTriggerValue);
    }
}

void DuqAudioProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    juce::ignoreUnused(parameterIndex, gestureIsStarting);
}

void DuqAudioProcessor::performUndoRedo(bool isUndo)
{
    // Suppress the undoManager change-callback → undoTriggerParam bump while we
    // perform a host-driven (or button-driven) undo, so we don't re-record it.
    isHostUndoing.store(true);
    if (isUndo) undoManager.undo();
    else        undoManager.redo();
    triggerAsyncUpdate();
}

void DuqAudioProcessor::handleAsyncUpdate()
{
    isHostUndoing.store(false);

    int capturedNote = capturedCalibrationNote.exchange(-1);
    if (capturedNote != -1)
    {
        // Calculate offset: offset = 1 - (note / 12)
        // If note is 12 (C0 in many standards), offset is 1 - 1 = 0.
        // If note is 24 (C0 in some standards), offset is 1 - 2 = -1.
        int offset = 1 - (capturedNote / 12);
        config->setMidiOctaveOffset(offset);
    }
}

void DuqAudioProcessor::timerCallback()
{
    // Drain any host-driven undo/redo request on the message thread.
    if (int req = pendingHostUndoRedo.exchange(0); req != 0)
        performUndoRedo(req < 0);

    auto envelopes = parameters.state.getChildWithName("ENVELOPES");
    for (int i = 0; i < Defaults::maxEnvelopeSlots; ++i)
    {
        auto& pending = pendingEnvEdits[static_cast<size_t>(i)];
        if (!pending.dirty.load(std::memory_order_acquire))
            continue;

        if (!envelopes.isValid() || i >= envelopes.getNumChildren())
            continue;

        if (!pending.dirty.exchange(false, std::memory_order_acq_rel))
            continue;

        auto env = envelopes.getChild(i);

        if (pending.rateDirty.exchange(false, std::memory_order_acq_rel))
            env.setProperty("rate", (double)pending.rate.load(std::memory_order_relaxed), nullptr);

        if (pending.depthDirty.exchange(false, std::memory_order_acq_rel))
            env.setProperty("depth", (double)pending.depth.load(std::memory_order_relaxed), nullptr);

        if (pending.smoothDirty.exchange(false, std::memory_order_acq_rel))
            env.setProperty("smooth", (double)pending.smooth.load(std::memory_order_relaxed), nullptr);
    }

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

        // Priority: Use automated parameters for the automated envelope slots
        if (i < Defaults::maxEnvelopeSlots)
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
        
        // rawSmooth is now 0-500 ms
        float smoothTimeSec = rawSmooth / 1000.0f; 
        de.smooth = rawSmooth / 500.0f; // Normalized 0..1 for UI/Internal consistency

        double srate = getSampleRate();
        if (srate <= 0) srate = 44100.0; // Fallback

        if (smoothTimeSec > 0.0001f)
            de.smoothCoeff = 1.0f - std::exp(-1.0f / (smoothTimeSec * (float)srate));
        else
            de.smoothCoeff = 1.0f;

        de.triggerNote = envVT.getProperty("triggerNote", 60);
        de.isFrequencyMode = (bool)envVT.getProperty("rateIsFrequencyMode", true);
        de.isDisabled = (bool)envVT.getProperty("disabled", false);

        double currentRate = de.rate;
        if (!de.isFrequencyMode)
            currentRate = de.rate * 2.0; // Default 120 bpm when transport is unavailable

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
                    de.segments.push_back({ 0.5f, CurveType::Exponential });
                }
            }
        }

        // Precompute the steady-state smoothed shape so the audio traces the same
        // "smoothness line" the UI draws. Empty when smoothing is off.
        if (rawSmooth > 0.0f && de.points.size() >= 2)
        {
            const DSPEnvelope& shapeEnv = de;
            auto evaluateY = [&shapeEnv](float x) -> float {
                size_t idx = 0;
                return EnvelopeEvaluator::evaluate(shapeEnv, (double)x, idx);
            };
            double effectiveRate = EnvelopeSmoothing::effectiveCyclesPerSecond(rawRate, de.isFrequencyMode);
            EnvelopeSmoothing::computeSteadyStateSmoothing(evaluateY, rawSmooth, effectiveRate, de.smoothedShape);
        }

        newEnvelopes.push_back(std::move(de));
    }

    // Build the rest of the new state locally before taking the lock.
    float newMixPercent = parameters.getRawParameterValue("mix")->load();
    float lookaheadMs = parameters.getRawParameterValue("lookahead")->load();

    double srate = getSampleRate();
    if (srate <= 0.0)
        srate = 44100.0;

    float newLookaheadSmoothCoeff;
    constexpr float lookaheadSmoothingTimeSeconds = 0.005f;
    if (lookaheadSmoothingTimeSeconds > 0.0f)
        newLookaheadSmoothCoeff = 1.0f - std::exp(-1.0f / (lookaheadSmoothingTimeSeconds * (float)srate));
    else
        newLookaheadSmoothCoeff = 0.005f;

    int targetLookaheadSamples = (int)(lookaheadMs * srate / 1000.0);

    int oldLookaheadSamples;
    {
        const juce::SpinLock::ScopedLockType sl(dspLock);
        std::swap(dspState.envelopes, newEnvelopes);

        oldLookaheadSamples = dspState.lookaheadSamples;
        dspState.mixPercent = newMixPercent;
        dspState.lookaheadSmoothCoeff = newLookaheadSmoothCoeff;
        dspState.lookaheadSamples = targetLookaheadSamples;

        // Initialize smoothing on first run or if it's way off; otherwise preserve
        // the existing smoothing state across syncs.
        if (dspState.currentLookaheadSamples < 0.0f)
            dspState.currentLookaheadSamples = (float)dspState.lookaheadSamples;
    }
    // newEnvelopes (the old envelope vector) is destroyed here, outside the lock.

    if (oldLookaheadSamples != targetLookaheadSamples)
    {
        setLatencySamples(targetLookaheadSamples);
        updateHostDisplay();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DuqAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::SharedResourcePointer<ConfigManager> config;
    auto controls = config->getDefaultControls();

    // --- Global Parameters ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mix", 1 }, "Mix", 0.0f, 100.0f, 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lookahead", 1 },
        "Lookahead",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::lookahead,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // --- Envelope Parameters (automated slots) ---
    for (int i = 0; i < Defaults::maxEnvelopeSlots; ++i)
    {
        juce::String prefix = "env" + juce::String(i) + "_";
        
        // Rate range is 0-100 (handles both Hz and Sync index)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "rate", 1 }, "Env " + juce::String(i + 1) + " Rate", 0.0f, 100.0f, (float)controls.rate));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "depth", 1 }, "Env " + juce::String(i + 1) + " Depth", 0.0f, 100.0f, controls.depth));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ prefix + "smooth", 1 },
            "Env " + juce::String(i + 1) + " Smooth",
            juce::NormalisableRange<float>(0.0f, 500.0f, 0.1f),
            controls.smooth,
            juce::AudioParameterFloatAttributes()
                .withLabel("ms")
                .withStringFromValueFunction([](float value, int) { return juce::String(value, 1); })
                .withValueFromStringFunction([](const juce::String& text) { return text.getFloatValue(); })));
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
    int newIndex = envelopes.getNumChildren();

    auto env = createDefaultEnvelope(name, note);

    envelopes.addChild(env, -1, &undoManager);

    // If it's an automated slot, update the parameters to match the defaults we just set in 'env'
    if (newIndex < Defaults::maxEnvelopeSlots)
    {
        juce::String prefix = "env" + juce::String(newIndex) + "_";
        
        auto setParam = [this, prefix](const juce::String& suffix, float value) {
            if (auto* param = parameters.getParameter(prefix + suffix))
                param->setValueNotifyingHost(parameters.getParameterRange(prefix + suffix).convertTo0to1(value));
        };
        
        setParam("rate", (float)(double)env.getProperty("rate"));
        setParam("depth", (float)(double)env.getProperty("depth"));
        setParam("smooth", (float)(double)env.getProperty("smooth"));
    }
}

// Seed a freshly triggered voice's gain state so it immediately holds the
// envelope's start value. Without this, a voice activated between control-rate
// boundaries would apply a stale gain until the next boundary, breaking the
// smooth onset.
static void initVoiceGain(EnvelopeVoice& v, const DSPEnvelope& env)
{
    // Start on the smoothness line (its steady-state phase-0 value) when smoothing
    // is active, otherwise the raw envelope start. Keeps a fresh trigger consistent
    // with what the UI draws.
    float startVal;
    if (!env.smoothedShape.empty())
    {
        startVal = env.smoothedShape.front();
    }
    else
    {
        size_t dummyIndex = 0;
        startVal = EnvelopeEvaluator::evaluate(env, 0.0, dummyIndex);
    }
    v.currentGain = 1.0f - (1.0f - (startVal * startVal)) * env.depth;
    v.targetGain = v.currentGain;
    v.gainDelta = 0.0f;
    v.lastSegmentIndex = 0;
}

void DuqAudioProcessor::handleMidiEvent(const juce::MidiMessage& msg)
{
    // Called with dspLock already held.
    if (msg.isNoteOn())
    {
        int note = msg.getNoteNumber();
        activeNotes[static_cast<size_t>(note)].store(true, std::memory_order_relaxed);

        // MIDI Calibration
        if (calibrationMode.load())
        {
            capturedCalibrationNote.store(note);
            calibrationMode.store(false);
            triggerAsyncUpdate();
            return; // Don't trigger envelopes while calibrating
        }

        // Find envelopes that match this note
        for (size_t i = 0; i < dspState.envelopes.size(); ++i)
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
                        v.isActive = true;
                        v.currentPhase = 0.0;
                        initVoiceGain(v, env);

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
                            v.envelopeIndex = static_cast<int>(i);
                            v.currentPhase = 0.0;
                            v.noteNumber = note;
                            v.isActive = true;
                            initVoiceGain(v, env);

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
        activeNotes[static_cast<size_t>(note)].store(false, std::memory_order_relaxed);

        // In ONE-SHOT mode, we do NOT deactivate voices on Note Off.
        // They finish when currentPhase >= 1.0.
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        for (auto& n : activeNotes)
            n.store(false, std::memory_order_relaxed);

        for (auto& v : voices)
        {
            v.isActive = false;
            v.envelopeIndex = -1;
        }
    }
    else if (msg.isResetAllControllers())
    {
        // Conservative: just clear the note-held state, leave voices to finish naturally.
        for (auto& n : activeNotes)
            n.store(false, std::memory_order_relaxed);
    }
}

void DuqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numInputChannels = getBusCount(true) > 0 ? getBusBuffer(buffer, true, 0).getNumChannels() : 0;
    const int numOutputChannels = getBusCount(false) > 0 ? getBusBuffer(buffer, false, 0).getNumChannels() : 0;
    const int numChannels = std::min(numInputChannels, numOutputChannels);
    
    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    float maxReduction = 0.0f;

    // --- Get write position ---
    int writeIndex = monpos.load(std::memory_order_relaxed);

    const juce::SpinLock::ScopedLockType sl(dspLock);

    double srate = getSampleRate();
    if (srate <= 0.0)
        srate = 44100.0;

    double bpm = 120.0;
    if (auto* playhead = getPlayHead())
    {
        if (auto pos = playhead->getPosition())
        {
            if (auto b = pos->getBpm())
                bpm = *b;
        }
    }

    for (auto& env : dspState.envelopes)
    {
        if (!env.isFrequencyMode)
            env.phaseIncrement = env.rate * (bpm / 60.0) / srate;
    }

    // --- Handle Manual Trigger ---
    int mTrig = manualTriggerIndex.exchange(-1);
    if (mTrig >= 0 && mTrig < (int)dspState.envelopes.size())
    {
        for (auto& v : voices)
        {
            if (!v.isActive)
            {
                const auto& env = dspState.envelopes[mTrig];
                v.envelopeIndex = mTrig;
                v.currentPhase = 0.0;
                v.noteNumber = -1; // Manual
                initVoiceGain(v, env);

                v.isActive = true;
                break;
            }
        }
    }

    const int delaySize = delayBuffer.getNumSamples();
    constexpr int controlRate = 32;

    // Sample-accurate MIDI: dispatch each event exactly at its sample position
    // (this scope already holds dspLock, matching the previous processMidi() contract).
    auto midiIt = midi.cbegin();
    const auto midiEndIt = midi.cend();

    for (int i = 0; i < numSamples; ++i)
    {
        // --- Handle MIDI events due at or before this sample ---
        while (midiIt != midiEndIt && (*midiIt).samplePosition <= i)
        {
            handleMidiEvent((*midiIt).getMessage());
            ++midiIt;
        }

        // --- Control Rate Update ---
        if (i % controlRate == 0)
        {
            const int samplesToNextControl = std::min(controlRate, numSamples - i);
            const float invSamples = 1.0f / (float)samplesToNextControl;

            for (auto& v : voices)
            {
                if (v.isActive && v.envelopeIndex >= 0 && v.envelopeIndex < (int)dspState.envelopes.size())
                {
                    const auto& env = dspState.envelopes[v.envelopeIndex];

                    // When smoothing is active, sample the precomputed steady-state
                    // shape (the same curve the UI draws) by phase; otherwise follow
                    // the raw envelope. Smoothing lives in the shape, so the gain
                    // just tracks it (the per-block gainDelta ramp de-zippers).
                    float envVal = env.smoothedShape.empty()
                        ? EnvelopeEvaluator::evaluate(env, v.currentPhase, v.lastSegmentIndex)
                        : EnvelopeSmoothing::sampleShape(env.smoothedShape, v.currentPhase);
                    float mappedVal = envVal * envVal;
                    float targetVoiceGain = 1.0f - (1.0f - mappedVal) * env.depth;

                    v.targetGain = targetVoiceGain;
                    v.gainDelta = (v.targetGain - v.currentGain) * invSamples;
                }
            }
        }

        // --- Write to Delay Buffer ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayBuffer.setSample(ch, delayWritePos, buffer.getReadPointer(ch)[i]);
        }

        // --- Read from Delay Buffer (Lookahead) ---
        // Smoothly approach target lookahead
        dspState.currentLookaheadSamples += ((float)dspState.lookaheadSamples - dspState.currentLookaheadSamples) * dspState.lookaheadSmoothCoeff;

        int readPos = (delayWritePos - (int)dspState.currentLookaheadSamples + delaySize) % delaySize;
        float sampleGain = 1.0f;

        for (auto& v : voices)
        {
            if (v.isActive && v.envelopeIndex >= 0 && v.envelopeIndex < (int)dspState.envelopes.size())
            {
                const auto& env = dspState.envelopes[v.envelopeIndex];
                
                v.currentGain += v.gainDelta;
                sampleGain *= juce::jlimit(0.0f, 1.0f, v.currentGain);

                // Advance phase
                v.currentPhase += env.phaseIncrement;

                if (v.currentPhase >= 1.0)
                {
                    if (env.isFrequencyMode)
                    {
                        v.currentPhase = std::fmod(v.currentPhase, 1.0);

                        // If note is released (or it was a manual trigger), stop at end of cycle
                        bool isNoteHeld = (v.noteNumber >= 0 && activeNotes[static_cast<size_t>(v.noteNumber)].load(std::memory_order_relaxed));

                        if (!isNoteHeld)
                        {
                            v.isActive = false;
                            v.currentPhase = 1.0;
                        }
                    }
                    else
                    {
                        v.currentPhase = 1.0;
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
        if (numChannels > 0)
        {
            for (int ch = 0; ch < numChannels; ++ch)
                monitorSample += delayBuffer.getSample(ch, readPos);
            monitorSample /= (float)numChannels;
        }

        monSamplesPre[writeIndex] = monitorSample;

        float finalMonitorSample = 0.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            float s_orig = delayBuffer.getSample(ch, readPos);
            
            // Peak input (before modulation)
            if (ch == 0) inputPeak = std::max(inputPeak, std::abs(s_orig));

            float s_wet = s_orig * currentSmoothedGain;
            float s_final = s_wet * mixVal + s_orig * (1.0f - mixVal);
            
            channelData[i] = s_final;
            finalMonitorSample += s_final;
            
            // Peak output
            outputPeak = std::max(outputPeak, std::abs(s_final));
        }

        if (numChannels > 0)
            finalMonitorSample /= (float)numChannels;
        
        monSamplesPost[writeIndex] = finalMonitorSample;

        // Sidechain input
        float sidechainSample = 0.0f;
        if (getBusCount(true) > 1)
        {
            auto scBus = getBusBuffer(buffer, true, 1);
            int scChannels = scBus.getNumChannels();
            if (scChannels > 0)
            {
                for (int ch = 0; ch < scChannels; ++ch)
                    sidechainSample += scBus.getReadPointer(ch)[i];
                sidechainSample /= (float)scChannels;
            }
        }
        monSamplesSidechain[writeIndex] = sidechainSample;

        writeIndex++;
        if (writeIndex >= monitorBufferSize)
            writeIndex = 0;

        delayWritePos++;
        if (delayWritePos >= delaySize)
            delayWritePos = 0;
    }

    // Drain any trailing events (e.g. samplePosition == numSamples edge case).
    while (midiIt != midiEndIt)
    {
        handleMidiEvent((*midiIt).getMessage());
        ++midiIt;
    }

    monpos.store(writeIndex, std::memory_order_relaxed);

    inputMeterLevel.store(inputPeak, std::memory_order_relaxed);
    outputMeterLevel.store(outputPeak, std::memory_order_relaxed);

    // For reduction, just show the instantaneous state at the end of the block
    reductionMeterLevel.store(1.0f - masterGain.getCurrentValue(), std::memory_order_relaxed);
}

juce::UndoManager& DuqAudioProcessor::getUndoManager()
{
    return undoManager;
}

std::vector<double> DuqAudioProcessor::getActivePhasesForEnvelope(int envelopeIndex) const
{
    std::vector<double> phases;
    const juce::SpinLock::ScopedTryLockType sl(dspLock);
    if (!sl.isLocked())
        return phases;

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
    const juce::SpinLock::ScopedTryLockType sl(dspLock);
    if (!sl.isLocked())
        return 0.0;

    if (envelopeIndex < 0 || envelopeIndex >= (int)dspState.envelopes.size())
        return 0.0;

    return dspState.envelopes[envelopeIndex].phaseIncrement;
}

void DuqAudioProcessor::resetVoices()
{
    const juce::SpinLock::ScopedLockType sl(dspLock);
    for (auto& v : voices)
    {
        v.isActive = false;
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

void DuqAudioProcessor::setCurrentProgram ([[maybe_unused]] int index)
{
}

const juce::String DuqAudioProcessor::getProgramName ([[maybe_unused]] int index)
{
    return {};
}

void DuqAudioProcessor::changeProgramName ([[maybe_unused]] int index, [[maybe_unused]] const juce::String& newName)
{
}

//==============================================================================
void DuqAudioProcessor::prepareToPlay (double sampleRate, [[maybe_unused]] int samplesPerBlock)
{
    masterGain.reset(sampleRate, 0.001); // 1ms smoothing for punchy transients
    masterGain.setCurrentAndTargetValue(1.0f);

    for (auto& n : activeNotes)
        n.store(false, std::memory_order_relaxed);
    resetVoices();

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
        getEnvelopesTree().removeListener(this);

        parameters.replaceState(juce::ValueTree::fromXml(*xml));

        parameters.state.addListener(this);
        getEnvelopesTree().addListener(this);

        syncToDSP();
        resetVoices();
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

        if (envIndex >= 0 && envIndex < Defaults::maxEnvelopeSlots)
        {
            juce::String propName = i.toString();
            if (propName == "rate" || propName == "depth" || propName == "smooth")
            {
                juce::String paramID = "env" + juce::String(envIndex) + "_" + propName;
                if (auto* param = parameters.getParameter(paramID))
                {
                    float newValue = (float)(double)v.getProperty(i);
                    float targetValue = parameters.getParameterRange(paramID).convertTo0to1(newValue);

                    if (std::abs(param->getValue() - targetValue) > 0.0001f)
                        param->setValueNotifyingHost(targetValue);
                }
            }
        }
    }
}

void DuqAudioProcessor::valueTreeChildOrderChanged(juce::ValueTree& v, int, int)
{
    requiresSync = true;
    resetVoices();

    if (v == getEnvelopesTree())
        rebindSlotParametersFromTree();
}

void DuqAudioProcessor::rebindSlotParametersFromTree()
{
    // After a reorder, the ValueTree order has changed but the automated slot
    // parameters (env{i}_rate/depth/smooth) are still bound to their old index.
    // Re-push each slot's parameter values from the (now reordered) tree so the
    // knobs follow the envelope rather than the slot.
    auto envelopes = getEnvelopesTree();

    isRebindingSlots = true;

    for (int i = 0; i < Defaults::maxEnvelopeSlots && i < envelopes.getNumChildren(); ++i)
    {
        auto env = envelopes.getChild(i);
        juce::String prefix = "env" + juce::String(i) + "_";

        auto pushParam = [this, &env, &prefix](const juce::String& suffix, double defaultValue)
        {
            if (auto* param = parameters.getParameter(prefix + suffix))
            {
                float value = (float)(double)env.getProperty(suffix, defaultValue);
                float target = parameters.getParameterRange(prefix + suffix).convertTo0to1(value);
                param->setValueNotifyingHost(target);
            }
        };

        pushParam("rate", 2.0);
        pushParam("depth", 100.0);
        pushParam("smooth", 0.0);
    }

    isRebindingSlots = false;
}

//==============================================================================
void DuqAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    requiresSync = true;

    if (isRebindingSlots)
        return;

    // Sync automated parameters back into the ValueTree for UI consistency
    if (parameterID.startsWith("env"))
    {
        int underscorePos = parameterID.indexOf("_");
        if (underscorePos > 3)
        {
            int envIndex = parameterID.substring(3, underscorePos).getIntValue();
            juce::String propName = parameterID.substring(underscorePos + 1);

            if (envIndex >= 0 && envIndex < Defaults::maxEnvelopeSlots)
            {
                auto& pending = pendingEnvEdits[static_cast<size_t>(envIndex)];
                if (propName == "rate")
                {
                    pending.rate.store(newValue, std::memory_order_relaxed);
                    pending.rateDirty.store(true, std::memory_order_release);
                }
                else if (propName == "depth")
                {
                    pending.depth.store(newValue, std::memory_order_relaxed);
                    pending.depthDirty.store(true, std::memory_order_release);
                }
                else if (propName == "smooth")
                {
                    pending.smooth.store(newValue, std::memory_order_relaxed);
                    pending.smoothDirty.store(true, std::memory_order_release);
                }
                pending.dirty.store(true, std::memory_order_release);
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
