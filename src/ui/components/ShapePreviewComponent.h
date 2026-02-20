#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../sections/GridBackground.h"
#include "../sections/EnvelopePathRenderer.h"
#include "../../model/EnvelopeData.h"

class ShapePreviewComponent : public juce::Component
{
public:
    ShapePreviewComponent()
    {
        addAndMakeVisible(grid);
        addAndMakeVisible(path);
        
        grid.setInterceptsMouseClicks(false, false);
        path.setInterceptsMouseClicks(false, false);
    }

    void setShape(const EnvelopeShape& shape)
    {
        currentShape = shape;
        updateView();
    }

    void resized() override
    {
        grid.setBounds(getLocalBounds());
        path.setBounds(getLocalBounds());
        updateView();
    }

private:
    void updateView()
    {
        GridViewState state;
        state.viewArea = getLocalBounds().reduced(10);
        state.uniformZoom = 1.0f;
        state.offsetX = 0.0f;
        state.offsetY = 0.0f;
        state.gridPower = 3; // Simpler grid for preview

        grid.setViewState(state);
        path.update(state, currentShape.toValueTree(), {}, {}, {}, 0.0f);
    }

    GridBackground grid;
    EnvelopePathRenderer path;
    EnvelopeShape currentShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapePreviewComponent)
};
