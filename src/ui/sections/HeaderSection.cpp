#include "HeaderSection.h"
#include "../utils/IconFactory.h"

HeaderSection::HeaderSection()
{
    auto setupIconButton = [](juce::DrawableButton& button,
        const juce::String& iconName)
        {
            button.setClickingTogglesState(false);

            button.setColour(juce::DrawableButton::backgroundColourId,
                juce::Colours::transparentBlack);

            button.setColour(juce::DrawableButton::backgroundOnColourId,
                juce::Colours::white.withAlpha(0.08f));

            auto normal = Icons::load(iconName, juce::Colours::white);
            auto over = Icons::load(iconName, juce::Colours::white.withAlpha(0.85f));
            auto down = Icons::load(iconName, juce::Colours::white.withAlpha(0.6f));

            if (normal != nullptr)
                button.setImages(normal.get(), over.get(), down.get(), nullptr);
        };

    setupIconButton(undoButton, "undo");
    setupIconButton(redoButton, "redo");

    undoButton.setTooltip("Undo");
    redoButton.setTooltip("Redo");

    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);

    undoButton.onClick = [this]
        {
            if (undoCallback)
                undoCallback();
        };

    redoButton.onClick = [this]
        {
            if (redoCallback)
                redoCallback();
        };
}

//==============================================================================

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

    // ---------- Background Gradient ----------
    juce::ColourGradient gradient(
        juce::Colour(18, 18, 18),
        0, 0,
        juce::Colour(10, 10, 10),
        0, bounds.getBottom(),
        false);

    g.setGradientFill(gradient);
    g.fillAll();

    // ---------- Bottom Divider ----------
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(0.0f,
        bounds.getBottom() - 1.0f,
        bounds.getRight(),
        bounds.getBottom() - 1.0f,
        1.0f);

    const int leftPadding = 24;
    const int rightPadding = 24;

    // ---------- Brand Title ----------
    g.setColour(juce::Colours::white);

    juce::Font brandFont(48.0f, juce::Font::bold);
    g.setFont(brandFont);

    juce::Rectangle<int> titleArea(leftPadding, 0, 240, getHeight());
    g.drawText("DUQ",
        titleArea,
        juce::Justification::centredLeft);

    // Accent underline
    auto accentY = getHeight() - 6;
    g.setColour(juce::Colour(0xff4cc9f0)); // subtle blue accent
    g.drawLine((float)leftPadding,
        (float)accentY,
        (float)(leftPadding + 70),
        (float)accentY,
        2.0f);

    // ---------- Right Meta Info ----------
    const int metaWidth = 260;
    juce::Rectangle<int> metaArea(
        getWidth() - metaWidth - rightPadding,
        0,
        metaWidth,
        getHeight());

    // Project URI (top-right)
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(11.0f));
    g.drawText(projectURI,
        metaArea.removeFromTop(22),
        juce::Justification::centredRight);

    // Version (bottom-right)
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("v" + versionString,
        metaArea.removeFromBottom(24),
        juce::Justification::centredRight);
}

//==============================================================================

void HeaderSection::resized()
{
    auto area = getLocalBounds();

    const int buttonSize = 30;
    const int spacing = 14;
    const int rightInset = 24;

    auto rightArea = area.removeFromRight(140);
    rightArea.removeFromRight(rightInset);

    auto buttonArea = rightArea.removeFromLeft(buttonSize);
    undoButton.setBounds(
        buttonArea.withSizeKeepingCentre(buttonSize, buttonSize));

    rightArea.removeFromLeft(spacing);

    buttonArea = rightArea.removeFromLeft(buttonSize);
    redoButton.setBounds(
        buttonArea.withSizeKeepingCentre(buttonSize, buttonSize));
}
