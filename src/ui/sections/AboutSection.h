#pragma once

#include "../utils/GlobalLookAndFeel.h"

class AboutSection : public juce::Component
{
public:
    AboutSection();
    ~AboutSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onClose;

private:
    juce::DrawableButton closeButton{ "close", juce::DrawableButton::ImageFitted };
    juce::Label titleLabel{ "title", "DUQ" };
    juce::Label versionLabel{ "version", "" };
    
    struct ModernScrollbarLF : public GlobalLookAndFeel
    {
        ModernScrollbarLF() { refreshColours(); }

        void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
            int x, int y, int width, int height,
            bool isScrollbarVertical, int thumbStartPosition,
            int thumbSize, bool isMouseOver, bool isMouseDown) override;

        void drawScrollbarButton(juce::Graphics& g, juce::ScrollBar& scrollbar,
            int width, int height, int buttonDirection,
            bool isScrollbarVertical,
            bool isMouseOverButton, bool isMouseDownOnButton) override {}
    } scrollbarLF;

    struct AboutContent : public juce::Component
    {
        AboutContent();
        void setText(const juce::String& text);
        void resized() override;
        int getRequiredHeight(int width);
        juce::Label label;
    } content;

    juce::Viewport viewport;

    juce::HyperlinkButton githubLink{ "GitHub Repository", juce::URL(PROJECT_GITHUB_URL) };
    juce::HyperlinkButton websiteLink{ "Plugin Website", juce::URL(PROJECT_URI) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutSection)
};
