#include "EnvelopeGraphComponent.h"

EnvelopeGraphComponent::EnvelopeGraphComponent()
{
}

void EnvelopeGraphComponent::setEnvelope(EnvelopeData* data)
{
    currentEnvelope = data;
    repaint();
}

void EnvelopeGraphComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);

    drawGrid(g, bounds);
    drawEnvelope(g, bounds);
}

void EnvelopeGraphComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!currentEnvelope)
        return;

    auto area = getLocalBounds();
    auto& points = currentEnvelope->points;

    // ---- 1. Right-click curve handle to reset ----
    if (e.mods.isRightButtonDown())
    {
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            auto& a = points[i];
            auto& b = points[i + 1];

            float midX = (a.x + b.x) * 0.5f;
            float midY = (a.y + b.y) * 0.5f;

            auto pixel = toPixel({ midX, midY }, area);

            if (pixel.getDistanceFrom(e.position) < 6.0f)
            {
                points[i].curve = 0.0f;
                repaint();
                return;
            }
        }
    }

    // ---- 2. Anchor hit test ----
    for (size_t i = 0; i < points.size(); ++i)
    {
        auto pos = toPixel(points[i], area);

        if (pos.getDistanceFrom(e.position) < 8.0f)
        {
            draggedPointIndex = (int)i;
            draggedCurveIndex = -1;
            return;
        }
    }

    // ---- 3. Curve handle hit test ----
    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        auto& a = points[i];
        auto& b = points[i + 1];

        float midX = (a.x + b.x) * 0.5f;
        float midY = (a.y + b.y) * 0.5f;

        auto pixel = toPixel({ midX, midY }, area);

        if (pixel.getDistanceFrom(e.position) < 6.0f)
        {
            draggedCurveIndex = (int)i;
            draggedPointIndex = -1;
            return;
        }
    }
}


void EnvelopeGraphComponent::mouseDoubleClick(
    const juce::MouseEvent& e)
{
    if (!currentEnvelope)
        return;

    auto area = getLocalBounds();
    auto& points = currentEnvelope->points;

    // Check anchor hit
    for (size_t i = 1; i < points.size() - 1; ++i)
    {
        auto pos = toPixel(points[i], area);

        if (pos.getDistanceFrom(e.position) < 8.0f)
        {
            points.erase(points.begin() + i);
            repaint();
            return;
        }
    }

    // Otherwise insert
    auto newPoint = toNormalized(e.position, area);

    auto it = std::lower_bound(
        points.begin(),
        points.end(),
        newPoint.x,
        [](const EnvelopePoint& p, float x)
        {
            return p.x < x;
        });

    points.insert(it, newPoint);

    repaint();
}


void EnvelopeGraphComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!currentEnvelope)
        return;

    auto area = getLocalBounds();
    auto& points = currentEnvelope->points;

    // ---- Drag anchor ----
    if (draggedPointIndex >= 0)
    {
        auto newPoint = toNormalized(e.position, area);

        if (draggedPointIndex == 0)
            newPoint.x = 0.0f;
        else if (draggedPointIndex == points.size() - 1)
            newPoint.x = 1.0f;
        else
        {
            float left = points[draggedPointIndex - 1].x + 0.001f;
            float right = points[draggedPointIndex + 1].x - 0.001f;

            newPoint.x = juce::jlimit(left, right, newPoint.x);
        }

        points[draggedPointIndex].x = newPoint.x;
        points[draggedPointIndex].y = newPoint.y;

        repaint();
        return;
    }

    // ---- Drag curve handle ----
    if (draggedCurveIndex >= 0)
    {
        auto normalized = toNormalized(e.position, area);

        float delta =
            normalized.y - points[draggedCurveIndex].y;

        points[draggedCurveIndex].curve =
            juce::jlimit(-1.0f, 1.0f, delta * 2.0f);

        repaint();
        return;
    }
}


void EnvelopeGraphComponent::mouseUp(
    const juce::MouseEvent&)
{
    draggedPointIndex = -1;
}

void EnvelopeGraphComponent::drawGrid(
    juce::Graphics& g,
    juce::Rectangle<int> area)
{
    const int verticalLines = 16;
    const int horizontalLines = 8;

    g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));

    // Vertical
    for (int i = 1; i < verticalLines; ++i)
    {
        float x = area.getX() +
            area.getWidth() *
            (float)i / verticalLines;

        g.drawLine(x,
            (float)area.getY(),
            x,
            (float)area.getBottom());
    }

    // Horizontal
    for (int i = 1; i < horizontalLines; ++i)
    {
        float y = area.getY() +
            area.getHeight() *
            (float)i / horizontalLines;

        g.drawLine((float)area.getX(),
            y,
            (float)area.getRight(),
            y);
    }

    // Border
    g.setColour(juce::Colours::grey);
    g.drawRect(area, 1);
}

void EnvelopeGraphComponent::drawEnvelope(
    juce::Graphics& g,
    juce::Rectangle<int> area)
{
    if (!currentEnvelope)
        return;

    auto& points = currentEnvelope->points;

    if (points.size() < 2)
        return;

    juce::Path path;

    const int resolutionPerSegment = 32;

    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        auto& a = points[i];
        auto& b = points[i + 1];

        for (int step = 0; step <= resolutionPerSegment; ++step)
        {
            float t = (float)step / resolutionPerSegment;

            float shapedT = std::pow(
                t,
                1.0f + a.curve * 4.0f
            );

            float x = juce::jmap(
                t,
                0.0f, 1.0f,
                a.x, b.x
            );

            float y = juce::jmap(
                shapedT,
                0.0f, 1.0f,
                a.y, b.y
            );

            auto pixel = toPixel({ x, y }, area);

            if (i == 0 && step == 0)
                path.startNewSubPath(pixel);
            else
                path.lineTo(pixel);
        }
    }

    g.setColour(juce::Colours::blue);
    g.strokePath(path, juce::PathStrokeType(2.0f));

    // Draw anchors
    for (size_t i = 0; i < points.size(); ++i)
    {
        auto pos = toPixel(points[i], area);

        g.setColour(juce::Colours::white);
        g.fillEllipse(pos.x - 5, pos.y - 5, 10, 10);
    }

    // Draw mini curve handles
    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        auto& a = points[i];
        auto& b = points[i + 1];

        float midX = (a.x + b.x) * 0.5f;
        float midY = (a.y + b.y) * 0.5f;

        auto pixel = toPixel({ midX, midY }, area);

        g.setColour(juce::Colours::orange);
        g.fillEllipse(pixel.x - 4, pixel.y - 4, 8, 8);
    }
}


juce::Point<float> EnvelopeGraphComponent::toPixel(
    const EnvelopePoint& p,
    juce::Rectangle<int> area) const
{
    float x = area.getX() + p.x * area.getWidth();
    float y = area.getBottom() - p.y * area.getHeight();
    return { x, y };
}

EnvelopePoint EnvelopeGraphComponent::toNormalized(
    juce::Point<float> pos,
    juce::Rectangle<int> area) const
{
    float x = (pos.x - area.getX()) / area.getWidth();
    float y = (area.getBottom() - pos.y) / area.getHeight();

    return {
        juce::jlimit(0.0f, 1.0f, x),
        juce::jlimit(0.0f, 1.0f, y)
    };
}

