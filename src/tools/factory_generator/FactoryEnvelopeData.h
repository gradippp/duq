#pragma once

#include "model/EnvelopeData.h"

/**
    Authoring-only envelope aggregate used by the factory content generator to
    build ValueTrees for the shipped presets. Not used by the runtime plugin,
    which works with ValueTrees / DSPEnvelope directly — hence it lives here
    rather than in the runtime model header.
*/
struct EnvelopeViewState
{
    float zoomX = 1.0f;
    float zoomY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    int gridPower = 4;
};

/**
    The complete data for a single envelope instance.
*/
struct EnvelopeData
{
    juce::String name;
    bool isDisabled = false;

    EnvelopeShape shape;
    EnvelopeControls controls;
    EnvelopeViewState viewState;

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree vt("ENVELOPE");
        vt.setProperty("name", name, nullptr);
        vt.setProperty("disabled", isDisabled, nullptr);

        // Merge controls into the main tree for legacy support / APVTS compatibility
        auto ctrlVT = controls.toValueTree();
        for (int i = 0; i < ctrlVT.getNumProperties(); ++i)
            vt.setProperty(ctrlVT.getPropertyName(i), ctrlVT.getProperty(ctrlVT.getPropertyName(i)), nullptr);

        shape.applyToValueTree(vt);

        // View state
        vt.setProperty("zoomX", viewState.zoomX, nullptr);
        vt.setProperty("zoomY", viewState.zoomY, nullptr);
        vt.setProperty("offsetX", viewState.offsetX, nullptr);
        vt.setProperty("offsetY", viewState.offsetY, nullptr);
        vt.setProperty("gridPower", viewState.gridPower, nullptr);

        return vt;
    }
};
