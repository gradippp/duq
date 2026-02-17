#include "GridSection.h"

float applyCurve(float t, float curve)
{
    if (curve == 0.0f)
        return t;

    float k = curve * 4.0f; // scale aggression

    if (curve > 0)
        return 1.0f - std::pow(1.0f - t, 1.0f + k);
    else
        return std::pow(t, 1.0f - k);
}

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

void GridSection::deletePoint(int index)
{
    if (!envelope)
        return;

    auto& points = envelope->points;

    const int lastIndex = (int)points.size() - 1;

    // Protect endpoints
    if (index == 0 || index == lastIndex)
        return;

    if (index >= 0 && index < points.size())
    {
        points.erase(points.begin() + index);
        rebuildPointComponents();
        repaint();
    }
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

void GridSection::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!envelope)
        return;

    if (!e.mods.isLeftButtonDown())
        return;

    // If double click landed on a child component, ignore
    if (e.eventComponent != this)
        return;

    auto normalized = pixelToNormalized(e.position);

    auto& points = envelope->points;

    if (normalized.x <= 0.0f || normalized.x >= 1.0f)
        return;

    EnvelopePoint newPoint;
    newPoint.x = normalized.x;
    newPoint.y = normalized.y;
    newPoint.curve = 0.0f;

    auto it = std::lower_bound(points.begin(), points.end(), newPoint.x,
        [](const EnvelopePoint& p, float value)
        {
            return p.x < value;
        });

    points.insert(it, newPoint);

    rebuildPointComponents();
    repaint();
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

        const int resolution = 40;

        for (int s = 0; s <= resolution; ++s)
        {
            float t = (float)s / resolution;

            float shapedT = applyCurve(t, p1.curve);

            float x = juce::jmap(t, p1.x, p2.x);
            float y = juce::jmap(shapedT, p1.y, p2.y);

            auto pixel = normalizedToPixel({ x, y });

            if (i == 0 && s == 0)
                path.startNewSubPath(pixel);
            else
                path.lineTo(pixel);
        }
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
