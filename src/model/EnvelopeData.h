#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

enum class CurveType
{
    Exponential = 0,
    Linear,
    Logarithmic,
    SCurve,
    Step
};

struct EnvelopePoint
{
    float x = 0.0f;
    float y = 0.0f;

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree vt("POINT");
        vt.setProperty("x", x, nullptr);
        vt.setProperty("y", y, nullptr);
        return vt;
    }

    static EnvelopePoint fromValueTree(const juce::ValueTree& vt)
    {
        return { (float)vt.getProperty("x"), (float)vt.getProperty("y") };
    }
};

struct EnvelopeSegment
{
    float curve = 0.5f;
    CurveType type = CurveType::Exponential;

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree vt("SEGMENT");
        vt.setProperty("curve", curve, nullptr);
        vt.setProperty("type", (int)type, nullptr);
        return vt;
    }

    static EnvelopeSegment fromValueTree(const juce::ValueTree& vt)
    {
        return { (float)vt.getProperty("curve"), (CurveType)(int)vt.getProperty("type") };
    }
};

/**
    Represents the geometric shape of an envelope (points and curves).
*/
struct EnvelopeShape
{
    std::vector<EnvelopePoint> points;
    std::vector<EnvelopeSegment> segments;

    void addPoint(float x, float y, float curve = 0.5f, CurveType type = CurveType::Exponential)
    {
        points.push_back({ x, y });
        if (points.size() > 1)
            segments.push_back({ curve, type });
    }

    void applyToValueTree(juce::ValueTree& vt, juce::UndoManager* um = nullptr) const
    {
        juce::ValueTree pointsVT("POINTS");
        for (const auto& p : points) pointsVT.addChild(p.toValueTree(), -1, nullptr);
        vt.addChild(pointsVT, -1, um);

        juce::ValueTree segmentsVT("SEGMENTS");
        for (const auto& s : segments) segmentsVT.addChild(s.toValueTree(), -1, nullptr);
        vt.addChild(segmentsVT, -1, um);
    }

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree vt("SHAPE");
        applyToValueTree(vt);
        return vt;
    }

    static EnvelopeShape fromValueTree(const juce::ValueTree& vt)
    {
        EnvelopeShape shape;
        
        auto pointsVT = vt.getChildWithName("POINTS");
        if (!pointsVT.isValid() && vt.hasType("SHAPE")) pointsVT = vt.getChildWithName("POINTS"); // redundant but for clarity
        
        if (pointsVT.isValid())
        {
            for (int i = 0; i < pointsVT.getNumChildren(); ++i)
                shape.points.push_back(EnvelopePoint::fromValueTree(pointsVT.getChild(i)));
        }

        auto segmentsVT = vt.getChildWithName("SEGMENTS");
        if (segmentsVT.isValid())
        {
            for (int i = 0; i < segmentsVT.getNumChildren(); ++i)
                shape.segments.push_back(EnvelopeSegment::fromValueTree(segmentsVT.getChild(i)));
        }

        return shape;
    }
};

/**
    Represents the playback parameters of an envelope.
*/
struct EnvelopeControls
{
    double rate = 2.0;
    float depth = 100.0f;
    float smooth = 0.0f;
    int triggerNote = 36;
    bool rateIsFrequencyMode = true;

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree vt("CONTROLS");
        vt.setProperty("rate", rate, nullptr);
        vt.setProperty("depth", (double)depth, nullptr);
        vt.setProperty("smooth", (double)smooth, nullptr);
        vt.setProperty("triggerNote", triggerNote, nullptr);
        vt.setProperty("rateIsFrequencyMode", rateIsFrequencyMode, nullptr);
        return vt;
    }
};

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
