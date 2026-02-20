#include "HeaderSection.h"
#include "../utils/IconFactory.h"
#include "../utils/FontManager.h"

CompactTimingSlider::CompactTimingSlider(const juce::String& label) : labelName(label)
{
    setSliderStyle(juce::Slider::LinearBarVertical);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    setRange(0.0, 100.0, 0.1);
}

void CompactTimingSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(Theme::Colours::sectionBackground.withAlpha(0.8f));
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Fill based on value (Visual Progress)
    auto fillWidth = bounds.getWidth() * static_cast<float>(getValue() / getMaximum());
    g.setColour(Theme::Colours::accent.withAlpha(0.1f));
    g.fillRoundedRectangle(bounds.withWidth(fillWidth), 2.0f);

    // Label
    g.setColour(Theme::Colours::textLabel);
    g.setFont(FontManager::getBarlowBold(10.0f));
    auto labelArea = bounds.removeFromLeft(bounds.getWidth() * 0.5f).reduced(6, 0);
    g.drawFittedText(labelName, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
    
    // Value
    g.setColour(Theme::Colours::textMain);
    g.setFont(FontManager::getJetBrainsMono(11.0f));
    g.drawFittedText(juce::String(getValue(), 1) + " ms", bounds.reduced(6, 0).toNearestInt(), juce::Justification::centredRight, 1);
    
    // Border
    g.setColour(Theme::Colours::border.withAlpha(0.5f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
}

void CompactTimingSlider::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        showValueEntryDialog();
        return;
    }
    juce::Slider::mouseDown(e);
}

void CompactTimingSlider::showValueEntryDialog()
{
    auto* window = new juce::AlertWindow("Enter Value", "Type a new value (ms):", juce::AlertWindow::NoIcon);
    window->addTextEditor("value", juce::String(getValue()));
    window->addButton("OK", 1);
    window->addButton("Cancel", 0);
    
    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result) {
        if (result == 1)
        {
            auto val = window->getTextEditor("value")->getText().getDoubleValue();
            setValue(val, juce::sendNotification);
        }
        delete window;
    }));
}

//==============================================================================

CompactKnob::CompactKnob(const juce::String& label) : labelName(label)
{
    setLookAndFeel(&lnf);
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters(juce::degreesToRadians(135.0f), juce::degreesToRadians(405.0f), true);
    setRange(0.0, 1.0);
}

CompactKnob::~CompactKnob()
{
    setLookAndFeel(nullptr);
}

void CompactKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() < 10.0f || bounds.getHeight() < 10.0f) return;

    auto h = bounds.getHeight();
    auto knobArea = bounds.removeFromLeft(h).reduced(2.0f);
    
    if (knobArea.getWidth() > 0 && knobArea.getHeight() > 0)
    {
        float normalizedValue = (float)valueToProportionOfLength(getValue());
        getLookAndFeel().drawRotarySlider(g, (int)knobArea.getX(), (int)knobArea.getY(), (int)knobArea.getWidth(), (int)knobArea.getHeight(),
                                          normalizedValue,
                                          juce::degreesToRadians(135.0f), juce::degreesToRadians(405.0f), *this);
    }

    // Label and Value
    if (bounds.getWidth() > 10.0f)
    {
        g.setColour(Theme::Colours::textLabel);
        g.setFont(FontManager::getBarlowBold(10.0f));
        auto labelArea = bounds.removeFromTop(bounds.getHeight() * 0.5f).reduced(4, 0);
        g.drawFittedText(labelName, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
        
        g.setColour(Theme::Colours::textMain);
        g.setFont(FontManager::getJetBrainsMono(10.0f));
        g.drawFittedText(juce::String(juce::roundToInt(getValue())) + "%", bounds.reduced(4, 0).toNearestInt(), juce::Justification::centredLeft, 1);
    }
}

HeaderSection::HeaderSection()
    : lookaheadSlider("LOOKAHEAD"),
      lookbehindSlider("LOOKBEHIND"),
      mixKnob("MIX")
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
    setupIconButton(settingsButton, "settings");
    
    undoButton.setTooltip("Undo");
    redoButton.setTooltip("Redo");
    saveProjectButton.setTooltip("Save Project Preset");
    initPresetButton.setTooltip("Init Preset (Reset State)");
    settingsButton.setTooltip("Settings");

    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    addAndMakeVisible(saveProjectButton);
    addAndMakeVisible(initPresetButton);
    addAndMakeVisible(settingsButton);

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

    addAndMakeVisible(mixKnob);
    mixKnob.setTooltip("Global Wet/Dry Mix");

    undoButton.onClick = [this] { if (undoCallback) undoCallback(); };
    redoButton.onClick = [this] { if (redoCallback) redoCallback(); };
    saveProjectButton.onClick = [this] { if (onSaveProject) onSaveProject(); };
    initPresetButton.onClick = [this] { if (onInitPreset) onInitPreset(); };
    settingsButton.onClick = [this] { if (onSettingsClicked) onSettingsClicked(); };

    // --- Lookahead / Lookbehind ---
    addAndMakeVisible(lookaheadSlider);
    addAndMakeVisible(lookbehindSlider);

    lookaheadSlider.onValueChange = [this] { repaint(); };
    lookbehindSlider.onValueChange = [this] { repaint(); };
}

void HeaderSection::setupAttachments(juce::AudioProcessorValueTreeState& vts)
{
    lookaheadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "lookahead", lookaheadSlider);
    lookbehindAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "lookbehind", lookbehindSlider);
    
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "mix", mixKnob);
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
}

//==============================================================================

void HeaderSection::resized()
{
    auto area = getLocalBounds();

    // --- Left Brand ---
    brandLabel.setBounds(area.removeFromLeft(100).reduced(15, 0));

    const int buttonSize = 24;
    const int spacing = 6;
    const int timingWidth = 140;
    const int timingHeight = 20;
    const int timingGap = 2;
    const int bayWidth = 280;

    // Center Preset Group Rect
    auto centerBay = getLocalBounds().withSizeKeepingCentre(bayWidth, 28);
    presetNameLabel.setBounds(centerBay);

    // Timing Sliders stacked to the left of the bay
    int stackX = centerBay.getX() - spacing - timingWidth;
    int totalStackHeight = (timingHeight * 2) + timingGap;
    int stackY = centerBay.getCentreY() - totalStackHeight / 2;

    lookaheadSlider.setBounds(stackX, stackY, timingWidth, timingHeight);
    lookbehindSlider.setBounds(stackX, stackY + timingHeight + timingGap, timingWidth, timingHeight);

    // Init/Save to the right of the bay
    initPresetButton.setBounds(centerBay.getRight() + spacing, centerBay.getCentreY() - buttonSize/2, buttonSize, buttonSize);
    saveProjectButton.setBounds(initPresetButton.getRight() + spacing, centerBay.getCentreY() - buttonSize/2, buttonSize, buttonSize);

    // --- Right Area (Mix, Undo/Redo) ---
    auto rightArea = getLocalBounds().removeFromRight(230).reduced(10, 0);
    
    settingsButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
    rightArea.removeFromRight(spacing);

    mixKnob.setBounds(rightArea.removeFromRight(80).reduced(0, 5));

    rightArea.removeFromRight(15);

    redoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
    rightArea.removeFromRight(spacing);
    undoButton.setBounds(rightArea.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
}
