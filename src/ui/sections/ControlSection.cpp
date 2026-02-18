#include "ControlSection.h"

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

            int index = juce::jlimit(0, 5, (int)value);
            return rateDivisions[index];
        };

    // Slider value changes
    rateKnob.onValueChanged = [this](double value)
        {
            if (!envelope.isValid() || isInitialising)
                return;

            envelope.setProperty("rate", value, undoManager);
        };

    depthKnob.onValueChanged = [this](double value)
        {
            if (!envelope.isValid() || isInitialising)
                return;

            envelope.setProperty("depth", value, undoManager);
        };

    smoothKnob.onValueChanged = [this](double value)
        {
            if (!envelope.isValid() || isInitialising)
                return;

            envelope.setProperty("smooth", value, undoManager);
        };

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

    if (envelope.isValid())
    {
        envelope.addListener(this);
        hasEnvelope = true;

        rateKnob.setVisible(true);
        depthKnob.setVisible(true);
        smoothKnob.setVisible(true);

        refreshFromTree();
    }
    else
    {
        clearEnvelope();
    }
}

void ControlSection::refreshFromTree()
{
    if (!envelope.isValid())
        return;

    isInitialising = true;

    rateIsFrequencyMode = (bool)envelope["rateIsFrequencyMode"];
    applyRateMode();

    rateKnob.getSlider().setValue((double)envelope["rate"], juce::dontSendNotification);
    depthKnob.getSlider().setValue((double)envelope["depth"], juce::dontSendNotification);
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

    rateKnob.setVisible(false);
    depthKnob.setVisible(false);
    smoothKnob.setVisible(false);

    repaint();
}

void ControlSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1);

    constexpr int headerHeight = 32;
    auto headerArea = bounds.removeFromTop(headerHeight);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.15f));
    g.fillRect(headerArea);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));

    g.drawText("Controls",
        headerArea.reduced(10, 0),
        juce::Justification::centredLeft);

    if (!hasEnvelope)
    {
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(14.0f));

        g.drawText("Add an envelope",
            bounds,
            juce::Justification::centred);

        return;
    }
}

void ControlSection::applyRateMode()
{
    if (!envelope.isValid())
        return;

    if (rateIsFrequencyMode)
    {
        rateKnob.getSlider().setRange(0.1, 100.0, 0.01);
        rateKnob.setLabel("Frequency");
    }
    else
    {
        rateKnob.getSlider().setRange(0, 5, 1);
        rateKnob.setLabel("Rate");
    }

    rateKnob.refreshValueLabel();
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
