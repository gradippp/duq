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
    versionLabel.setFont(FontManager::getJetBrainsMono(14.0f));
    versionLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.4f));
    versionLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(aboutText);
    aboutText.setMultiLine(true);
    aboutText.setReadOnly(true);
    aboutText.setScrollbarsShown(true);
    aboutText.setCaretVisible(false);
    aboutText.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    aboutText.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    aboutText.setColour(juce::TextEditor::textColourId, juce::Colours::white.withAlpha(0.7f));
    aboutText.setFont(FontManager::getInterRegular(16.0f));

    // Load text from BinaryData
    int dataSize = 0;
    const char* data = BinaryData::getNamedResource("about_txt", dataSize);
    if (data != nullptr && dataSize > 0)
    {
        aboutText.setText(juce::String::fromUTF8(data, dataSize));
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
    
    // Subtle inner glow/line at the top
    g.setColour(juce::Colours::white.withAlpha(0.03f));
    g.fillRect(bounds.removeFromTop(1.0f));

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

    // --- Text Area (no padding, directly under divider) ---
// --- Text Area ---
    const int dividerY = 120;
    const int verticalPadding = 5;

    aboutText.setBounds(
        60,                                    // left margin
        dividerY + verticalPadding,            // 5px below divider
        getWidth() - 120,                      // symmetric horizontal margins
        getHeight() - (dividerY + verticalPadding)
    );
}
