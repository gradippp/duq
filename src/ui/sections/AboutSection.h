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
    
    juce::TextEditor aboutText;
    
    juce::HyperlinkButton githubLink{ "GitHub Repository", juce::URL("https://github.com/agradip/Duq") };
    juce::HyperlinkButton websiteLink{ "Plugin Website", juce::URL("https://agradip.fyi/duq") };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutSection)
};
