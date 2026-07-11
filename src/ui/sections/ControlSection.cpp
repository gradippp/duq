#include "ControlSection.h"
#include "../utils/FontManager.h"
#include "../../PluginProcessor.h"
#include "../../dsp/RateUtils.h"
#include "../../Globals.h"

static const std::vector<juce::String> rateDivisions =
{
    "1/1",
    "1/2",
    "1/4",
    "1/8",
    "1/16",
    "1/32"
};

ControlSection::ControlSection()
{
    knobList = { &rateKnob, &depthKnob, &smoothKnob };

    for (auto* knob : knobList)
    {
        knob->getSlider().onDragStart = [this]
            {
                if (undoManager)
                    undoManager->beginNewTransaction("Change Control");
            };
    }

    rateKnob.extendContextMenu =
        [this](juce::PopupMenu& menu)
        {
            menu.addItem(100,
                "Toggle Frequency Mode",
                true,
                rateIsFrequencyMode);
        };

    rateKnob.onCustomMenuResult =
        [this](int result)
        {
            if (result == 100 && envelope.isValid())
            {
                bool newValue = !(bool)envelope["rateIsFrequencyMode"];
                envelope.setProperty("rateIsFrequencyMode", newValue, undoManager);
            }
        };

    rateKnob.valueFormatter =
        [this](double value)
        {
            if (rateIsFrequencyMode)
                return juce::String(value, 2) + " Hz";

            // Map 0..100 to a sync division label
            return rateDivisions[static_cast<size_t>(RateUtils::syncDivisionIndex(value))];
        };

    smoothKnob.valueFormatter =
        [](double value)
        {
            return juce::String(value, 1) + " ms";
        };

    rateKnob.onValueChanged = [this](double value) {
        if (envelope.isValid() && rateAttachment == nullptr)
            envelope.setProperty("rate", value, undoManager);
    };

    depthKnob.onValueChanged = [this](double value) {
        if (envelope.isValid() && depthAttachment == nullptr)
            envelope.setProperty("depth", value, undoManager);
    };

    smoothKnob.onValueChanged = [this](double value) {
        if (envelope.isValid() && smoothAttachment == nullptr)
            envelope.setProperty("smooth", value, undoManager);
    };

    depthKnob.getSlider().setRange(0.0, 100.0, 0.1);
    smoothKnob.getSlider().setRange(0.0, 500.0, 0.1);

    addAndMakeVisible(rateKnob);
    addAndMakeVisible(depthKnob);
    addAndMakeVisible(smoothKnob);
}


ControlSection::~ControlSection()
{
    if (envelope.isValid())
        envelope.removeListener(this);
}

void ControlSection::setEnvelope(juce::ValueTree env)
{
    if (envelope.isValid())
        envelope.removeListener(this);

    envelope = env;

    // Reset attachments first
    rateAttachment.reset();
    depthAttachment.reset();
    smoothAttachment.reset();

    if (envelope.isValid())
    {
        envelope.addListener(this);
        hasEnvelope = true;

        rateKnob.setVisible(true);
        depthKnob.setVisible(true);
        smoothKnob.setVisible(true);

        // Link to automation parameters if within the automated envelope slots
        if (processor != nullptr)
        {
            auto parent = envelope.getParent();
            if (parent.isValid())
            {
                int index = parent.indexOf(envelope);
                if (index >= 0 && index < Defaults::maxEnvelopeSlots)
                {
                    juce::String prefix = "env" + juce::String(index) + "_";
                    auto& vts = processor->parameters;
                    
                    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "rate", rateKnob.getSlider());
                    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "depth", depthKnob.getSlider());
                    smoothAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "smooth", smoothKnob.getSlider());
                }
            }
        }

        refreshFromTree();
    }
    else
    {
        clearEnvelope();
    }
}

void ControlSection::setProcessor(DuqAudioProcessor* p)
{
    processor = p;
}

void ControlSection::refreshFromTree()
{
    if (!envelope.isValid())
        return;

    isInitialising = true;

    rateIsFrequencyMode = (bool)envelope["rateIsFrequencyMode"];
    applyRateMode();

    if (rateAttachment == nullptr)
        rateKnob.getSlider().setValue((double)envelope["rate"], juce::dontSendNotification);

    if (depthAttachment == nullptr)
        depthKnob.getSlider().setValue((double)envelope["depth"], juce::dontSendNotification);

    if (smoothAttachment == nullptr)
        smoothKnob.getSlider().setValue((double)envelope["smooth"], juce::dontSendNotification);

    rateKnob.refreshValueLabel();
    depthKnob.refreshValueLabel();
    smoothKnob.refreshValueLabel();

    isInitialising = false;

    repaint();
}


void ControlSection::valueTreePropertyChanged(
    juce::ValueTree&,
    const juce::Identifier&)
{
    if (isInitialising)
        return;

    refreshFromTree();
}


void ControlSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
}

void ControlSection::clearEnvelope()
{
    if (envelope.isValid())
        envelope.removeListener(this);

    envelope = {};
    hasEnvelope = false;

    // Reset attachments
    rateAttachment.reset();
    depthAttachment.reset();
    smoothAttachment.reset();

    rateKnob.setVisible(false);
    depthKnob.setVisible(false);
    smoothKnob.setVisible(false);

    repaint();
}

void ControlSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(T_COL(sectionBackground));

    g.setColour(T_COL(border));
    g.drawRect(bounds, 1);

    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(T_COL(headerBackground));
    g.fillRect(headerArea);

    g.setColour(T_COL(textMain));
    g.setFont(FontManager::getBarlowBold(16.0f));

    g.drawText("CONTROLS",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);

    if (!hasEnvelope)
    {
        g.setColour(T_COL(textDimmed));
        g.setFont(FontManager::getInterRegular(14.0f));

        g.drawText("Select an envelope to edit",
            bounds,
            juce::Justification::centred);

        return;
    }
}

void ControlSection::applyRateMode()
{
    if (!envelope.isValid())
        return;

    // Keep range consistent with APVTS parameter (0.1..100.0)
    // The valueFormatter handles the "Sync" display (1/1, 1/2 etc).
    rateKnob.getSlider().setRange(0.1, 100.0, 0.01);

    if (rateIsFrequencyMode)
        rateKnob.setLabel("Frequency");
    else
        rateKnob.setLabel("Rate");

    rateKnob.refreshValueLabel();
}

void ControlSection::lookAndFeelChanged()
{
    for (auto* knob : knobList)
    {
        knob->sendLookAndFeelChange();
    }
    repaint();
}

void ControlSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr int headerHeight = 32;
    constexpr int rowHeight = 56;
    constexpr int gap = 4;
    constexpr int bottomPadding = 8;

    bounds.removeFromTop(headerHeight);
    bounds.removeFromBottom(bottomPadding);

    rateKnob.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(gap);

    depthKnob.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(gap);

    smoothKnob.setBounds(bounds.removeFromTop(rowHeight));
}
