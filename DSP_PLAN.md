# Plan: DSP Implementation for DUQ

This plan outlines the implementation of sample-accurate envelope modulation triggered by MIDI.

## Phase 1: DSP Data Structures & Thread Safety
- [ ] Define `DSPPoint` and `DSPSegment` (POD structs) in a new `src/dsp/EnvelopeProcessor.h`.
- [ ] Define `DSPEnvelope`:
    - `std::vector<DSPPoint> points`
    - `std::vector<DSPSegment> segments`
    - `double rate`, `float depth`, `float smooth`
    - `int triggerNote`, `bool isFrequencyMode`
    - `bool isDisabled`
- [ ] Implement a `DSPState` container in `DuqAudioProcessor` that holds a `std::vector<DSPEnvelope>`.
- [ ] Create a `syncToDSP()` method to update the `DSPState` from the `ValueTree` safely using a `juce::CriticalSection` or a lock-free swap.

## Phase 2: Envelope Evaluator
- [ ] Implement `float evaluateEnvelope(const DSPEnvelope& env, double phase)`:
    - Efficiently find the segment for the current phase (use a cached index).
    - Implement `applyCurve` logic for all `CurveType`s.
    - Output 0.0 to 1.0.

## Phase 3: Trigger & Playback Logic
- [ ] Create `EnvelopeVoice` struct:
    - `int envelopeIndex`
    - `double currentPhase`
    - `double phaseDelta`
    - `bool isActive`
- [ ] Implement `processMidi()` to handle Note On/Off:
    - Match `triggerNote` to `DSPEnvelope`.
    - Start new `EnvelopeVoice` on Note On.
    - Handle retriggering (Restart vs. Legato).

## Phase 4: Audio Processing Loop
- [ ] In `processBlock()`:
    - Calculate `phaseDelta` based on `sampleRate` and `env.rate`.
    - For each sample:
        - Advance phases of all active `EnvelopeVoice`s.
        - Calculate aggregate gain (Multiply gains of all active voices).
        - Apply `depth` (Gain = 1.0 - (1.0 - rawGain) * depth).
        - Apply `smooth` (Slew rate limiter or `juce::LinearSmoothedValue`).
        - Multiply audio samples by the final gain.
- [ ] Update `reductionMeterLevel` and `outputMeterLevel`.

## Phase 5: Host Synchronization
- [ ] Use `juce::AudioPlayHead` to get BPM and time signature.
- [ ] Implement Beat-to-Seconds conversion for sync mode.
- [ ] Ensure phase remains consistent when host transport starts/stops.

## Phase 6: Verification
- [ ] Verify MIDI triggering starts the modulation.
- [ ] Verify `depth` and `smooth` parameters work as expected.
- [ ] Test performance with multiple active envelopes.
- [ ] Verify visual feedback in the monitor buffer matches the audible effect.
