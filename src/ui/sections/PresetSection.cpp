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
    titleLabel.setColour(juce::Label::textColourId, T_COL(textMain).withAlpha(0.8f));

    auto normal = Icons::load("close", T_COL(textMain).withAlpha(0.6f));
    auto over = Icons::load("close", T_COL(textMain));
    auto down = Icons::load("close", T_COL(textMain).withAlpha(0.4f));
    closeButton.setImages(normal.get(), over.get(), down.get());
    addAndMakeVisible(closeButton);
    closeButton.onClick = [this]() { if (onClose) onClose(); };

    // Search Bar
    addAndMakeVisible(searchEditor);
    searchEditor.setTextToShowWhenEmpty("SEARCH PRESETS...", T_COL(textDimmed));
    searchEditor.setJustification(juce::Justification::centred);
    searchEditor.setFont(FontManager::getInterRegular(14.0f));
    searchEditor.setColour(juce::TextEditor::backgroundColourId, T_COL(presetBrowserFooter).withAlpha(0.2f));
    searchEditor.setColour(juce::TextEditor::outlineColourId, T_COL(border).withAlpha(0.1f));
    searchEditor.setColour(juce::TextEditor::focusedOutlineColourId, T_COL(border).withAlpha(0.3f));
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
    isSavingMode = false;
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

void PresetSection::startSavingProcess()
{
    isSavingMode = true;
    searchText = "";
    searchEditor.setText("", juce::dontSendNotification);
    refreshPresetList();
    presetList.scrollToEnsureRowIsOnscreen(0);
    
    // We want the first row to be the editable one
    repaint();
}

void PresetSection::refreshPresetList()
{
    allFiles.clear();
    auto dir = (mode == Mode::Project) ? PresetManager::getProjectDirectory() : PresetManager::getEnvelopeDirectory();
    auto extension = (mode == Mode::Project) ? PresetManager::projectExtension : PresetManager::envelopeExtension;
    
    if (dir.exists() && dir.isDirectory()) {
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*." + extension);
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
    int fileIndex = isSavingMode ? index - 1 : index;
    if (fileIndex < 0 || fileIndex >= (int)filteredFiles.size()) return;
    
    auto file = filteredFiles[fileIndex];
    
    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::WarningIcon,
        "Delete Preset",
        "Are you sure you want to delete '" + file.getFileNameWithoutExtension() + "'? This action cannot be undone.",
        this,
        juce::ModalCallbackFunction::create([this, file](int result) {
            if (result != 0) {
                if (file.deleteFile())
                    refreshPresetList();
            }
        })
    );
}

int PresetSection::getNumRows() 
{ 
    int count = (int)filteredFiles.size();
    if (isSavingMode) count++;
    return count;
}

void PresetSection::paintListBoxItem(int, juce::Graphics&, int, int, bool) {}

juce::Component* PresetSection::refreshComponentForRow(int rowNumber, bool isSelected, juce::Component* existingComponentToUpdate)
{
    auto* row = static_cast<PresetRowComponent*>(existingComponentToUpdate);
    if (row == nullptr) row = new PresetRowComponent(*this, rowNumber);
    
    bool isEditable = isSavingMode && rowNumber == 0;
    int dataIdx = isSavingMode ? rowNumber - 1 : rowNumber;
    
    row->update(dataIdx, isSelected, isEditable);
    return row;
}

void PresetSection::listBoxItemClicked(int rowNumber, const juce::MouseEvent& e)
{
    if (isSavingMode && rowNumber == 0) return;
    
    int dataIdx = isSavingMode ? rowNumber - 1 : rowNumber;
    if (dataIdx < 0 || dataIdx >= (int)filteredFiles.size()) return;
    
    auto file = filteredFiles[dataIdx];

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
    g.setColour(T_COL(presetBrowserBackground));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle inner shadow / border
    g.setColour(T_COL(border).withAlpha(0.2f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);

    // Footer area
    auto footerArea = getLocalBounds().removeFromBottom(32).toFloat();
    g.setColour(T_COL(presetBrowserFooter).withAlpha(0.3f));
    g.fillRoundedRectangle(footerArea.reduced(2.0f), 2.0f);
    
    g.setColour(T_COL(presetBrowserFooterLine));
    g.drawLine(footerArea.getX() + 10, footerArea.getY(), footerArea.getRight() - 10, footerArea.getY());

    auto dir = (mode == Mode::Project) 
        ? PresetManager::getProjectDirectory() 
        : PresetManager::getEnvelopeDirectory();

    g.setColour(T_COL(textDimmed));
    g.setFont(FontManager::getJetBrainsMono(10.0f));
    g.drawText("Presets location: " + dir.getFullPathName(), footerArea.reduced(15, 0), juce::Justification::centredLeft);

    if (filteredFiles.empty() && !isSavingMode)
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
    auto delIcon = Icons::load("delete", T_COL(textMain).withAlpha(0.4f));
    auto delIconOver = Icons::load("delete", T_COL(danger));
    deleteButton.setImages(delIcon.get(), delIconOver.get(), delIcon.get());
    addAndMakeVisible(deleteButton);
    deleteButton.onClick = [this]() { owner.deletePreset(rowDataIndex); };

    addAndMakeVisible(editLabel);
    editLabel.setEditable(true, true, false);
    editLabel.setFont(FontManager::getInterRegular(14.0f));
    editLabel.setColour(juce::Label::textColourId, T_COL(textMain));
    editLabel.setColour(juce::Label::textWhenEditingColourId, T_COL(textMain));
    editLabel.setJustificationType(juce::Justification::centredLeft);

    editLabel.onEditorShow = [this]() {
        if (auto* editor = editLabel.getCurrentTextEditor())
            editor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    };

    editLabel.onEditorHide = [this]() {
        auto name = editLabel.getText().trim();
        if (name.isNotEmpty()) {
            if (owner.mode == PresetSection::Mode::Project) {
                PresetManager::saveProjectByName(owner.processor.parameters.state, name);
            } else {
                auto env = owner.targetEnvelope.isValid() ? owner.targetEnvelope : owner.processor.getEnvelopesTree().getChild(0);
                PresetManager::saveEnvelopeByName(env, name);
            }
            owner.isSavingMode = false;
            owner.refreshPresetList();
        }
    };
}

void PresetSection::PresetRowComponent::update(int idx, bool sel, bool editable)
{
    rowDataIndex = idx;
    isSelected = sel;
    isEditableMode = editable;

    if (isEditableMode) {
        editLabel.setVisible(true);
        editLabel.setText("New Preset", juce::dontSendNotification);
        editLabel.showEditor();
        deleteButton.setVisible(false);
    } else {
        editLabel.setVisible(false);
        // Don't allow deletion in Import mode
        deleteButton.setVisible(owner.mode != PresetSection::Mode::Import);
    }
    
    repaint();
}

void PresetSection::PresetRowComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    if (isSelected) {
        g.setColour(T_COL(uiSelected));
        g.fillRoundedRectangle(area.reduced(4, 2), 4.0f);
    } else if (isHovering) {
        g.setColour(T_COL(uiHover));
        g.fillRoundedRectangle(area.reduced(4, 2), 4.0f);
    }

    if (!isEditableMode && rowDataIndex >= 0 && rowDataIndex < (int)owner.filteredFiles.size()) {
        auto file = owner.filteredFiles[rowDataIndex];
        auto name = file.getFileName();
        
        auto ext = (owner.mode == PresetSection::Mode::Project) ? PresetManager::projectExtension : PresetManager::envelopeExtension;
        
        if (name.endsWith("." + ext))
            name = name.dropLastCharacters(ext.length() + 1);

        g.setColour(isSelected ? T_COL(textMain) : T_COL(textDimmed));
        g.setFont(FontManager::getInterRegular(14.0f));
        g.drawText(name, area.reduced(15, 0), juce::Justification::centredLeft, true);
    }
}

void PresetSection::PresetRowComponent::resized()
{
    auto area = getLocalBounds().reduced(15, 0);
    editLabel.setBounds(area.withTrimmedRight(40));
    deleteButton.setBounds(getWidth() - 40, (getHeight() - 20) / 2, 20, 20);
}

void PresetSection::PresetRowComponent::mouseDown(const juce::MouseEvent& e)
{
    if (isEditableMode) return;
    owner.presetList.selectRow(rowDataIndex + (owner.isSavingMode ? 1 : 0));
    owner.listBoxItemClicked(rowDataIndex + (owner.isSavingMode ? 1 : 0), e);
}
