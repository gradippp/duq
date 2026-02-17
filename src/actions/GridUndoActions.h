#pragma once

class MovePointAction : public juce::UndoableAction
{
public:
    MovePointAction(EnvelopeData& env, int idx,
        EnvelopePoint oldP,
        EnvelopePoint newP)
        : envelope(env), index(idx),
        oldPoint(oldP), newPoint(newP) {
    }

    bool perform() override
    {
        envelope.points[index] = newPoint;
        return true;
    }

    bool undo() override
    {
        envelope.points[index] = oldPoint;
        return true;
    }

    int getSizeInUnits() override { return 1; }

private:
    EnvelopeData& envelope;
    int index;
    EnvelopePoint oldPoint, newPoint;
};


class AddPointAction : public juce::UndoableAction
{
public:
    AddPointAction(EnvelopeData& env, EnvelopePoint p, int idx)
        : envelope(env), point(p), index(idx) {
    }

    bool perform() override
    {
        envelope.points.insert(envelope.points.begin() + index, point);
        return true;
    }

    bool undo() override
    {
        envelope.points.erase(envelope.points.begin() + index);
        return true;
    }

    int getSizeInUnits() override { return 1; }

private:
    EnvelopeData& envelope;
    EnvelopePoint point;
    int index;
};

class DeletePointAction : public juce::UndoableAction
{
public:
    DeletePointAction(EnvelopeData& env, EnvelopePoint p, int idx)
        : envelope(env), point(p), index(idx) {
    }

    bool perform() override
    {
        envelope.points.erase(envelope.points.begin() + index);
        return true;
    }

    bool undo() override
    {
        envelope.points.insert(envelope.points.begin() + index, point);
        return true;
    }

    int getSizeInUnits() override { return 1; }

private:
    EnvelopeData& envelope;
    EnvelopePoint point;
    int index;
};
