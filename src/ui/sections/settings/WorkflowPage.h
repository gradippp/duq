#include "SettingsPageBase.h"
#include "../../components/ShapePreviewComponent.h"

class WorkflowPage : public SettingsPageBase,
                     private juce::Slider::Listener,
                     private juce::ComboBox::Listener,
                     public juce::ChangeListener
{
public:
    WorkflowPage();
    ~WorkflowPage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void setProcessor(class DuqAudioProcessor* p);
    void updateEnvelopeList();
    void updateShapePreview();

    std::function<void()> onImportFromBrowser;
    std::function<void(juce::ValueTree)> onShapeChanged;

    bool isWaitingForImport = false;

private:
    void sliderValueChanged(juce::Slider* s) override;
    void comboBoxChanged(juce::ComboBox* cb) override;

    class DuqAudioProcessor* processor = nullptr;

    // --- Grid & Editing ---
    juce::ComboBox defaultCurveCombo;
    juce::Slider defaultTensionSlider;
    juce::Slider undoLimitSlider;
    juce::TextButton clearUndoButton{ "Clear History" };

    // --- Envelope Defaults ---
    juce::Slider defaultRateSlider;
    juce::Slider defaultDepthSlider;
    juce::Slider defaultSmoothSlider;

    // --- Default Shape ---
    juce::ComboBox currentEnvelopesCombo;
    juce::TextButton importFromBrowserButton{ "Import from Preset Browser" };
    ShapePreviewComponent shapePreview;

    juce::SharedResourcePointer<ConfigManager> config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkflowPage)
};
