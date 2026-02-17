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

float GridSection::getCurveForSegment(int index) const
{
    return envelope->points[index].curve;
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

    juce::Path path;

    auto& points = envelope->points;

    auto first = normalizedToPixel({ points[0].x, points[0].y });
    path.startNewSubPath(first);

    for (int i = 0; i < (int)points.size() - 1; ++i)
    {
        auto& p1 = points[i];
        auto& p2 = points[i + 1];

        auto start = normalizedToPixel({ p1.x, p1.y });
        auto end = normalizedToPixel({ p2.x, p2.y });

        float midX = (start.x + end.x) * 0.5f;
        float midY = (start.y + end.y) * 0.5f;

        // midpoint in normalized space
        float midXn = (p1.x + p2.x) * 0.5f;
        float midYn = (p1.y + p2.y) * 0.5f;

        // curve in normalized Y
        float controlYn = midYn + p1.curve * 0.25f;

        auto control = normalizedToPixel({ midXn, controlYn });

        path.quadraticTo(control, end);
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
    anchorComponents.clear();

    if (!envelope)
        return;

    auto& points = envelope->points;

    // === Build points ===
    for (int i = 0; i < (int)points.size(); ++i)
    {
        auto comp = std::make_unique<PointComponent>(*this, i);

        comp->onDrag = [this](int index, juce::Point<float> pos)
            {
                if (!envelope)
                    return;

                auto& pts = envelope->points;

                pos.x = juce::jlimit(0.0f, 1.0f, pos.x);
                pos.y = juce::jlimit(0.0f, 1.0f, pos.y);

                const int lastIndex = (int)pts.size() - 1;

                if (index == 0)
                {
                    pts[index].x = 0.0f;
                    pts[index].y = pos.y;
                }
                else if (index == lastIndex)
                {
                    pts[index].x = 1.0f;
                    pts[index].y = pos.y;
                }
                else
                {
                    float leftLimit = pts[index - 1].x + 0.001f;
                    float rightLimit = pts[index + 1].x - 0.001f;

                    pos.x = juce::jlimit(leftLimit, rightLimit, pos.x);

                    pts[index].x = pos.x;
                    pts[index].y = pos.y;
                }

                updatePointPositions();
                repaint();
            };

        addAndMakeVisible(comp.get());
        pointComponents.push_back(std::move(comp));
    }

    // === Build anchors (ONE PER SEGMENT) ===
    for (int i = 0; i < (int)points.size() - 1; ++i)
    {
        auto anchor = std::make_unique<AnchorComponent>(*this, i);

        anchor->onCurveChanged = [this](int segmentIndex, float newCurve)
            {
                if (!envelope)
                    return;

                envelope->points[segmentIndex].curve =
                    juce::jlimit(-1.0f, 1.0f, newCurve);

                repaint();
            };

        addAndMakeVisible(anchor.get());
        anchorComponents.push_back(std::move(anchor));
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

    // Update anchors
    for (int i = 0; i < (int)anchorComponents.size(); ++i)
    {
        auto& p1 = envelope->points[i];
        auto& p2 = envelope->points[i + 1];

        // Midpoint in normalized space
        float midX = (p1.x + p2.x) * 0.5f;
        float midY = (p1.y + p2.y) * 0.5f;

        // Apply curve offset in normalized Y space
        float curveOffset = p1.curve * 0.25f;  // normalized strength

        float controlY = midY + curveOffset;

        anchorComponents[i]->setNormalizedPosition({ midX, controlY });
    }

}
