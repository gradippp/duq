#include "PresetManager.h"
#include "../Globals.h"

const juce::String PresetManager::envelopeExtension = "duq.env";
const juce::String PresetManager::projectExtension = "duq";

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

    // Add metadata
    juce::ValueTree metadata("METADATA");
    metadata.setProperty("plugin", "Duq", nullptr);
    metadata.setProperty("version", PROJECT_VERSION, nullptr);
    metadata.setProperty("manufacturer", "Duq", nullptr);
    cleanEnv.addChild(metadata, -1, nullptr);

    return saveValueTreeToXml(cleanEnv, file);
}

bool PresetManager::saveProject(const juce::ValueTree& state, const juce::File& file)
{
    if (!state.isValid())
        return false;

    juce::ValueTree envelopes;
    juce::ValueTree root;

    if (state.getType() == juce::Identifier("ENVELOPES"))
    {
        envelopes = state;
    }
    else
    {
        root = state;
        envelopes = state.getChildWithName("ENVELOPES");
    }

    if (!envelopes.isValid())
        return false;

    juce::ValueTree cleanProject = envelopes.createCopy();

    // Copy global properties if we have the root
    if (root.isValid())
    {
        const juce::Identifier props[] = { "mix", "lookahead", "lookbehind" };
        for (const auto& id : props)
        {
            if (root.hasProperty(id))
                cleanProject.setProperty(id, root.getProperty(id), nullptr);
        }
    }

    // Add metadata
    juce::ValueTree metadata("METADATA");
    metadata.setProperty("plugin", "Duq", nullptr);
    metadata.setProperty("version", PROJECT_VERSION, nullptr);
    metadata.setProperty("manufacturer", "Duq", nullptr);
    cleanProject.addChild(metadata, -1, nullptr);

    return saveValueTreeToXml(cleanProject, file);
}

bool PresetManager::saveEnvelopeByName(const juce::ValueTree& envelope, const juce::String& name)
{
    auto file = getEnvelopeDirectory().getChildFile(name).withFileExtension(envelopeExtension);
    return saveEnvelope(envelope, file);
}

bool PresetManager::saveProjectByName(const juce::ValueTree& state, const juce::String& name)
{
    auto file = getProjectDirectory().getChildFile(name).withFileExtension(projectExtension);
    return saveProject(state, file);
}

juce::ValueTree PresetManager::loadEnvelope(const juce::File& file)
{
    return loadValueTreeFromXml(file, juce::Identifier("ENVELOPE"));
}

juce::ValueTree PresetManager::loadProject(const juce::File& file)
{
    return loadValueTreeFromXml(file, juce::Identifier("ENVELOPES"));
}

void PresetManager::importEnvelope(juce::ValueTree& envelopesTree, juce::UndoManager* undoManager, std::function<void(int)> onComplete)
{
    if (!envelopesTree.isValid())
        return;

    auto chooser = std::make_shared<juce::FileChooser>(
        "Import Envelope",
        getEnvelopeDirectory(),
        "*." + envelopeExtension);

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [envelopesTree, undoManager, onComplete, chooser](const juce::FileChooser& fc) mutable
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                auto importedEnv = loadEnvelope(file);
                if (importedEnv.isValid())
                {
                    if (undoManager)
                        undoManager->beginNewTransaction("Import Envelope");

                    // 1. Handle name collisions
                    auto baseName = importedEnv.getProperty("name", "Imported").toString();
                    int counter = 1;
                    juce::String finalName = baseName;
                    bool nameExists = true;
                    
                    while (nameExists) 
                    {
                        nameExists = false;
                        for (int i = 0; i < envelopesTree.getNumChildren(); ++i) 
                        {
                            if (envelopesTree.getChild(i)["name"].toString() == finalName) 
                            {
                                nameExists = true;
                                break;
                            }
                        }
                        if (nameExists) 
                            finalName = baseName + " " + juce::String(++counter);
                    }
                    importedEnv.setProperty("name", finalName, nullptr);

                    // 2. Find next free trigger note
                    int nextNote = Theme::Defaults::triggerNote;
                    for (int n = Theme::Defaults::triggerNote; n <= 127; ++n) 
                    {
                        bool isNoteUsed = false;
                        for (int i = 0; i < envelopesTree.getNumChildren(); ++i) 
                        {
                            if ((int)envelopesTree.getChild(i)["triggerNote"] == n) 
                            {
                                isNoteUsed = true;
                                break;
                            }
                        }
                        if (!isNoteUsed) 
                        {
                            nextNote = n;
                            break;
                        }
                    }
                    importedEnv.setProperty("triggerNote", nextNote, nullptr);

                    const int newIndex = envelopesTree.getNumChildren();
                    envelopesTree.addChild(importedEnv, -1, undoManager);
                    
                    if (onComplete)
                        onComplete(newIndex);
                }
                else
                {
                    showCorruptPresetAlert();
                }
            }
        });
}

void PresetManager::showCorruptPresetAlert()
{
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon,
        "Preset Error",
        "This preset has been corrupt or is not supported in this version of DUQ",
        "OK");
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

    auto vt = juce::ValueTree::fromXml(*xml);

    // Metadata validation
    auto metadata = vt.getChildWithName("METADATA");
    if (!metadata.isValid() || metadata.getProperty("plugin").toString() != "Duq")
    {
        // For now, we allow legacy presets without metadata, but they must still pass deep validation.
        // If we want to strictly require metadata, we'd return juce::ValueTree() here.
    }

    // Deep validation for ENVELOPE structure
    auto validateEnvelope = [](juce::ValueTree env) -> bool
    {
        if (env.getType() != juce::Identifier("ENVELOPE"))
            return false;

        auto points = env.getChildWithName("POINTS");
        if (!points.isValid() || points.getNumChildren() < 2)
            return false;

        // Verify points have required properties
        for (int i = 0; i < points.getNumChildren(); ++i)
        {
            auto p = points.getChild(i);
            if (!p.hasProperty("x") || !p.hasProperty("y"))
                return false;
        }

        // SEGMENTS is mandatory in this version
        auto segments = env.getChildWithName("SEGMENTS");
        if (!segments.isValid() || segments.getNumChildren() != points.getNumChildren() - 1)
            return false;

        for (int i = 0; i < segments.getNumChildren(); ++i)
        {
            auto s = segments.getChild(i);
            if (!s.hasProperty("curve") || !s.hasProperty("type"))
                return false;
        }

        return true;
    };

    if (expectedType == juce::Identifier("ENVELOPE"))
    {
        if (!validateEnvelope(vt))
            return juce::ValueTree();
    }
    else if (expectedType == juce::Identifier("ENVELOPES"))
    {
        for (int i = 0; i < vt.getNumChildren(); ++i)
        {
            auto child = vt.getChild(i);
            if (child.getType() == juce::Identifier("METADATA"))
                continue;

            if (!validateEnvelope(child))
                return juce::ValueTree();
        }
    }

    return vt;
}
