#include "WorkflowPage.h"
#include "../../utils/MidiUtils.h"
#include "../../../PluginProcessor.h"

WorkflowPage::WorkflowPage()
    : SettingsPageBase("WORKFLOW & EDITING")
{
    // --- Grid & Editing ---
    defaultCurveCombo.addItemList({"Exponential", "Linear", "Logarithmic", "S-Curve", "Step"}, 1);
    defaultCurveCombo.setSelectedItemIndex(config->getDefaultCurve());
    defaultCurveCombo.addListener(this);
    addAndMakeVisible(defaultCurveCombo);

    defaultTensionSlider.setRange(-1.0, 1.0, 0.01);
    defaultTensionSlider.setValue(config->getDefaultTension(), juce::dontSendNotification);
    defaultTensionSlider.addListener(this);
    addAndMakeVisible(defaultTensionSlider);

    undoLimitSlider.setRange(10, 500, 1);
    undoLimitSlider.setValue(200, juce::dontSendNotification);
    undoLimitSlider.addListener(this);
    addAndMakeVisible(undoLimitSlider);

    // --- Envelope Defaults ---
    defaultRateSlider.setRange(0.1, 100.0, 0.1);
    defaultRateSlider.setValue(config->getDefaultRate(), juce::dontSendNotification);
    defaultRateSlider.addListener(this);
    addAndMakeVisible(defaultRateSlider);

    defaultDepthSlider.setRange(0.0, 100.0, 0.1);
    defaultDepthSlider.setValue(config->getDefaultDepth(), juce::dontSendNotification);
    defaultDepthSlider.addListener(this);
    addAndMakeVisible(defaultDepthSlider);

    defaultSmoothSlider.setRange(0.0, 1000.0, 1.0);
    defaultSmoothSlider.setValue(config->getDefaultSmooth(), juce::dontSendNotification);
    defaultSmoothSlider.addListener(this);
    addAndMakeVisible(defaultSmoothSlider);

    // --- Default Shape ---
    currentEnvelopesCombo.setTextWhenNoChoicesAvailable("No Envelopes Found");
    currentEnvelopesCombo.addListener(this);
    addAndMakeVisible(currentEnvelopesCombo);

    importFromBrowserButton.onClick = [this]
    {
        if (onImportFromBrowser) onImportFromBrowser();
    };
    addAndMakeVisible(importFromBrowserButton);

    addAndMakeVisible(shapePreview);
    shapePreview.setShape(config->getDefaultShape());

    updateEnvelopeList();
}

WorkflowPage::~WorkflowPage() 
{
}

void WorkflowPage::setProcessor(DuqAudioProcessor* p)
{
    processor = p;
    updateEnvelopeList();
}

void WorkflowPage::updateEnvelopeList()
{
    currentEnvelopesCombo.clear();
    if (processor == nullptr) return;

    auto envelopes = processor->getEnvelopesTree();
    for (int i = 0; i < envelopes.getNumChildren(); ++i)
    {
        auto env = envelopes.getChild(i);
        currentEnvelopesCombo.addItem(env.getProperty("name").toString(), i + 1);
    }
}

void WorkflowPage::updateShapePreview()
{
    shapePreview.setShape(config->getDefaultShape());
}

void WorkflowPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    auto area = getContentArea();
    
    // Group 1: Editing
    drawControlLabel(g, "Default Curve Type", defaultCurveCombo.getBounds());
    drawControlLabel(g, "Default Tension", defaultTensionSlider.getBounds());
    drawControlLabel(g, "Undo History Limit", undoLimitSlider.getBounds());

    // Group 2: Envelope Defaults
    g.setColour(T_COL(textLabel));
    g.setFont(FontManager::getBarlowBold(14.0f));
    int group2Y = undoLimitSlider.getBottom() + 40;
    g.drawText("ENVELOPE DEFAULTS", 20, group2Y, 200, 30, juce::Justification::centredLeft);
    g.setColour(T_COL(border).withAlpha(0.3f));
    g.drawLine(20, group2Y + 25, getWidth() - 20, group2Y + 25, 1.0f);

    drawControlLabel(g, "Default Rate", defaultRateSlider.getBounds());
    drawControlLabel(g, "Default Depth (%)", defaultDepthSlider.getBounds());
    drawControlLabel(g, "Default Smooth (ms)", defaultSmoothSlider.getBounds());

    // Group 3: Default Shape
    int group3Y = defaultSmoothSlider.getBottom() + 40;
    g.setColour(T_COL(textLabel));
    g.setFont(FontManager::getBarlowBold(14.0f));
    g.drawText("DEFAULT ENVELOPE SHAPE", 20, group3Y, 200, 30, juce::Justification::centredLeft);
    g.setColour(T_COL(border).withAlpha(0.3f));
    g.drawLine(20, group3Y + 25, getWidth() - 20, group3Y + 25, 1.0f);

    drawControlLabel(g, "Set default from current envelope:", currentEnvelopesCombo.getBounds());
}

void WorkflowPage::resized()
{
    const int startY = 80;
    const int rowHeight = 50;
    const int spacingY = 20;
    const int sectionSpacing = 60;

    auto area = getLocalBounds().withTrimmedTop(startY).reduced(20, 0);
    
    // Group 1: Grid & Editing
    // Row 1: Curve and Tension
    auto row1 = area.removeFromTop(rowHeight);
    defaultCurveCombo.setBounds(row1.removeFromLeft(180).reduced(0, 10));
    row1.removeFromLeft(40);
    defaultTensionSlider.setBounds(row1.removeFromLeft(180).reduced(0, 10));
    
    area.removeFromTop(spacingY);

    // Row 2: Undo Limit (Solo)
    auto row2 = area.removeFromTop(rowHeight);
    undoLimitSlider.setBounds(row2.removeFromLeft(180).reduced(0, 10));

    area.removeFromTop(sectionSpacing);

    // Group 2: Envelope Defaults
    // Row 3: Rate and Depth
    auto row3 = area.removeFromTop(rowHeight);
    defaultRateSlider.setBounds(row3.removeFromLeft(180).reduced(0, 10));
    row3.removeFromLeft(40);
    defaultDepthSlider.setBounds(row3.removeFromLeft(180).reduced(0, 10));
    
    area.removeFromTop(spacingY);

    // Row 4: Smooth (Solo)
    auto row4 = area.removeFromTop(rowHeight);
    defaultSmoothSlider.setBounds(row4.removeFromLeft(180).reduced(0, 10));

    area.removeFromTop(sectionSpacing);

    // Group 3: Default Shape
    // Row 5: Current Envelopes and Import Button
    auto row5 = area.removeFromTop(rowHeight);
    currentEnvelopesCombo.setBounds(row5.removeFromLeft(200).reduced(0, 10));
    row5.removeFromLeft(40);
    importFromBrowserButton.setBounds(row5.removeFromLeft(200).reduced(0, 10));

    area.removeFromTop(20);
    shapePreview.setBounds(area.removeFromTop(150).withWidth(440));
}

void WorkflowPage::sliderValueChanged(juce::Slider* s)
{
    if (s == &defaultTensionSlider) config->setDefaultTension((float)s->getValue());
    else if (s == &defaultRateSlider) config->setDefaultRate((float)s->getValue());
    else if (s == &defaultDepthSlider) config->setDefaultDepth((float)s->getValue());
    else if (s == &defaultSmoothSlider) config->setDefaultSmooth((float)s->getValue());
    else if (s == &undoLimitSlider) config->setUndoLimit((int)s->getValue());
}

void WorkflowPage::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &defaultCurveCombo) config->setDefaultCurve(cb->getSelectedItemIndex());
    else if (cb == &currentEnvelopesCombo)
    {
        if (processor == nullptr) return;
        auto envelopes = processor->getEnvelopesTree();
        auto env = envelopes.getChild(cb->getSelectedItemIndex());
        if (env.isValid())
        {
            auto shape = EnvelopeShape::fromValueTree(env);
            config->setDefaultShape(shape);
            shapePreview.setShape(shape);
        }
    }
}

void WorkflowPage::lookAndFeelChanged()
{
    defaultCurveCombo.setColour(juce::ComboBox::textColourId, T_COL(textMain));
    currentEnvelopesCombo.setColour(juce::ComboBox::textColourId, T_COL(textMain));
    repaint();
}
