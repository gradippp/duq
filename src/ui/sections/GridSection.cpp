#include "GridSection.h"

GridSection::GridSection()
{
    setOpaque(true);
}

void GridSection::setEnvelope(EnvelopeData* newEnvelope)
{
    envelope = newEnvelope;
    rebuildPointComponents();
    repaint();
}

void GridSection::resized()
{
    viewArea = getLocalBounds().reduced(20);
    updatePointPositions();
}

juce::Point<float> GridSection::normalizedToPixel(juce::Point<float> p) const
{
    return {
        viewArea.getX() + p.x * viewArea.getWidth(),
        viewArea.getY() + (1.0f - p.y) * viewArea.getHeight()
    };
}

juce::Point<float> GridSection::pixelToNormalized(juce::Point<float> p) const
{
    float nx = (p.x - viewArea.getX()) / viewArea.getWidth();
    float ny = 1.0f - ((p.y - viewArea.getY()) / viewArea.getHeight());

    return {
        juce::jlimit(0.0f, 1.0f, nx),
        juce::jlimit(0.0f, 1.0f, ny)
    };
}

void GridSection::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.2f));
    g.fillRect(viewArea);

    drawGrid(g);

    if (!envelope)
        return;

    auto& points = envelope->points;

    if (points.size() < 2)
        return;

    juce::Path path;

    auto first = normalizedToPixel({ points[0].x, points[0].y });
    path.startNewSubPath(first);

    for (size_t i = 1; i < points.size(); ++i)
    {
        auto p = normalizedToPixel({ points[i].x, points[i].y });
        path.lineTo(p);
    }

    g.setColour(juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void GridSection::drawGrid(juce::Graphics& g)
{
    const int gridLines = 8; // simple fixed grid for now

    float dx = (float)viewArea.getWidth() / gridLines;
    float dy = (float)viewArea.getHeight() / gridLines;

    g.setColour(juce::Colours::white.withAlpha(0.05f));

    for (int i = 0; i <= gridLines; ++i)
    {
        float x = viewArea.getX() + dx * i;
        float y = viewArea.getY() + dy * i;

        g.drawLine(x, viewArea.getY(), x, viewArea.getBottom());
        g.drawLine(viewArea.getX(), y, viewArea.getRight(), y);
    }
}

void GridSection::rebuildPointComponents()
{
    removeAllChildren();
    pointComponents.clear();

    if (!envelope)
        return;

    for (int i = 0; i < (int)envelope->points.size(); ++i)
    {
        auto comp = std::make_unique<PointComponent>(*this, i);

        comp->onDrag = [this](int index, juce::Point<float> pos)
            {
                if (!envelope)
                    return;

                pos.x = juce::jlimit(0.0f, 1.0f, pos.x);
                pos.y = juce::jlimit(0.0f, 1.0f, pos.y);

                envelope->points[index].x = pos.x;
                envelope->points[index].y = pos.y;

                std::sort(envelope->points.begin(),
                    envelope->points.end(),
                    [](auto& a, auto& b)
                    {
                        return a.x < b.x;
                    });

                updatePointPositions();
                repaint();
            };

        addAndMakeVisible(comp.get());
        pointComponents.push_back(std::move(comp));
    }

    updatePointPositions();
}

void GridSection::updatePointPositions()
{
    if (!envelope)
        return;

    for (int i = 0; i < (int)pointComponents.size(); ++i)
    {
        auto& p = envelope->points[i];
        pointComponents[i]->setNormalizedPosition({ p.x, p.y });
    }
}
