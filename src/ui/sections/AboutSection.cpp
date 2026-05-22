#include "AboutSection.h"
#include "../utils/FontManager.h"
#include "../utils/IconFactory.h"
#include "BinaryData.h"

AboutSection::AboutSection()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(FontManager::getInterBold(54.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(versionLabel);
    versionLabel.setText("VERSION " + juce::String(PROJECT_VERSION) + " (build " + juce::String(PROJECT_GIT_COMMIT) + ")", juce::dontSendNotification);
    versionLabel.setFont(FontManager::getJetBrainsMono(14.0f));
    versionLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.4f));
    versionLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(viewport);
    viewport.setOpaque(false);
    viewport.setViewedComponent(&content, false);
    if (auto* viewed = viewport.getViewedComponent())
        viewed->setOpaque(false);
    
    viewport.setScrollBarsShown(true, false, true, false);
    viewport.setScrollBarThickness(10);
    viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
    viewport.setSingleStepSizes(1, 20);
    viewport.setLookAndFeel(&scrollbarLF);

    // Load text from BinaryData
    int dataSize = 0;
    const char* data = BinaryData::getNamedResource("about_txt", dataSize);
    if (data != nullptr && dataSize > 0)
    {
        content.setText(juce::String::fromUTF8(data, dataSize));
    }

    addAndMakeVisible(githubLink);
    githubLink.setFont(FontManager::getInterMedium(14.0f), false);
    githubLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff4cc9f0).withAlpha(0.8f));
    githubLink.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(websiteLink);
    websiteLink.setFont(FontManager::getInterMedium(14.0f), false);
    websiteLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff4cc9f0).withAlpha(0.8f));
    websiteLink.setJustificationType(juce::Justification::centredRight);

    auto normal = Icons::load("close", juce::Colours::white.withAlpha(0.6f));
    auto over = Icons::load("close", juce::Colours::white);
    auto down = Icons::load("close", juce::Colours::white.withAlpha(0.4f));
    closeButton.setImages(normal.get(), over.get(), down.get());
    addAndMakeVisible(closeButton);
    
    closeButton.onClick = [this] { if (onClose) onClose(); };

    setOpaque(true);
}

AboutSection::~AboutSection()
{
    viewport.setLookAndFeel(nullptr);
}

// --- AboutContent Implementation ---
AboutSection::AboutContent::AboutContent()
{
    addAndMakeVisible(label);
    label.setFont(FontManager::getInterRegular(16.0f));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
    label.setJustificationType(juce::Justification::topLeft);
    label.setMinimumHorizontalScale(1.0f);
}

void AboutSection::AboutContent::setText(const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
}

void AboutSection::AboutContent::resized()
{
    label.setBounds(getLocalBounds());
}

int AboutSection::AboutContent::getRequiredHeight(int width)
{
    juce::AttributedString s;
    s.append(label.getText(), label.getFont(), label.findColour(juce::Label::textColourId));
    
    juce::TextLayout layout;
    layout.createLayout(s, (float)width);
    return (int)layout.getHeight() + 20; // Some bottom padding
}

// --- ModernScrollbarLF Implementation ---
void AboutSection::ModernScrollbarLF::drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
    int x, int y, int width, int height,
    bool isScrollbarVertical, int thumbStartPosition,
    int thumbSize, bool isMouseOver, bool isMouseDown)
{
    if (thumbSize <= 0)
        return;

    auto thumbBounds = isScrollbarVertical ? juce::Rectangle<int>(x, thumbStartPosition, width, thumbSize)
                                           : juce::Rectangle<int>(thumbStartPosition, y, thumbSize, height);

    g.setColour(juce::Colours::white.withAlpha(isMouseOver ? 0.2f : 0.1f));
    g.fillRoundedRectangle(thumbBounds.reduced(3).toFloat(), 2.0f);
}

void AboutSection::lookAndFeelChanged()
{
    scrollbarLF.refreshColours();
    repaint();
}

void AboutSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Modern deep gradient background
    juce::ColourGradient bgGrad(juce::Colour(0xff141416), 0, 0,
                               juce::Colour(0xff080809), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Metallic outer border
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRect(bounds, 1.0f);

    // Horizontal divider
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawLine(60.0f, 120.0f, (float)getWidth() - 60.0f, 120.0f, 1.0f);
}

void AboutSection::resized()
{
    auto bounds = getLocalBounds();

    // --- Header Area (Title and Top-Right Links) ---
    auto headerArea = bounds.removeFromTop(120).reduced(40, 0);

    auto topRightArea = headerArea.removeFromRight(180);
    const int xSize = 24;
    closeButton.setBounds(
        topRightArea.removeFromTop(50)
        .removeFromRight(10)
        .withSizeKeepingCentre(xSize, xSize));

    websiteLink.setBounds(topRightArea.removeFromTop(25));
    githubLink.setBounds(topRightArea.removeFromTop(25));

    titleLabel.setBounds(headerArea.removeFromTop(60));
    versionLabel.setBounds(headerArea.removeFromTop(20));

    // --- Viewport Area ---
    const int dividerY = 120;
    const int verticalPadding = 20; // More padding for modern feel

    auto viewportBounds = juce::Rectangle<int>(
        60,                                    // left margin
        dividerY + verticalPadding,
        getWidth() - 120,                      // symmetric horizontal margins
        getHeight() - (dividerY + verticalPadding) - 20 // bottom margin
    );

    viewport.setBounds(viewportBounds);
    
    // Calculate and set content height
    const int contentWidth = viewport.getMaximumVisibleWidth();
    const int contentHeight = content.getRequiredHeight(contentWidth);
    content.setSize(contentWidth, contentHeight);
}
