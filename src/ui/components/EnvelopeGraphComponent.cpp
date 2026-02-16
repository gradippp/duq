#include "EnvelopeGraphComponent.h"
#include <cmath>


static constexpr float curveStrength = 0.25f;
static constexpr float curveSensitivity = 0.0025f;

EnvelopeGraphComponent::EnvelopeGraphComponent() {}

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

    // ---- Right click: reset curve ----
    if (e.mods.isRightButtonDown())
    {
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            auto handle = getHandlePosition(i, area);

            if (handle.getDistanceFrom(e.position) < 8.0f)
            {
                points[i].curve = 0.0f;
                repaint();
                return;
            }
        }
    }

    // ---- Anchor hit test ----
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

    // ---- Curve handle hit test ----
    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        auto handle = getHandlePosition(i, area);

        if (handle.getDistanceFrom(e.position) < 8.0f)
        {
            draggedCurveIndex = (int)i;
            draggedPointIndex = -1;
            dragStartCurveValue = points[i].curve;
            return;
        }
    }
}

void EnvelopeGraphComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!currentEnvelope)
        return;

    auto area = getLocalBounds();
    auto& points = currentEnvelope->points;

    // Remove anchor (except first/last)
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

    // Insert new point
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
        const auto& a = points[draggedCurveIndex];
        const auto& b = points[draggedCurveIndex + 1];

        juce::Point<float> p0 = toPixel(a, area);
        juce::Point<float> p3 = toPixel(b, area);

        juce::Point<float> direction
        {
            p3.x - p0.x,
            p3.y - p0.y
        };

        juce::Point<float> normal(-direction.y, direction.x);

        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0.0001f)
        {
            normal.x /= len;
            normal.y /= len;
        }

        auto mouseDownInt = e.getMouseDownPosition();

        juce::Point<float> mouseDown
        {
            (float)mouseDownInt.x,
            (float)mouseDownInt.y
        };

        juce::Point<float> dragVec
        {
            e.position.x - mouseDown.x,
            e.position.y - mouseDown.y
        };


        float projected =
            dragVec.x * normal.x +
            dragVec.y * normal.y;

        points[draggedCurveIndex].curve =
            juce::jlimit(-1.0f, 1.0f,
                dragStartCurveValue + projected * curveSensitivity);

        repaint();
        return;
    }
}

void EnvelopeGraphComponent::mouseUp(const juce::MouseEvent&)
{
    draggedPointIndex = -1;
    draggedCurveIndex = -1;
}

void EnvelopeGraphComponent::drawGrid(
    juce::Graphics& g,
    juce::Rectangle<int> area)
{
    const int verticalLines = 16;
    const int horizontalLines = 8;

    g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));

    for (int i = 1; i < verticalLines; ++i)
    {
        float x = area.getX() +
            area.getWidth() * (float)i / verticalLines;

        g.drawLine(x, (float)area.getY(),
            x, (float)area.getBottom());
    }

    for (int i = 1; i < horizontalLines; ++i)
    {
        float y = area.getY() +
            area.getHeight() * (float)i / horizontalLines;

        g.drawLine((float)area.getX(), y,
            (float)area.getRight(), y);
    }

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

    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        const auto& a = points[i];
        const auto& b = points[i + 1];

        juce::Point<float> p0 = toPixel(a, area);
        juce::Point<float> p3 = toPixel(b, area);

        juce::Point<float> direction
        {
            p3.x - p0.x,
            p3.y - p0.y
        };



        juce::Point<float> p1
        {
            p0.x + direction.x * 0.33f,
            p0.y + direction.y * 0.33f
        };

        juce::Point<float> p2
        {
            p0.x + direction.x * 0.66f,
            p0.y + direction.y * 0.66f
        };

        juce::Point<float> normal(-direction.y, direction.x);

        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0.0001f)
        {
            normal.x /= len;
            normal.y /= len;
        }

        float segmentLength = direction.getDistanceFromOrigin();
        float strength = segmentLength * curveStrength;

        p1.x += normal.x * a.curve * strength;
        p1.y += normal.y * a.curve * strength;

        p2.x -= normal.x * a.curve * strength;
        p2.y -= normal.y * a.curve * strength;

        if (i == 0)
            path.startNewSubPath(p0);

        path.cubicTo(p1, p2, p3);
    }

    g.setColour(juce::Colours::blue);
    g.strokePath(path, juce::PathStrokeType(2.0f));

    // Anchors
    for (const auto& p : points)
    {
        auto pos = toPixel(p, area);
        g.setColour(juce::Colours::white);
        g.fillEllipse(pos.x - 5.0f,
            pos.y - 5.0f,
            10.0f,
            10.0f);
    }

    // Handles
    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        auto handle = getHandlePosition(i, area);

        g.setColour(juce::Colours::orange);
        g.fillEllipse(handle.x - 4.0f,
            handle.y - 4.0f,
            8.0f,
            8.0f);
    }
}

juce::Point<float> EnvelopeGraphComponent::getHandlePosition(
    size_t index,
    juce::Rectangle<int> area) const
{
    const auto& a = currentEnvelope->points[index];
    const auto& b = currentEnvelope->points[index + 1];

    juce::Point<float> p0 = toPixel(a, area);
    juce::Point<float> p3 = toPixel(b, area);

    juce::Point<float> direction
    {
        p3.x - p0.x,
        p3.y - p0.y
    };

    juce::Point<float> mid
    {
        p0.x + direction.x * 0.5f,
        p0.y + direction.y * 0.5f
    };


    juce::Point<float> normal(-direction.y, direction.x);

    float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
    if (len > 0.0001f)
    {
        normal.x /= len;
        normal.y /= len;
    }


    float segmentLength = direction.getDistanceFromOrigin();
    float strength = segmentLength * curveStrength;

    return {
        mid.x + normal.x * a.curve * strength,
        mid.y + normal.y * a.curve * strength
    };
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
