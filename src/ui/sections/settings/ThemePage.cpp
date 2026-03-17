#include "ThemePage.h"

ThemePage::ThemePage()
    : SettingsPageBase("UI THEMES & COLOURS")
{
    ThemeManager::getInstance().addChangeListener(this);

    themeCombo.setLookAndFeel(&comboLNF);
    themeCombo.addListener(this);
    addAndMakeVisible(themeCombo);

    importButton.setLookAndFeel(&buttonLNF);
    importButton.addListener(this);
    addAndMakeVisible(importButton);

    exportButton.setLookAndFeel(&buttonLNF);
    exportButton.addListener(this);
    addAndMakeVisible(exportButton);

    resetButton.setLookAndFeel(&buttonLNF);
    resetButton.addListener(this);
    addAndMakeVisible(resetButton);

    addAndMakeVisible(editor);

    refreshThemeList();
}

ThemePage::~ThemePage()
{
    ThemeManager::getInstance().removeChangeListener(this);
    themeCombo.setLookAndFeel(nullptr);
    importButton.setLookAndFeel(nullptr);
    exportButton.setLookAndFeel(nullptr);
    resetButton.setLookAndFeel(nullptr);
}

void ThemePage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    auto area = getLocalBounds().withTrimmedTop(80).reduced(20, 0);
    
    // Horizontal separator between editor and management
    float sepY = (float)(editor.getBottom() + 20);
    
    g.setColour(T_COL(border).withAlpha(0.2f));
    g.drawLine(20.0f, sepY, (float)getWidth() - 20.0f, sepY, 1.0f);

    drawControlLabel(g, "Select Theme", themeCombo.getBounds());
}

void ThemePage::resized()
{
    auto area = getLocalBounds().withTrimmedTop(80).reduced(20, 0);
    
    // 1. Color Editor at the top
    editor.setBounds(area.removeFromTop(450));
    
    area.removeFromTop(60); // spacing for labels and separator
    
    // 2. Management Panel in a row below
    auto controlsRow = area.removeFromTop(120);
    
    int colWidth = 220;
    int spacing = 40;

    auto leftCol = controlsRow.removeFromLeft(colWidth);
    themeCombo.setBounds(leftCol.removeFromTop(30));

    controlsRow.removeFromLeft(spacing);
    
    auto rightCol = controlsRow.removeFromLeft(colWidth);
    importButton.setBounds(rightCol.removeFromTop(30));
    rightCol.removeFromTop(10);
    exportButton.setBounds(rightCol.removeFromTop(30));
    rightCol.removeFromTop(15);
    resetButton.setBounds(rightCol.removeFromTop(30));
}

int ThemePage::getRequiredHeight()
{
    return 800; // Increased height to accommodate vertical stack
}

void ThemePage::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &themeCombo)
    {
        loadSelectedTheme();
    }
}

void ThemePage::buttonClicked(juce::Button* b)
{
    if (b == &importButton) importTheme();
    else if (b == &exportButton) exportTheme();
    else if (b == &resetButton)
    {
        ThemeManager::getInstance().loadDefaultTheme();
        themeCombo.setSelectedId(1, juce::dontSendNotification);
        config->setActiveThemePath("");
    }
}

void ThemePage::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    repaint();
}

void ThemePage::refreshThemeList()
{
    themeCombo.clear(juce::dontSendNotification);
    themeCombo.addItem("Default (Dark)", 1);
    
    juce::File themesDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Duq")
        .getChildFile("Themes");
    
    if (!themesDir.exists()) themesDir.createDirectory();

    auto files = themesDir.findChildFiles(juce::File::findFiles, false, "*.duqtheme");
    int id = 2;
    for (auto& f : files)
    {
        themeCombo.addItem(f.getFileNameWithoutExtension(), id++);
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
            themeCombo.setText(activeFile.getFileNameWithoutExtension(), juce::dontSendNotification);
            ThemeManager::getInstance().loadThemeFromFile(activeFile);
        }
        else
        {
            themeCombo.setSelectedId(1, juce::dontSendNotification);
        }
    }
}

void ThemePage::loadSelectedTheme()
{
    if (themeCombo.getSelectedId() == 1)
    {
        ThemeManager::getInstance().loadDefaultTheme();
        config->setActiveThemePath("");
        return;
    }

    juce::File themesDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Duq")
        .getChildFile("Themes");
    
    juce::File themeFile = themesDir.getChildFile(themeCombo.getText() + ".duqtheme");
    if (themeFile.existsAsFile())
    {
        ThemeManager::getInstance().loadThemeFromFile(themeFile);
        config->setActiveThemePath(themeFile.getFullPathName());
    }
}

void ThemePage::importTheme()
{
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    auto chooser = std::make_shared<juce::FileChooser>("Import Theme", juce::File(), "*.duqtheme");
    
    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto result = fc.getResult();
        if (result.existsAsFile())
        {
            juce::File themesDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("Duq")
                .getChildFile("Themes");
            
            if (!themesDir.exists()) themesDir.createDirectory();
            
            juce::File target = themesDir.getChildFile(result.getFileName());
            result.copyFileTo(target);
            refreshThemeList();
            themeCombo.setText(target.getFileNameWithoutExtension());
            loadSelectedTheme();
        }
    });
}

void ThemePage::exportTheme()
{
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
    auto chooser = std::make_shared<juce::FileChooser>("Export Current Theme", juce::File(), "*.duqtheme");
    
    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto result = fc.getResult();
        if (result != juce::File())
        {
            if (result.getFileExtension() != ".duqtheme")
                result = result.withFileExtension(".duqtheme");
                
            ThemeManager::getInstance().saveThemeToFile(result);
        }
    });
}
