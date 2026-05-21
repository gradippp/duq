#include <memory>
#include "EnvelopeListSection.h"
#include "../../PluginProcessor.h"
#include "../utils/FontManager.h"
#include "../../model/EnvelopeData.h"
#include "../../utils/ConfigManager.h"
#include "../../Globals.h"
#include "../../utils/PresetManager.h"
#include "../utils/IconFactory.h"

juce::Font EnvelopeListSection::CustomButtonLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return FontManager::getBarlowBold(12.0f);
}

void EnvelopeListSection::CustomButtonLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);

    auto textColour = button.findColour(button.isEnabled() ? juce::TextButton::textColourOffId : juce::TextButton::textColourOnId);
    g.setColour(textColour);

    auto bounds = button.getLocalBounds().toFloat();
    auto iconName = button.getComponentID();

    if (iconName.isNotEmpty())
    {
        auto icon = Icons::load(iconName, textColour);
        if (icon != nullptr)
        {
            float iconSize = 14.0f;
            float spacing = 6.0f;
            float textWidth = g.getCurrentFont().getStringWidthFloat(button.getButtonText());
            float totalWidth = iconSize + spacing + textWidth;

            auto startX = (bounds.getWidth() - totalWidth) * 0.5f;
            auto iconArea = juce::Rectangle<float>(startX, (bounds.getHeight() - iconSize) * 0.5f, iconSize, iconSize);
            icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred, 1.0f);

            auto textArea = bounds.withLeft(iconArea.getRight() + spacing);
            g.drawFittedText(button.getButtonText(), textArea.toNearestInt(), juce::Justification::centredLeft, 2);
            return;
        }
    }

    g.drawFittedText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 2);
}

EnvelopeListSection::EnvelopeListSection()
{
    auto setupButton = [this](juce::TextButton& button, const juce::String& text, const juce::String& iconName, const juce::String& tooltip)
    {
        button.setLookAndFeel(&buttonLnf);
        button.setButtonText(text);
        button.setComponentID(iconName);
        button.setTooltip(tooltip);
        button.setColour(juce::TextButton::buttonColourId, T_COL(background).withAlpha(0.4f));
        button.setColour(juce::TextButton::buttonOnColourId, T_COL(uiHover));
        button.setColour(juce::TextButton::textColourOffId, T_COL(textLabel));
        button.setColour(juce::TextButton::textColourOnId, T_COL(textMain));
    };

    setupButton(addButton, "ADD", "add", "Add a new default envelope");
    setupButton(importButton, "IMPORT", "import", "Import an envelope preset (.duq.env)");

    addAndMakeVisible(addButton);
    addAndMakeVisible(importButton);

    viewport.setLookAndFeel(&viewportLnf);
    viewport.setViewedComponent(&rowContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    addButton.onClick = [this]()
        {
            if (processor == nullptr)
                return;

            undoManager->beginNewTransaction("Add Envelope");
            
            const int newIndex = envelopesTree.getNumChildren();
            processor->addEnvelope(generateDefaultName(), -1); // -1 uses config default note
            selectEnvelope(newIndex);
        };

    importButton.onClick = [this]()
        {
            if (onImportRequested)
                onImportRequested();
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

void EnvelopeListSection::valueTreeChildOrderChanged(juce::ValueTree& v, int, int)
{
    if (v == envelopesTree)
        rebuildRowsFromModel();
}

void EnvelopeListSection::valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& i)
{
    if (v == envelopesTree)
        rebuildRowsFromModel();
}


void EnvelopeListSection::lookAndFeelChanged()
{
    addButton.setColour(juce::TextButton::buttonColourId, T_COL(background).withAlpha(0.4f));
    addButton.setColour(juce::TextButton::buttonOnColourId, T_COL(uiHover));
    addButton.setColour(juce::TextButton::textColourOffId, T_COL(textLabel));
    addButton.setColour(juce::TextButton::textColourOnId, T_COL(textMain));

    importButton.setColour(juce::TextButton::buttonColourId, T_COL(background).withAlpha(0.4f));
    importButton.setColour(juce::TextButton::buttonOnColourId, T_COL(uiHover));
    importButton.setColour(juce::TextButton::textColourOffId, T_COL(textLabel));
    importButton.setColour(juce::TextButton::textColourOnId, T_COL(textMain));

    repaint();
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

        row->onSaveRequested = [this, envTree]()
            {
                if (onSaveRequested)
                    onSaveRequested(envTree);
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
    
    rowContainer.repaint();
    repaint();
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

    g.fillAll(T_COL(sectionBackground));

    // Outer border
    g.setColour(T_COL(border));
    g.drawRect(bounds, 1);

    // ===== Header Area =====
    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(T_COL(headerBackground));
    g.fillRect(headerArea);

    g.setColour(T_COL(textMain));
    g.setFont(FontManager::getBarlowBold(16.0f));

    g.drawText("ENVELOPES",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);

    auto footerBounds = getLocalBounds().removeFromBottom(40);
    g.setColour(T_COL(border).withAlpha(0.5f));
    g.drawLine(0.0f, footerBounds.getY(), (float)getWidth(), footerBounds.getY(), 1.0f);
}

void EnvelopeListSection::paintOverChildren(juce::Graphics& g)
{
    // Drop Indicator
    if (isDragging && dropIndex >= 0)
    {
        constexpr int headerHeight = 32;
        constexpr int rowHeight = 28;
        auto footerBounds = getLocalBounds().removeFromBottom(40);
        
        float dropY = (float)(headerHeight + dropIndex * rowHeight) - (float)viewport.getViewPosition().y;

        // Ensure we don't draw over the header or footer
        if (dropY >= (float)headerHeight && dropY < (float)footerBounds.getY())
        {
            g.setColour(T_COL(accent));
            g.drawLine(0.0f, dropY, (float)getWidth(), dropY, 2.0f);
        }
    }
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
    juce::SharedResourcePointer<ConfigManager> config;

    while (true)
    {
        juce::String candidate = config->getProps()->getValue("envelopeName", Defaults::envelopeName) + " " + juce::String(counter);
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

// ==============================================================================
// Drag and Drop Target
// ==============================================================================

bool EnvelopeListSection::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    return dynamic_cast<EnvelopeRowComponent*>(details.sourceComponent.get()) != nullptr;
}

void EnvelopeListSection::itemDragEnter(const juce::DragAndDropTarget::SourceDetails&)
{
    isDragging = true;
    repaint();
}

void EnvelopeListSection::itemDragMove(const juce::DragAndDropTarget::SourceDetails& details)
{
    auto localPos = details.localPosition.toFloat();
    
    // Header is 32px
    float y = localPos.y - 32.0f + (float)viewport.getViewPosition().y;
    
    // We want to insert BETWEEN rows, so we use floor to find which row slot the mouse is in
    int index = juce::jlimit(0, envelopesTree.getNumChildren(), (int)std::floor(y / 28.0f + 0.5f));

    if (index != dropIndex)
    {
        dropIndex = index;
        repaint();
    }
}

void EnvelopeListSection::itemDragExit(const juce::DragAndDropTarget::SourceDetails&)
{
    isDragging = false;
    dropIndex = -1;
    repaint();
}

void EnvelopeListSection::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    auto* sourceRow = dynamic_cast<EnvelopeRowComponent*>(details.sourceComponent.get());
    if (sourceRow != nullptr && envelopesTree.isValid())
    {
        int currentIndex = rows.indexOf(sourceRow);
        int newIndex = dropIndex;

        if (currentIndex >= 0 && newIndex >= 0)
        {
            // If dropping below the current item, the new index shifts
            if (newIndex > currentIndex)
                newIndex--;

            if (currentIndex != newIndex)
            {
                if (undoManager != nullptr)
                    undoManager->beginNewTransaction("Reorder Envelopes");

                selectedIndex = newIndex;
                envelopesTree.moveChild(currentIndex, newIndex, undoManager);
            }
        }
    }

    isDragging = false;
    dropIndex = -1;
    repaint();
    rowContainer.repaint();
}

