#include "WorkflowPage.h"

WorkflowPage::WorkflowPage()
    : SettingsPageBase("WORKFLOW & EDITING")
{
    snapSensitivitySlider.setRange(0.0, 0.1, 0.001);
    snapSensitivitySlider.setValue(config->getSnapSensitivity(), juce::dontSendNotification);
    snapSensitivitySlider.addListener(this);
    addAndMakeVisible(snapSensitivitySlider);

    defaultCurveCombo.setLookAndFeel(&comboBoxLNF);
    defaultCurveCombo.addItemList({"Exponential", "Linear", "Logarithmic", "S-Curve", "Step"}, 1);
    defaultCurveCombo.setSelectedItemIndex(config->getDefaultCurve());
    defaultCurveCombo.addListener(this);
    addAndMakeVisible(defaultCurveCombo);

    undoLimitSlider.setRange(10, 500, 1);
    undoLimitSlider.setValue(200, juce::dontSendNotification);
    undoLimitSlider.addListener(this);
    addAndMakeVisible(undoLimitSlider);
}

WorkflowPage::~WorkflowPage() 
{
    defaultCurveCombo.setLookAndFeel(nullptr);
}

void WorkflowPage::paint(juce::Graphics& g)
{
    SettingsPageBase::paint(g);

    drawControlLabel(g, "Snapping Sensitivity", snapSensitivitySlider.getBounds());
    drawControlLabel(g, "Default Curve Type", defaultCurveCombo.getBounds());
    drawControlLabel(g, "Undo History Limit", undoLimitSlider.getBounds());
}

void WorkflowPage::resized()
{
    auto area = getContentArea();
    snapSensitivitySlider.setBounds(area.removeFromTop(30).withWidth(200));
    area.removeFromTop(45);
    defaultCurveCombo.setBounds(area.removeFromTop(30).withWidth(150));
    area.removeFromTop(45);
    undoLimitSlider.setBounds(area.removeFromTop(30).withWidth(200));
}

void WorkflowPage::sliderValueChanged(juce::Slider* s)
{
    if (s == &snapSensitivitySlider)
        config->setSnapSensitivity((float)snapSensitivitySlider.getValue());
}

void WorkflowPage::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &defaultCurveCombo)
        config->setDefaultCurve(defaultCurveCombo.getSelectedItemIndex());
}
