#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/EnvelopeData.h"
#include "../Globals.h"

class ConfigManager : public juce::ChangeBroadcaster
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
    
    void setWaveformQuality (int quality) { getProps()->setValue ("waveformQuality", quality); sendChangeMessage(); }
    int getWaveformQuality() const { return getProps()->getIntValue ("waveformQuality", 1); } // 0: Low, 1: Med, 2: High

    void setShowTooltips (bool show) { getProps()->setValue ("showTooltips", show); sendChangeMessage(); }
    bool getShowTooltips() const { return getProps()->getBoolValue ("showTooltips", true); }

    void setShowSourceSignal (bool show) { getProps()->setValue ("showSourceSignal", show); sendChangeMessage(); }
    bool getShowSourceSignal() const { return getProps()->getBoolValue ("showSourceSignal", true); }

    void setShowSidechainSignal (bool show) { getProps()->setValue ("showSidechainSignal", show); sendChangeMessage(); }
    bool getShowSidechainSignal() const { return getProps()->getBoolValue ("showSidechainSignal", true); }

    void setSnapSensitivity (float sensitivity) { getProps()->setValue ("snapSensitivity", sensitivity); sendChangeMessage(); }
    float getSnapSensitivity() const { return (float)getProps()->getDoubleValue ("snapSensitivity", 0.01); }

    void setUndoLimit (int limit) { getProps()->setValue ("undoLimit", limit); sendChangeMessage(); }
    int getUndoLimit() const { return getProps()->getIntValue ("undoLimit", 200); }

    void setDefaultCurve (int type) { getProps()->setValue ("defaultCurve", type); sendChangeMessage(); }
    int getDefaultCurve() const { return getProps()->getIntValue ("defaultCurve", Defaults::curveType); }

    void setDefaultTension (float tension) { getProps()->setValue ("defaultTension", tension); sendChangeMessage(); }
    float getDefaultTension() const { return (float)getProps()->getDoubleValue ("defaultTension", Defaults::curve); }

    void setDefaultRate (float rate) { getProps()->setValue ("defaultRate", rate); sendChangeMessage(); }
    float getDefaultRate() const { return (float)getProps()->getDoubleValue ("defaultRate", Defaults::rate); }

    void setDefaultDepth (float depth) { getProps()->setValue ("defaultDepth", depth); sendChangeMessage(); }
    float getDefaultDepth() const { return (float)getProps()->getDoubleValue ("defaultDepth", Defaults::depth); }

    void setDefaultSmooth (float smooth) { getProps()->setValue ("defaultSmooth", smooth); sendChangeMessage(); }
    float getDefaultSmooth() const { return (float)getProps()->getDoubleValue ("defaultSmooth", Defaults::smooth); }

    void setDefaultTriggerNote (int note) { getProps()->setValue ("defaultTriggerNote", note); sendChangeMessage(); }
    int getDefaultTriggerNote() const { return getProps()->getIntValue ("defaultTriggerNote", Defaults::triggerNote); }

    void setDefaultPoints (const juce::String& xml) { getProps()->setValue ("defaultPoints", xml); sendChangeMessage(); }
    juce::String getDefaultPoints() const { return getProps()->getValue ("defaultPoints", ""); }

    void setActiveThemePath (const juce::String& path) { getProps()->setValue ("activeThemePath", path); sendChangeMessage(); }
    juce::String getActiveThemePath() const { return getProps()->getValue ("activeThemePath", ""); }

    void setMidiOctaveOffset (int offset) { getProps()->setValue ("midiOctaveOffset", offset); sendChangeMessage(); }
    int getMidiOctaveOffset() const { return getProps()->getIntValue ("midiOctaveOffset", 0); }

    // --- High-level Struct Helpers ---

    EnvelopeControls getDefaultControls() const
    {
        EnvelopeControls c;
        c.rate = getDefaultRate();
        c.depth = getDefaultDepth();
        c.smooth = getDefaultSmooth();
        c.triggerNote = getDefaultTriggerNote();
        c.rateIsFrequencyMode = true; // Default
        return c;
    }

    void setDefaultControls (const EnvelopeControls& c)
    {
        setDefaultRate ((float)c.rate);
        setDefaultDepth (c.depth);
        setDefaultSmooth (c.smooth);
        setDefaultTriggerNote (c.triggerNote);
    }

    EnvelopeShape getDefaultShape() const
    {
        auto xmlStr = getDefaultPoints();
        if (xmlStr.isEmpty()) 
        {
            // Default linear ramp if nothing saved
            EnvelopeShape s;
            s.addPoint(0.0f, 0.0f);
            s.addPoint(1.0f, 1.0f);
            return s;
        }

        if (auto xml = juce::XmlDocument::parse(xmlStr))
            return EnvelopeShape::fromValueTree(juce::ValueTree::fromXml(*xml));

        return {};
    }

    void setDefaultShape (const EnvelopeShape& s)
    {
        if (auto xml = s.toValueTree().createXml())
            setDefaultPoints (xml->toString());
    }

private:
    mutable juce::ApplicationProperties properties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigManager)
};
