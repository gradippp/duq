#include "ControlSection.h"
#include "../../actions/ControlUndoActions.h"

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

    // ===== Default local state =====
    rateIsFrequencyMode = true;

    // ===== Rate Context Menu =====
    rateKnob.extendContextMenu =
        [this](juce::PopupMenu& menu)
        {
            menu.addItem(100,
                "Set to Frequency mode",
                true,
                rateIsFrequencyMode);
        };

    rateKnob.onCustomMenuResult =
        [this](int result)
        {
            if (result == 100 && currentData && undoManager)
            {
                bool newValue = !currentData->rateIsFrequencyMode;

                undoManager->perform(
                    new ChangeBoolMemberAction(
                        *currentData,
                        &EnvelopeData::rateIsFrequencyMode,
                        newValue));

                if (onEnvelopeChanged)
                    onEnvelopeChanged(*currentData);
            }
        };


    // ===== Rate Formatter =====
    rateKnob.valueFormatter =
        [this](double value)
        {
            if (rateIsFrequencyMode)
                return juce::String(value, 2) + " Hz";

            int index = (int)value;
            index = juce::jlimit(0,
                (int)rateDivisions.size() - 1,
                index);

            return rateDivisions[index];
        };

    // ===== Slider Callbacks =====
    for (size_t i = 0; i < knobList.size(); ++i)
    {
        knobList[i]->onValueChanged =
            [this, i](double value)
            {
                if (isInitialising)
                    return;

                if (!currentData || !undoManager)
                    return;

                auto member = dataMembers[i];

                undoManager->perform(
                    new ChangeDoubleMemberAction(
                        *currentData,
                        member,
                        value));

                if (onEnvelopeChanged)
                    onEnvelopeChanged(*currentData);
            };
    }


    // ===== Add Components =====
    addAndMakeVisible(rateKnob);
    addAndMakeVisible(depthKnob);
    addAndMakeVisible(smoothKnob);

    // ===== Initialize Mode =====
    applyRateMode();
}

ControlSection::~ControlSection()
{
    if (undoManager)
        undoManager->removeChangeListener(this);
}

void ControlSection::setUndoManager(juce::UndoManager& um)
{
    undoManager = &um;
    undoManager->addChangeListener(this);
}

void ControlSection::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (!currentData)
        return;

    loadEnvelope(*currentData);

    if (onEnvelopeChanged)
        onEnvelopeChanged(*currentData);
}

void ControlSection::clearEnvelope()
{
    currentData = nullptr;
    hasEnvelope = false;

    rateKnob.setVisible(false);
    depthKnob.setVisible(false);
    smoothKnob.setVisible(false);

    repaint();
}

void ControlSection::loadEnvelope(EnvelopeData& data)
{
    isInitialising = true;

    currentData = &data;
    hasEnvelope = true;

    rateKnob.setVisible(true);
    depthKnob.setVisible(true);
    smoothKnob.setVisible(true);

    rateIsFrequencyMode = data.rateIsFrequencyMode;
    applyRateMode();

    rateKnob.getSlider().setValue(data.rate, juce::dontSendNotification);
    depthKnob.getSlider().setValue(data.depth, juce::dontSendNotification);
    smoothKnob.getSlider().setValue(data.smooth, juce::dontSendNotification);

    rateKnob.refreshValueLabel();
    depthKnob.refreshValueLabel();
    smoothKnob.refreshValueLabel();

    isInitialising = false;

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
    if (!currentData)
        return;

    if (currentData->rateIsFrequencyMode)
    {
        rateKnob.getSlider().setRange(0.1, 100.0f, 0.01);
        rateKnob.setLabel("Frequency");
    }
    else
    {
        rateKnob.getSlider().setRange(
            0,
            (int)rateDivisions.size() - 1,
            1.0);
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
