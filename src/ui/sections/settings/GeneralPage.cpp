#include "GeneralPage.h"
#include "../../../utils/PresetManager.h"

GeneralPage::GeneralPage()
    : SettingsPageBase("GENERAL SETTINGS")
{
    ThemeManager::getInstance().addChangeListener(this);

    // General Settings
    waveformQualityCombo.setLookAndFeel(&comboBoxLNF);
    waveformQualityCombo.addItemList({"Low", "Medium", "High"}, 1);
    waveformQualityCombo.setSelectedItemIndex(config->getWaveformQuality());
    waveformQualityCombo.addListener(this);
    addAndMakeVisible(waveformQualityCombo);

    tooltipsToggle.setToggleState(config->getShowTooltips(), juce::dontSendNotification);
    tooltipsToggle.addListener(this);
    addAndMakeVisible(tooltipsToggle);

    // Theme Settings
    themeCombo.setLookAndFeel(&comboBoxLNF);
    themeCombo.addListener(this);
    addAndMakeVisible(themeCombo);

    importButton.setButtonText("IMPORT");
    importButton.setLookAndFeel(&buttonLNF);
    importButton.addListener(this);
    addAndMakeVisible(importButton);

    exportButton.setButtonText("EXPORT");
    exportButton.setLookAndFeel(&buttonLNF);
    exportButton.addListener(this);
    addAndMakeVisible(exportButton);

    resetButton.setButtonText("RESET");
    resetButton.setLookAndFeel(&buttonLNF);
    resetButton.addListener(this);
    addAndMakeVisible(resetButton);

    addAndMakeVisible(editor);

    refreshThemeList();
}

GeneralPage::~GeneralPage() 
{
    ThemeManager::getInstance().removeChangeListener(this);
    waveformQualityCombo.setLookAndFeel(nullptr);
    themeCombo.setLookAndFeel(nullptr);
    importButton.setLookAndFeel(nullptr);
    exportButton.setLookAndFeel(nullptr);
    resetButton.setLookAndFeel(nullptr);
}

void GeneralPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    drawControlLabel(g, "Waveform Quality", waveformQualityCombo.getBounds());

    // Separator between general and theme settings
    float sepY1 = (float)(tooltipsToggle.getBottom() + 30);
    g.setColour(T_COL(border).withAlpha(0.2f));
    g.drawLine(20.0f, sepY1, (float)getWidth() - 20.0f, sepY1, 1.0f);

    // Theme Section Title
    g.setColour(T_COL(accent).withAlpha(0.8f));
    g.setFont(FontManager::getInterBold(14.0f));
    g.drawText("UI THEMES & COLOURS", 20, (int)sepY1 + 20, 300, 20, juce::Justification::centredLeft);

    drawControlLabel(g, "Select Theme", themeCombo.getBounds());
}

void GeneralPage::resized()
{
    const int startY = 80;
    const int rowHeight = 30;
    const int spacingY = 40;

    auto area = getLocalBounds().withTrimmedTop(startY).reduced(20, 0);
    
    // Row 1: Waveform Quality
    auto row1 = area.removeFromTop(rowHeight);
    waveformQualityCombo.setBounds(row1.removeFromLeft(150));

    area.removeFromTop(spacingY);

    // Row 2: Tooltips
    tooltipsToggle.setBounds(area.removeFromTop(rowHeight));

    // Space before Theme Section
    area.removeFromTop(100); 

    // Theme Selection Row
    auto selectionRow = area.removeFromTop(40);
    int controlHeight = 28;
    
    themeCombo.setBounds(selectionRow.removeFromLeft(130).withSizeKeepingCentre(130, controlHeight));
    
    selectionRow.removeFromLeft(15);
    importButton.setBounds(selectionRow.removeFromLeft(80).withSizeKeepingCentre(80, controlHeight));
    
    selectionRow.removeFromLeft(8);
    exportButton.setBounds(selectionRow.removeFromLeft(80).withSizeKeepingCentre(80, controlHeight));
    
    selectionRow.removeFromLeft(8);
    resetButton.setBounds(selectionRow.removeFromLeft(80).withSizeKeepingCentre(80, controlHeight));

    // Color Editor
    area.removeFromTop(20);
    editor.setBounds(area.removeFromTop(editor.getRequiredHeight()));
}

int GeneralPage::getRequiredHeight()
{
    return 80 + 30 + 40 + 30 + 100 + 40 + 20 + editor.getRequiredHeight() + 40;
}

void GeneralPage::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &waveformQualityCombo)
    {
        config->setWaveformQuality(waveformQualityCombo.getSelectedItemIndex());
    }
    else if (cb == &themeCombo)
    {
        loadSelectedTheme();
    }
}

void GeneralPage::buttonClicked(juce::Button* b)
{
    if (b == &tooltipsToggle)
    {
        config->setShowTooltips(tooltipsToggle.getToggleState());
    }
    else if (b == &importButton) importTheme();
    else if (b == &exportButton) exportTheme();
    else if (b == &resetButton)
    {
        ThemeManager::getInstance().loadDefaultTheme();
        themeCombo.setSelectedId(1, juce::dontSendNotification);
        config->setActiveThemePath("");
    }
}

void GeneralPage::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    repaint();
}

void GeneralPage::refreshThemeList()
{
    themeCombo.clear(juce::dontSendNotification);
    themeCombo.addItem("Default (Dark)", 1);
    
    juce::File themesDir = PresetManager::getThemeDirectory();
    
    auto files = themesDir.findChildFiles(juce::File::findFiles, false, "*." + PresetManager::themeExtension);
    int id = 2;
    for (auto& f : files)
    {
        auto name = f.getFileName();
        if (name.endsWithIgnoreCase("." + PresetManager::themeExtension))
            name = name.dropLastCharacters(PresetManager::themeExtension.length() + 1);

        themeCombo.addItem(name, id++);
    }
    
    juce::String active = config->getActiveThemePath();
    if (active.isEmpty())
    {
        themeCombo.setSelectedId(1, juce::dontSendNotification);
    }
    else
    {
        juce::File activeFile(active);
        if (activeFile.existsAsFile())
        {
            auto name = activeFile.getFileName();
            if (name.endsWithIgnoreCase("." + PresetManager::themeExtension))
                name = name.dropLastCharacters(PresetManager::themeExtension.length() + 1);

            themeCombo.setText(name, juce::dontSendNotification);
            ThemeManager::getInstance().loadThemeFromFile(activeFile);
        }
        else
        {
            themeCombo.setSelectedId(1, juce::dontSendNotification);
        }
    }
}

void GeneralPage::loadSelectedTheme()
{
    if (themeCombo.getSelectedId() == 1)
    {
        ThemeManager::getInstance().loadDefaultTheme();
        config->setActiveThemePath("");
        return;
    }

    juce::File themesDir = PresetManager::getThemeDirectory();
    
    juce::File themeFile = themesDir.getChildFile(themeCombo.getText()).withFileExtension(PresetManager::themeExtension);
    if (themeFile.existsAsFile())
    {
        ThemeManager::getInstance().loadThemeFromFile(themeFile);
        config->setActiveThemePath(themeFile.getFullPathName());
    }
}

void GeneralPage::importTheme()
{
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    auto chooser = std::make_shared<juce::FileChooser>("Import Theme", 
                                                       PresetManager::getThemeDirectory(), 
                                                       "*." + PresetManager::themeExtension);
    
    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto result = fc.getResult();
        if (result.existsAsFile())
        {
            juce::File themesDir = PresetManager::getThemeDirectory();
            
            juce::File target = themesDir.getChildFile(result.getFileName());
            result.copyFileTo(target);
            refreshThemeList();
            
            auto name = target.getFileName();
            if (name.endsWithIgnoreCase("." + PresetManager::themeExtension))
                name = name.dropLastCharacters(PresetManager::themeExtension.length() + 1);
                
            themeCombo.setText(name, juce::dontSendNotification);
            loadSelectedTheme();
        }
    });
}

void GeneralPage::exportTheme()
{
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
    auto chooser = std::make_shared<juce::FileChooser>("Export Current Theme", 
                                                       PresetManager::getThemeDirectory(), 
                                                       "*." + PresetManager::themeExtension);
    
    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto result = fc.getResult();
        if (result != juce::File())
        {
            if (!result.getFileName().endsWithIgnoreCase("." + PresetManager::themeExtension))
                result = result.withFileExtension(PresetManager::themeExtension);
                
            ThemeManager::getInstance().saveThemeToFile(result);
            refreshThemeList();
        }
    });
}
