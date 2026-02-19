#include <memory>
#include "EnvelopeListSection.h"
#include "../../PluginProcessor.h"
#include "../utils/FontManager.h"
#include "../../model/EnvelopeData.h"
#include "../../Globals.h"
#include "../utils/PresetManager.h"

juce::Font EnvelopeListSection::CustomButtonLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return FontManager::getBarlowBold(12.0f);
}

EnvelopeListSection::EnvelopeListSection()
{
    auto setupButton = [this](juce::TextButton& button, const juce::String& text, const juce::String& tooltip)
        {
            button.setLookAndFeel(&buttonLnf);
            button.setButtonText(text);
            button.setTooltip(tooltip);
            button.setColour(juce::TextButton::buttonColourId, Theme::Colours::background.withAlpha(0.4f));
            button.setColour(juce::TextButton::buttonOnColourId, Theme::Colours::uiHover);
            button.setColour(juce::TextButton::textColourOffId, Theme::Colours::textLabel);
            button.setColour(juce::TextButton::textColourOnId, Theme::Colours::textMain);
        };

    setupButton(addButton, "+ ADD", "Add a new default envelope");
    setupButton(importButton, "IMPORT", "Import an envelope preset (.duq.env)");

    addAndMakeVisible(addButton);
    addAndMakeVisible(importButton);

    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    addButton.onClick = [this]()
        {
            if (!undoManager || !envelopesTree.isValid())
                return;

            undoManager->beginNewTransaction("Add Envelope");

            juce::ValueTree env("ENVELOPE");
            env.setProperty("name", generateDefaultName(), nullptr);
            env.setProperty("triggerNote", getNextFreeNote(Theme::Defaults::triggerNote), nullptr);
            env.setProperty("rate", Theme::Defaults::rate, nullptr);
            env.setProperty("depth", (double)Theme::Defaults::depth, nullptr);
            env.setProperty("smooth", (double)Theme::Defaults::smooth, nullptr);
            env.setProperty("rateIsFrequencyMode", Theme::Defaults::rateIsFrequencyMode, nullptr);

            juce::ValueTree points("POINTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints; ++i)
            {
                juce::ValueTree p("POINT");
                p.setProperty("x", Theme::Defaults::defaultPoints[i].x, nullptr);
                p.setProperty("y", Theme::Defaults::defaultPoints[i].y, nullptr);
                points.addChild(p, -1, nullptr);
            }

            juce::ValueTree segments("SEGMENTS");
            for (int i = 0; i < Theme::Defaults::numDefaultPoints - 1; ++i)
            {
                juce::ValueTree s("SEGMENT");
                s.setProperty("curve", Theme::Defaults::curve, nullptr);
                s.setProperty("type", Theme::Defaults::curveType, nullptr);
                segments.addChild(s, -1, nullptr);
            }

            env.addChild(points, -1, nullptr);
            env.addChild(segments, -1, nullptr);

            const int newIndex = envelopesTree.getNumChildren();
            envelopesTree.addChild(env, -1, undoManager);
            selectEnvelope(newIndex);
        };

    importButton.onClick = [this]()
        {
            if (!undoManager || !envelopesTree.isValid())
                return;

            auto fc = std::make_unique<juce::FileChooser>(
                "Import Envelope",
                PresetManager::getEnvelopeDirectory(),
                "*" + PresetManager::envelopeExtension);

            fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, fc_ptr = fc.release()](const juce::FileChooser& chooser) mutable
                {
                    std::unique_ptr<juce::FileChooser> fc(fc_ptr);
                    auto file = chooser.getResult();
                    if (file.existsAsFile())
                    {
                        auto importedEnv = PresetManager::loadEnvelope(file);
                        if (importedEnv.isValid())
                        {
                            undoManager->beginNewTransaction("Import Envelope");
                            
                            // Ensure the name is unique if needed, or keep original
                            if (importedEnv.hasProperty("name")) {
                                auto baseName = importedEnv["name"].toString();
                                int counter = 1;
                                juce::String finalName = baseName;
                                bool nameExists = true;
                                while (nameExists) {
                                    nameExists = false;
                                    for (int i = 0; i < envelopesTree.getNumChildren(); ++i) {
                                        if (envelopesTree.getChild(i)["name"].toString() == finalName) {
                                            nameExists = true;
                                            break;
                                        }
                                    }
                                    if (nameExists) {
                                        finalName = baseName + " " + juce::String(++counter);
                                    }
                                }
                                importedEnv.setProperty("name", finalName, nullptr);
                            }

                            // Ensure trigger note is free
                            importedEnv.setProperty("triggerNote", getNextFreeNote(Theme::Defaults::triggerNote), nullptr);

                            const int newIndex = envelopesTree.getNumChildren();
                            envelopesTree.addChild(importedEnv, -1, undoManager);
                            selectEnvelope(newIndex);
                        }
                        else
                        {
                            PresetManager::showCorruptPresetAlert();
                        }
                    }
                });
        };
}

EnvelopeListSection::~EnvelopeListSection()
{
    addButton.setLookAndFeel(nullptr);
    importButton.setLookAndFeel(nullptr);
}

void EnvelopeListSection::setProcessor(DuqAudioProcessor& p)
{
    processor = &p;
    envelopesTree = p.getEnvelopesTree();

    envelopesTree.addListener(this);

    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier&)
{
    if (v == envelopesTree)
        rebuildRowsFromModel();
}


void EnvelopeListSection::rebuildRowsFromModel()
{
    rowContainer.removeAllChildren();
    rows.clear();

    const int count = envelopesTree.getNumChildren();

    for (int i = 0; i < count; ++i)
    {
        auto envTree = envelopesTree.getChild(i);

        auto* row = new EnvelopeRowComponent(envTree);
        row->setUndoManager(undoManager);

        row->onDeleteRequested = [this, envTree]()
            {
                if (!undoManager || !envelopesTree.isValid())
                    return;

                int index = envelopesTree.indexOf(envTree);
                if (index < 0)
                    return;

                undoManager->beginNewTransaction("Delete Envelope");
                envelopesTree.removeChild(index, undoManager);
            };


        row->onReplaceRequested = [this, envTree]()
            {
                if (onReplaceRequested)
                    onReplaceRequested(envTree);
            };

        row->onNameChanged = [this, i](const juce::String& newName)
            {
                auto envTree = envelopesTree.getChild(i);
                envTree.setProperty("name", newName.trim(), undoManager);
            };

        row->onNoteChanged = [this, envTree](int newNote) mutable
            {
                newNote = juce::jlimit(0, 127, newNote);
                envTree.setProperty("triggerNote", newNote, undoManager);
            };

        row->onSelected = [this, i]()
            {
                selectEnvelope(i);
            };

        rows.add(row);
        rowContainer.addAndMakeVisible(row);
    }

    if (selectedIndex >= count)
        selectedIndex = count - 1;

    if (count == 0)
    {
        selectedIndex = -1;

        if (onEnvelopeSelected)
            onEnvelopeSelected(juce::ValueTree());

        resized();
        return;
    }

    if (selectedIndex < 0)
        selectedIndex = 0;

    selectEnvelope(selectedIndex);
    resized();
}


void EnvelopeListSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}

void EnvelopeListSection::selectEnvelope(int index)
{
    if (index < 0 || index >= envelopesTree.getNumChildren())
        return;

    selectedIndex = index;

    for (int i = 0; i < rows.size(); ++i)
        rows[i]->setSelected(i == index);

    if (onEnvelopeSelected)
        onEnvelopeSelected(envelopesTree.getChild(index));
}


void EnvelopeListSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(Theme::Colours::sectionBackground);

    // Outer border
    g.setColour(Theme::Colours::border);
    g.drawRect(bounds, 1);

    // ===== Header Area =====
    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(Theme::Colours::headerBackground);
    g.fillRect(headerArea);

    g.setColour(Theme::Colours::textMain);
    g.setFont(FontManager::getBarlowBold(16.0f));

    g.drawText("ENVELOPES",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);

    // Footer separator
    auto footerBounds = getLocalBounds().removeFromBottom(40);
    g.setColour(Theme::Colours::border.withAlpha(0.5f));
    g.drawLine(0.0f, footerBounds.getY(), (float)getWidth(), footerBounds.getY(), 1.0f);
}

void EnvelopeListSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr int headerHeight = 32;
    constexpr int buttonHeight = 28;
    constexpr int rowHeight = 28;

    // Remove header
    bounds.removeFromTop(headerHeight);

    // Bottom buttons
    auto footerArea = bounds.removeFromBottom(buttonHeight + 8).reduced(6, 4);
    
    addButton.setBounds(footerArea.removeFromLeft(footerArea.getWidth() / 2 - 2));
    footerArea.removeFromLeft(4);
    importButton.setBounds(footerArea);

    // Viewport takes remaining space
    viewport.setBounds(bounds);

    // ===== Layout rows inside rowContainer =====

    int totalHeight = rows.size() * rowHeight;

    rowContainer.setSize(bounds.getWidth(), totalHeight);

    auto rowBounds = rowContainer.getLocalBounds();

    for (auto* row : rows)
        row->setBounds(rowBounds.removeFromTop(rowHeight));
}

void EnvelopeListSection::setSelectedIndex(int index)
{
    selectedIndex = index;
}

int EnvelopeListSection::getSelectedIndex() const
{
    return selectedIndex;
}

int EnvelopeListSection::getEnvelopeCount() const
{
    return envelopesTree.getNumChildren();
}

bool EnvelopeListSection::isNoteAlreadyUsed(int note) const
{
    const int count = envelopesTree.getNumChildren();

    for (int i = 0; i < count; ++i)
    {
        if ((int)envelopesTree.getChild(i)["triggerNote"] == note)
            return true;
    }

    return false;
}

juce::String EnvelopeListSection::generateDefaultName() const
{
    int counter = 1;

    while (true)
    {
        juce::String candidate = Theme::Defaults::envelopeName + " " + juce::String(counter);
        bool exists = false;

        for (int i = 0; i < envelopesTree.getNumChildren(); ++i)
        {
            if (envelopesTree.getChild(i)["name"].toString() == candidate)
            {
                exists = true;
                break;
            }
        }

        if (!exists)
            return candidate;

        ++counter;
    }
}

int EnvelopeListSection::getNextFreeNote(int startFrom) const
{
    for (int note = startFrom; note <= 127; ++note)
    {
        if (!isNoteAlreadyUsed(note))
            return note;
    }

    return startFrom;
}

void EnvelopeListSection::updateMidiActivity(DuqAudioProcessor& processor)
{
    for (int i = 0; i < rows.size(); ++i)
    {
        int note = (int)envelopesTree.getChild(i)["triggerNote"];
        bool isActive = processor.isNoteActive(note);
        rows[i]->setActive(isActive);
    }
}
