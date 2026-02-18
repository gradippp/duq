#include "PresetManager.h"

const juce::String PresetManager::envelopeExtension = ".duq.env";
const juce::String PresetManager::projectExtension = ".duq";

juce::File PresetManager::getEnvelopeDirectory()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Duq")
        .getChildFile("Envelopes");

    if (!dir.exists())
        dir.createDirectory();

    return dir;
}

juce::File PresetManager::getProjectDirectory()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Duq")
        .getChildFile("Presets");

    if (!dir.exists())
        dir.createDirectory();

    return dir;
}

bool PresetManager::saveEnvelope(const juce::ValueTree& envelope, const juce::File& file)
{
    if (!envelope.isValid() || envelope.getType() != juce::Identifier("ENVELOPE"))
        return false;

    // Create a copy to strip UI-only properties
    juce::ValueTree cleanEnv = envelope.createCopy();
    
    cleanEnv.removeProperty("zoomX", nullptr);
    cleanEnv.removeProperty("zoomY", nullptr);
    cleanEnv.removeProperty("uniformZoom", nullptr);
    cleanEnv.removeProperty("offsetX", nullptr);
    cleanEnv.removeProperty("offsetY", nullptr);
    // Note: We keep 'gridPower' as it might be relevant to the rhythmic structure of the preset

    return saveValueTreeToXml(cleanEnv, file);
}

bool PresetManager::saveProject(const juce::ValueTree& state, const juce::File& file)
{
    if (!state.isValid() || state.getType() != juce::Identifier("ENVELOPES"))
        return false;

    return saveValueTreeToXml(state, file);
}

juce::ValueTree PresetManager::loadEnvelope(const juce::File& file)
{
    return loadValueTreeFromXml(file, juce::Identifier("ENVELOPE"));
}

juce::ValueTree PresetManager::loadProject(const juce::File& file)
{
    return loadValueTreeFromXml(file, juce::Identifier("ENVELOPES"));
}

bool PresetManager::saveValueTreeToXml(const juce::ValueTree& vt, const juce::File& file)
{
    std::unique_ptr<juce::XmlElement> xml = vt.createXml();
    if (xml == nullptr)
        return false;

    return xml->writeTo(file);
}

juce::ValueTree PresetManager::loadValueTreeFromXml(const juce::File& file, const juce::Identifier& expectedType)
{
    std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName(expectedType))
        return juce::ValueTree();

    return juce::ValueTree::fromXml(*xml);
}
