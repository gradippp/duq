#include "PresetSection.h"
#include "../utils/IconFactory.h"
#include "../../utils/PresetManager.h"
#include "../utils/FontManager.h"
#include "../../PluginProcessor.h"
#include "../../Globals.h"

PresetSection::PresetSection(DuqAudioProcessor& p)
    : processor(p)
{
    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(FontManager::getBarlowBold(22.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));

    auto normal = Icons::load("close", juce::Colours::white.withAlpha(0.6f));
    auto over = Icons::load("close", juce::Colours::white);
    auto down = Icons::load("close", juce::Colours::white.withAlpha(0.4f));
    closeButton.setImages(normal.get(), over.get(), down.get());
    addAndMakeVisible(closeButton);
    closeButton.onClick = [this]() { if (onClose) onClose(); };

    // Search Bar
    addAndMakeVisible(searchEditor);
    searchEditor.setTextToShowWhenEmpty("SEARCH PRESETS...", juce::Colours::white.withAlpha(0.3f));
    searchEditor.setJustification(juce::Justification::centred);
    searchEditor.setFont(FontManager::getInterRegular(14.0f));
    searchEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha(0.2f));
    searchEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha(0.1f));
    searchEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::white.withAlpha(0.3f));
    searchEditor.onTextChange = [this]() { searchText = searchEditor.getText(); filterPresets(); };

    // List
    addAndMakeVisible(presetList);
    presetList.setModel(this);
    presetList.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetList.setRowHeight(36);

    setOpaque(true);
}

PresetSection::~PresetSection() {}

void PresetSection::setMode(Mode newMode)
{
    mode = newMode;
    titleLabel.setText(mode == Mode::Project ? "PROJECT BROWSER" : "ENVELOPE BROWSER", juce::dontSendNotification);
    searchEditor.setText("", juce::dontSendNotification);
    refreshPresetList();
}

void PresetSection::setTargetEnvelope(juce::ValueTree envelope)
{
    targetEnvelope = envelope;
    refreshPresetList();
}

void PresetSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}

void PresetSection::refreshPresetList()
{
    allFiles.clear();
    auto dir = (mode == Mode::Project) ? PresetManager::getProjectDirectory() : PresetManager::getEnvelopeDirectory();
    auto extension = (mode == Mode::Project) ? PresetManager::projectExtension : PresetManager::envelopeExtension;
    
    if (dir.exists() && dir.isDirectory()) {
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*" + extension);
        for (auto f : files)
            allFiles.push_back(f);
    }
    filterPresets();
}

void PresetSection::filterPresets()
{
    filteredFiles.clear();
    if (searchText.isEmpty()) {
        filteredFiles = allFiles;
    } else {
        for (const auto& f : allFiles) {
            if (f.getFileNameWithoutExtension().containsIgnoreCase(searchText))
                filteredFiles.push_back(f);
        }
    }
    presetList.updateContent();
}

void PresetSection::deletePreset(int index)
{
    if (index < 0 || index >= (int)filteredFiles.size()) return;
    
    auto file = filteredFiles[index];
    if (file.deleteFile()) {
        refreshPresetList();
    }
}

int PresetSection::getNumRows() { return (int)filteredFiles.size(); }

void PresetSection::paintListBoxItem(int, juce::Graphics&, int, int, bool) {}

juce::Component* PresetSection::refreshComponentForRow(int rowNumber, bool isSelected, juce::Component* existingComponentToUpdate)
{
    auto* row = static_cast<PresetRowComponent*>(existingComponentToUpdate);
    if (row == nullptr) row = new PresetRowComponent(*this, rowNumber);
    row->update(rowNumber, isSelected);
    return row;
}

void PresetSection::listBoxItemClicked(int rowNumber, const juce::MouseEvent&)
{
    if (rowNumber < 0 || rowNumber >= (int)filteredFiles.size()) return;
    auto file = filteredFiles[rowNumber];

    if (mode == Mode::Envelope) {
        if (!targetEnvelope.isValid()) return;
        auto loaded = PresetManager::loadEnvelope(file);
        if (loaded.isValid()) {
            if (undoManager) undoManager->beginNewTransaction("Load Envelope: " + file.getFileNameWithoutExtension());
            targetEnvelope.copyPropertiesAndChildrenFrom(loaded, undoManager);
            if (onClose) onClose();
        }
    } else if (mode == Mode::Import) {
        auto loaded = PresetManager::loadEnvelope(file);
        if (loaded.isValid()) {
            auto envelopesTree = processor.getEnvelopesTree();
            if (undoManager) undoManager->beginNewTransaction("Import Envelope: " + file.getFileNameWithoutExtension());
            envelopesTree.addChild(loaded, -1, undoManager);
            if (onEnvelopeImported) onEnvelopeImported(envelopesTree.getNumChildren() - 1);
            if (onClose) onClose();
        }
    } else { // Project
        auto loaded = PresetManager::loadProject(file);
        if (loaded.isValid()) {
            if (undoManager) undoManager->beginNewTransaction("Load Project: " + file.getFileNameWithoutExtension());
            processor.getEnvelopesTree().copyPropertiesAndChildrenFrom(loaded, undoManager);
            if (onProjectLoaded) onProjectLoaded(file.getFileNameWithoutExtension());
            if (onClose) onClose();
        }
    }
}

void PresetSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Modern dark background
    g.setColour(juce::Colour(0xff121212));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle inner shadow / border
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);

    // Footer area
    auto footerArea = getLocalBounds().removeFromBottom(32).toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(footerArea.reduced(2.0f), 2.0f);
    
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(footerArea.getX() + 10, footerArea.getY(), footerArea.getRight() - 10, footerArea.getY());

    auto dir = (mode == Mode::Project) 
        ? PresetManager::getProjectDirectory() 
        : PresetManager::getEnvelopeDirectory();

    g.setColour(Theme::Colours::textDimmed);
    g.setFont(FontManager::getJetBrainsMono(10.0f));
    g.drawText("Presets location: " + dir.getFullPathName(), footerArea.reduced(15, 0), juce::Justification::centredLeft);

    if (filteredFiles.empty())
    {
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        g.setFont(FontManager::getBarlowBold(18.0f));
        g.drawFittedText("NO PRESETS FOUND", getLocalBounds().withTrimmedBottom(32), juce::Justification::centred, 1);
    }
}

void PresetSection::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(60);
    closeButton.setBounds(header.removeFromRight(50).withSizeKeepingCentre(24, 24));
    titleLabel.setBounds(header.withLeft(50));

    auto searchArea = area.removeFromTop(40).reduced(20, 5);
    searchEditor.setBounds(searchArea);

    area.removeFromBottom(32); // Footer height
    presetList.setBounds(area.reduced(20, 5));
}

// ===== PresetRowComponent Implementation =====

PresetSection::PresetRowComponent::PresetRowComponent(PresetSection& o, int idx) : owner(o), rowDataIndex(idx)
{
    auto delIcon = Icons::load("delete", juce::Colours::white.withAlpha(0.4f));
    auto delIconOver = Icons::load("delete", Theme::Colours::danger);
    deleteButton.setImages(delIcon.get(), delIconOver.get(), delIcon.get());
    addAndMakeVisible(deleteButton);
    deleteButton.onClick = [this]() { owner.deletePreset(rowDataIndex); };
}

void PresetSection::PresetRowComponent::update(int idx, bool sel)
{
    rowDataIndex = idx;
    isSelected = sel;
    repaint();
}

void PresetSection::PresetRowComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    if (isSelected) {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(area.reduced(4, 2), 4.0f);
    } else if (isHovering) {
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        g.fillRoundedRectangle(area.reduced(4, 2), 4.0f);
    }

    if (rowDataIndex < (int)owner.filteredFiles.size()) {
        auto name = owner.filteredFiles[rowDataIndex].getFileNameWithoutExtension();
        g.setColour(isSelected ? Theme::Colours::textMain : Theme::Colours::textDimmed);
        g.setFont(FontManager::getInterRegular(14.0f));
        g.drawText(name, area.reduced(15, 0), juce::Justification::centredLeft, true);
    }
}

void PresetSection::PresetRowComponent::resized()
{
    deleteButton.setBounds(getWidth() - 40, (getHeight() - 20) / 2, 20, 20);
}

void PresetSection::PresetRowComponent::mouseDown(const juce::MouseEvent& e)
{
    owner.presetList.selectRow(rowDataIndex);
    owner.listBoxItemClicked(rowDataIndex, e);
}
