#include "SettingsSection.h"
#include "settings/SettingsPageBase.h"
#include "settings/GeneralPage.h"
#include "settings/WorkflowPage.h"
#include "settings/AudioPage.h"
#include "../utils/IconFactory.h"

void SettingsSection::SidebarButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isMouseDownOnButton)
{
    auto bounds = getLocalBounds().toFloat();
    bool isSelected = getToggleState();

    if (isSelected)
    {
        g.setColour(Theme::Colours::uiSelected.withAlpha(0.2f));
        g.fillRect(bounds);
        g.setColour(Theme::Colours::accent);
        g.fillRect(bounds.removeFromLeft(3.0f));
    }
    else if (isMouseOverButton)
    {
        g.setColour(Theme::Colours::uiHover.withAlpha(0.1f));
        g.fillRect(bounds);
    }

    g.setColour(isSelected ? Theme::Colours::accent : Theme::Colours::textDimmed);
    g.setFont(FontManager::getBarlowBold(13.0f));
    g.drawText(getButtonText(), getLocalBounds().reduced(15, 0), juce::Justification::centredLeft);
}

SettingsSection::SettingsSection()
{
    viewport.setLookAndFeel(&viewportLNF);
    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, false, true, false);
    viewport.setScrollBarThickness(10);

    auto setupIconButton = [](juce::DrawableButton& button, const juce::String& iconName)
    {
        button.setClickingTogglesState(false);
        button.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        button.setColour(juce::DrawableButton::backgroundOnColourId, Theme::Colours::uiHover);

        auto normal = Icons::load(iconName, Theme::Colours::textMain);
        auto over = Icons::load(iconName, Theme::Colours::textMain.withAlpha(0.85f));
        auto down = Icons::load(iconName, Theme::Colours::textMain.withAlpha(0.6f));

        if (normal != nullptr)
            button.setImages(normal.get(), over.get(), down.get(), nullptr);
    };

    setupIconButton(backButton, "close");
    backButton.onClick = [this] { if (onClose) onClose(); };
    addAndMakeVisible(backButton);

    // Create Pages
    pages.push_back(std::make_unique<GeneralPage>());
    pages.push_back(std::make_unique<WorkflowPage>());
    pages.push_back(std::make_unique<AudioPage>());

    // Create Sidebar Buttons
    juce::StringArray names = { "GENERAL", "WORKFLOW", "AUDIO" };
    for (int i = 0; i < names.size(); ++i)
    {
        auto btn = std::make_unique<SidebarButton>(names[i]);
        btn->onClick = [this, i] { setPage(i); };
        addAndMakeVisible(btn.get());
        sidebarButtons.push_back(std::move(btn));
    }

    setPage(0);
}

SettingsSection::~SettingsSection() 
{
    viewport.setLookAndFeel(nullptr);
}

void SettingsSection::setPage(int index)
{
    if (index < 0 || index >= (int)pages.size()) return;

    for (int i = 0; i < (int)pages.size(); ++i)
    {
        sidebarButtons[i]->setToggleState(i == index, juce::dontSendNotification);
    }

    if (index == 1) // Workflow page
    {
        if (auto* workflow = dynamic_cast<WorkflowPage*>(pages[1].get()))
            workflow->updateEnvelopeList();
    }

    activePageIndex = index;
    viewport.setViewedComponent(pages[activePageIndex].get(), false);
    
    // Ensure the new page is correctly sized immediately
    resized(); 
    repaint();
}

void SettingsSection::setProcessor(DuqAudioProcessor* p)
{
    if (auto* workflow = getWorkflowPage())
        workflow->setProcessor(p);
}

WorkflowPage* SettingsSection::getWorkflowPage()
{
    if (pages.size() > 1)
        return dynamic_cast<WorkflowPage*>(pages[1].get());
    return nullptr;
}

void SettingsSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto sidebarArea = bounds.removeFromLeft(160);

    // Sidebar Background
    g.setColour(Theme::Colours::background.brighter(0.02f));
    g.fillRect(sidebarArea);

    // Sidebar Divider
    g.setColour(Theme::Colours::border);
    g.drawLine((float)sidebarArea.getRight(), 0.0f, (float)sidebarArea.getRight(), (float)getHeight(), 1.0f);

    // Section Title in Sidebar
    g.setColour(Theme::Colours::accent.withAlpha(0.8f));
    g.setFont(FontManager::getInterBold(18.0f));
    g.drawText("SETTINGS", sidebarArea.removeFromTop(60).reduced(15, 0), juce::Justification::centredLeft);
}

void SettingsSection::resized()
{
    auto bounds = getLocalBounds();
    auto sidebarArea = bounds.removeFromLeft(160);

    // Navigation Buttons
    sidebarArea.removeFromTop(60); // Skip Title
    for (auto& btn : sidebarButtons)
    {
        btn->setBounds(sidebarArea.removeFromTop(40));
    }

    backButton.setBounds(sidebarArea.getX() + 15, getHeight() - 50, 24, 24);

    viewport.setBounds(bounds);
    
    if (auto* currentPage = pages[activePageIndex].get())
    {
        int width = viewport.getMaximumVisibleWidth();
        
        // Pass 1: Set width and a sufficiently large height to allow absolute layout logic to run
        currentPage->setSize(width, 1500); 
        
        // Pass 2: Now that components are positioned, get the actual bottom and set final size
        if (auto* basePage = dynamic_cast<SettingsPageBase*>(currentPage))
        {
            currentPage->setSize(width, basePage->getRequiredHeight());
        }
    }
}
