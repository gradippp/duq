#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ConfigManager
{
public:
    ConfigManager()
    {
        juce::PropertiesFile::Options options;
        options.applicationName     = "Duq";
        options.filenameSuffix      = ".settings";
        options.folderName          = "Duq";
        options.storageFormat       = juce::PropertiesFile::storeAsXML;

        properties.setStorageParameters (options);
    }

    juce::PropertiesFile* getProps() const { return properties.getUserSettings(); }

    // --- Helpers for common settings ---
    
    void setWaveformQuality (int quality) { getProps()->setValue ("waveformQuality", quality); }
    int getWaveformQuality() const { return getProps()->getIntValue ("waveformQuality", 1); } // 0: Low, 1: Med, 2: High

    void setShowTooltips (bool show) { getProps()->setValue ("showTooltips", show); }
    bool getShowTooltips() const { return getProps()->getBoolValue ("showTooltips", true); }

    void setSnapSensitivity (float sensitivity) { getProps()->setValue ("snapSensitivity", sensitivity); }
    float getSnapSensitivity() const { return (float)getProps()->getDoubleValue ("snapSensitivity", 0.01); }

    void setDefaultCurve (int type) { getProps()->setValue ("defaultCurve", type); }
    int getDefaultCurve() const { return getProps()->getIntValue ("defaultCurve", 0); }

private:
    mutable juce::ApplicationProperties properties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigManager)
};
