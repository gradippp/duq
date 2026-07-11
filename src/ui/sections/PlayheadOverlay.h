#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GridBackground.h"
#include "../../PluginProcessor.h"

class PlayheadOverlay : public juce::Component
{
public:
    PlayheadOverlay()
    {
        setInterceptsMouseClicks(false, false);
    }

    void update(const GridViewState& newState, DuqAudioProcessor* p, int envIndex)
    {
        state = newState;
        processor = p;
        currentEnvelopeIndex = envIndex;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        if (processor == nullptr || currentEnvelopeIndex < 0) return;

        auto phases = processor->getActivePhasesForEnvelope(currentEnvelopeIndex);
        auto viewArea = state.viewArea;

        for (size_t pIdx = 0; pIdx < phases.size(); ++pIdx)
        {
            auto phase = phases[pIdx];
            auto pixelX = state.normalizedToPixel({ (float)phase, 0.5f }).x;

            if (pixelX >= viewArea.getX() && pixelX <= viewArea.getRight())
            {
                g.setColour(Theme::get(ThemeManager::playheadGlow));
                g.drawVerticalLine((int)pixelX, (float)viewArea.getY(), (float)viewArea.getBottom());

                g.setColour(Theme::get(ThemeManager::playhead));
                g.drawVerticalLine((int)pixelX, (float)viewArea.getY(), (float)viewArea.getBottom());
            }
        }
    }

private:
    GridViewState state;
    DuqAudioProcessor* processor = nullptr;
    int currentEnvelopeIndex = -1;
};
