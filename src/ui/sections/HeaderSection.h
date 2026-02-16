#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HeaderSection : public juce::Component
{
public:
    HeaderSection();
    ~HeaderSection() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setVersionString(const juce::String& version);
    void setProjectURI(const juce::String& uri);

private:
    juce::String versionString{ PROJECT_VERSION };
    juce::String projectURI{ PROJECT_URI };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderSection)
};
