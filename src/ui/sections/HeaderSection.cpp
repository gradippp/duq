#include "HeaderSection.h"
#include "../utils/IconFactory.h"
#include "../utils/FontManager.h"
#include "../../Globals.h"

HeaderSection::HeaderSection()
{
    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                Theme::Colours::uiHover);

            auto normal = Icons::load(iconName, Theme::Colours::textMain);
            auto over = Icons::load(iconName, Theme::Colours::textMain.withAlpha(0.85f));
            auto down = Icons::load(iconName, Theme::Colours::textMain.withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);
        };

    setupIconButton(undoButton, "undo");
    setupIconButton(redoButton, "redo");
    setupIconButton(saveProjectButton, "save");
    setupIconButton(initPresetButton, "close"); 
    
    undoButton.setTooltip("Undo");
    redoButton.setTooltip("Redo");
    saveProjectButton.setTooltip("Save Project Preset");
    initPresetButton.setTooltip("Init Preset (Reset State)");

    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    addAndMakeVisible(saveProjectButton);
    addAndMakeVisible(initPresetButton);

    addAndMakeVisible(presetNameLabel);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    presetNameLabel.setFont(FontManager::getJetBrainsMono(15.0f));
    presetNameLabel.setColour(juce::Label::textColourId, Theme::Colours::textMain.withAlpha(0.85f));
    presetNameLabel.setText(presetName.toUpperCase(), juce::dontSendNotification);
    presetNameLabel.onSingleClick = [this] { if (onLoadProject) onLoadProject(); };

    addAndMakeVisible(brandLabel);
    brandLabel.setText("DUQ", juce::dontSendNotification);
    brandLabel.setFont(FontManager::getInterBold(28.0f));
    brandLabel.setColour(juce::Label::textColourId, Theme::Colours::textMain.withAlpha(0.9f));
    brandLabel.onSingleClick = [this] { if (onAboutClicked) onAboutClicked(); };

    knobLookAndFeel = std::make_unique<FlatKnobLookAndFeel>();
    mixKnob = std::make_unique<ControlKnobComponent>("", 100.0f, "%");
    mixKnob->getSlider().setLookAndFeel(knobLookAndFeel.get());
    addAndMakeVisible(mixKnob.get());

    undoButton.onClick = [this] { if (undoCallback) undoCallback(); };
    redoButton.onClick = [this] { if (redoCallback) redoCallback(); };
    saveProjectButton.onClick = [this] { if (onSaveProject) onSaveProject(); };
    initPresetButton.onClick = [this] { if (onInitPreset) onInitPreset(); };

    // --- Lookahead / Lookbehind ---
    auto setupTimingSlider = [this](juce::Slider& s, const juce::String& tooltip)
    {
        s.setSliderStyle(juce::Slider::LinearBarVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setTooltip(tooltip);
        s.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        s.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(s);
    };

    setupTimingSlider(lookaheadSlider, "Lookahead (ms) - Delays audio to duck earlier");
    setupTimingSlider(lookbehindSlider, "Lookbehind (ms) - Delays envelope trigger");

    lookaheadSlider.onValueChange = [this] { repaint(); };
    lookbehindSlider.onValueChange = [this] { repaint(); };
}

void HeaderSection::setupAttachments(juce::AudioProcessorValueTreeState& vts)
{
    lookaheadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "lookahead", lookaheadSlider);
    lookbehindAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "lookbehind", lookbehindSlider);
    
    if (mixKnob)
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "mix", mixKnob->getSlider());
}

//==============================================================================

void HeaderSection::setPresetName(const juce::String& name)
{
    presetName = name;
    presetNameLabel.setText(presetName.toUpperCase(), juce::dontSendNotification);
    repaint();
}

void HeaderSection::updateUndoState(bool canUndo, bool canRedo)
{
    undoButton.setEnabled(canUndo);
    redoButton.setEnabled(canRedo);

    undoButton.setAlpha(canUndo ? 1.0f : 0.4f);
    redoButton.setAlpha(canRedo ? 1.0f : 0.4f);
}

void HeaderSection::setUndoCallback(std::function<void()> cb)
{
    undoCallback = std::move(cb);
}

void HeaderSection::setRedoCallback(std::function<void()> cb)
{
    redoCallback = std::move(cb);
}

void HeaderSection::setVersionString(const juce::String& version)
{
    versionString = version;
    repaint();
}

void HeaderSection::setProjectURI(const juce::String& uri)
{
    projectURI = uri;
    repaint();
}

//==============================================================================

void HeaderSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // ---------- Background ----------
    g.setColour(Theme::Colours::headerBackground);
    g.fillAll();

    // Subtle metallic top highlight
    g.setColour(Theme::Colours::accent.withAlpha(0.03f));
    g.fillRect(bounds.removeFromTop(1.0f));

    // ---------- Bottom Divider ----------
    g.setColour(Theme::Colours::border);
    g.drawLine(0.0f, bounds.getBottom() - 1.0f, bounds.getRight(), bounds.getBottom() - 1.0f, 1.0f);

    // ---------- Preset "Bay" (Center) ----------
    auto centerArea = getLocalBounds().withSizeKeepingCentre(280, 28).toFloat();
    g.setColour(Theme::Colours::background.withAlpha(0.4f));
    g.fillRoundedRectangle(centerArea, 2.0f);
    g.setColour(Theme::Colours::border.withAlpha(0.5f));
    g.drawRoundedRectangle(centerArea, 2.0f, 1.0f);

    // ---------- Slider Labels ----------
    g.setFont(FontManager::getBarlowBold(10.0f));
    
    auto drawInteractiveLabel = [&](juce::Slider& s, const juce::String& name)
    {
        if (!s.isVisible()) return;
        
        auto b = s.getBounds().toFloat();
        
        // Draw Name
        g.setColour(Theme::Colours::textDimmed);
        g.drawText(name, b.withY(b.getY() - 12).withHeight(12), juce::Justification::centred);
        
        // Draw Value
        g.setColour(Theme::Colours::textMain.withAlpha(0.9f));
        g.setFont(FontManager::getJetBrainsMono(12.0f));
        g.drawText(juce::String(s.getValue(), 1) + " ms", b, juce::Justification::centred);
    };

    drawInteractiveLabel(lookaheadSlider, "LOOKAHEAD");
    drawInteractiveLabel(lookbehindSlider, "LOOKBEHIND");
}

//==============================================================================

void HeaderSection::resized()
{
    auto area = getLocalBounds();

    // --- Left Brand ---
    brandLabel.setBounds(area.removeFromLeft(100).reduced(15, 0));

    // --- Timing Sliders (between Brand and Center) ---
    auto timingArea = area.removeFromLeft(120);
    int sliderHeight = 12;
    lookaheadSlider.setBounds(timingArea.removeFromTop(getHeight() / 2).withSizeKeepingCentre(100, sliderHeight).translated(0, 5));
    lookbehindSlider.setBounds(timingArea.withSizeKeepingCentre(100, sliderHeight).translated(0, 5));

    const int buttonSize = 24;
    const int spacing = 4;

    // --- Center Preset Group ---
    // [Name Bay (280px)] [Init] [Save]
    auto centerGroupArea = getLocalBounds().withSizeKeepingCentre(400, getHeight());
    
    // Name Bay centered
    int bayWidth = 280;
    auto bayRect = centerGroupArea.withSizeKeepingCentre(bayWidth, 28);
    presetNameLabel.setBounds(bayRect);
    
    // Init and Save next to the bay
    auto initPos = bayRect.getRelativePoint(1.0f, 0.5f).translated(spacing, -buttonSize/2);
    initPresetButton.setBounds(initPos.x, initPos.y, buttonSize, buttonSize);

    auto savePos = initPos.translated(buttonSize + spacing, 0);
    saveProjectButton.setBounds(savePos.x, savePos.y, buttonSize, buttonSize);

    // --- Right Tools Area ---
    auto rightArea = getLocalBounds().removeFromRight(200).reduced(10, 0);
    
    // Mix Knob at the far right
    if (mixKnob)
    {
        auto knobArea = rightArea.removeFromRight(60);
        mixKnob->setBounds(knobArea.reduced(0, 5));
    }

    rightArea.removeFromRight(15); // Gap

    // Undo / Redo
    redoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
    rightArea.removeFromRight(spacing);
    undoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
}
