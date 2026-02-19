#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PresetManager
{
public:
    // --- File Extensions ---
    static const juce::String envelopeExtension;
    static const juce::String projectExtension;

    // --- Directory Helpers ---
    static juce::File getEnvelopeDirectory();
    static juce::File getProjectDirectory();

    // --- Saving ---
    static bool saveEnvelope(const juce::ValueTree& envelope, const juce::File& file);
    static bool saveProject(const juce::ValueTree& state, const juce::File& file);

    // --- Loading ---
    static juce::ValueTree loadEnvelope(const juce::File& file);
    static juce::ValueTree loadProject(const juce::File& file);

    static void importEnvelope (juce::ValueTree& parent, juce::UndoManager* undoManager, std::function<void(int)> onComplete);

    static void showCorruptPresetAlert();

private:
    static bool saveValueTreeToXml(const juce::ValueTree& vt, const juce::File& file);
    static juce::ValueTree loadValueTreeFromXml(const juce::File& file, const juce::Identifier& expectedType);
};
