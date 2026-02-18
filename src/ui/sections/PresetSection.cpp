#include "PresetSection.h"
#include "../utils/IconFactory.h"
#include "../utils/PresetManager.h"

PresetSection::PresetSection()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font("Segoe UI", 20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));

    auto normal = Icons::load("delete", juce::Colours::white.withAlpha(0.6f));
    auto over = Icons::load("delete", juce::Colours::white);
    auto down = Icons::load("delete", juce::Colours::white.withAlpha(0.4f));
    closeButton.setImages(normal.get(), over.get(), down.get());

    addAndMakeVisible(closeButton);
    closeButton.setTooltip("Close Preset Selection");
    
    closeButton.onClick = [this]() {
        if (onClose)
            onClose();
    };

    addAndMakeVisible(presetList);
    presetList.setModel(this);
    presetList.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetList.setRowHeight(32);

    setOpaque(true);
}

PresetSection::~PresetSection()
{
}

void PresetSection::refreshPresetList()
{
    presetFiles.clear();
    auto dir = PresetManager::getEnvelopeDirectory();
    
    if (dir.exists() && dir.isDirectory())
    {
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*" + PresetManager::envelopeExtension);
        for (auto& f : files)
            presetFiles.push_back(f);
    }
    
    presetList.updateContent();
}

void PresetSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}

int PresetSection::getNumRows()
{
    return (int)presetFiles.size();
}

void PresetSection::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= (int)presetFiles.size())
        return;

    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat();

    if (rowIsSelected)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(area.reduced(2.0f), 4.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::plain));
    
    juce::String fileName = presetFiles[rowNumber].getFileNameWithoutExtension();
    // For .duq.env, getFileNameWithoutExtension() might leave .duq if it only strips one extension
    if (fileName.endsWith(".duq")) 
         fileName = fileName.substring(0, fileName.length() - 4);

    g.drawText(fileName, area.reduced(10, 0), juce::Justification::centredLeft, true);
}

void PresetSection::listBoxItemClicked(int rowNumber, const juce::MouseEvent&)
{
    if (rowNumber >= (int)presetFiles.size() || !targetEnvelope.isValid())
        return;

    auto loaded = PresetManager::loadEnvelope(presetFiles[rowNumber]);
    if (loaded.isValid())
    {
        if (undoManager)
            undoManager->beginNewTransaction("Load Preset: " + presetFiles[rowNumber].getFileNameWithoutExtension());

        targetEnvelope.copyPropertiesAndChildrenFrom(loaded, undoManager);
        
        if (onClose)
            onClose();
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

    if (presetFiles.empty())
    {
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        g.setFont(juce::Font("Segoe UI", 16.0f, juce::Font::plain));
        g.drawFittedText("NO PRESETS FOUND IN\n" + PresetManager::getEnvelopeDirectory().getFullPathName(), 
                          getLocalBounds(), 
                          juce::Justification::centred, 
                          2);
    }
}

void PresetSection::resized()
{
    auto area = getLocalBounds();
    
    auto headerArea = area.removeFromTop(60);
    
    // Close button (X) in the top right
    const int xSize = 24;
    closeButton.setBounds(headerArea.removeFromRight(50).withSizeKeepingCentre(xSize, xSize));
    
    // Title is centered in the header area
    titleLabel.setBounds(headerArea.withLeft(50));

    presetList.setBounds(area.reduced(20, 10));
}

void PresetSection::setTargetEnvelope(juce::ValueTree envelope)
{
    targetEnvelope = envelope;
    refreshPresetList();
}
