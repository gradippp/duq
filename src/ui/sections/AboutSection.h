#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

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
    juce::Label versionLabel{ "version", "VERSION 0.0.0" };
    
    struct ModernScrollbarLF : public juce::LookAndFeel_V4
    {
        ModernScrollbarLF() = default;

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

    juce::HyperlinkButton githubLink{ "GitHub Repository", juce::URL("https://github.com/agradip/Duq") };
    juce::HyperlinkButton websiteLink{ "Plugin Website", juce::URL("https://agradip.fyi/duq") };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutSection)
};
