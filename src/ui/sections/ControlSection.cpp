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

    // ===== Default local state =====
    currentData = EnvelopeData();  // default initialize
    rateIsFrequencyMode = currentData.rateIsFrequencyMode;

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
            if (result == 100)
            {
                rateIsFrequencyMode = !rateIsFrequencyMode;
                currentData.rateIsFrequencyMode = rateIsFrequencyMode;

                applyRateMode();

                if (onEnvelopeChanged)
                    onEnvelopeChanged(currentData);
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
                currentData.*(dataMembers[i]) = value;

                if (onEnvelopeChanged)
                    onEnvelopeChanged(currentData);
            };
    }


    // ===== Add Components =====
    addAndMakeVisible(rateKnob);
    addAndMakeVisible(depthKnob);
    addAndMakeVisible(smoothKnob);

    // ===== Initialize Mode =====
    applyRateMode();
}

void ControlSection::loadEnvelope(const EnvelopeData& data)
{
    currentData = data;   // VERY IMPORTANT

    rateIsFrequencyMode = data.rateIsFrequencyMode;
    applyRateMode();

    rateKnob.getSlider().setValue(data.rate, juce::dontSendNotification);
    depthKnob.getSlider().setValue(data.depth, juce::dontSendNotification);
    smoothKnob.getSlider().setValue(data.smooth, juce::dontSendNotification);

    rateKnob.refreshValueLabel();
    depthKnob.refreshValueLabel();
    smoothKnob.refreshValueLabel();
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
}

void ControlSection::applyRateMode()
{
    if (rateIsFrequencyMode)
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
