#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include "../model/EnvelopeData.h"

//==================================================
class ChangeDoubleMemberAction : public juce::UndoableAction
{
public:
    using MemberPtr = double EnvelopeData::*;

    ChangeDoubleMemberAction(EnvelopeData& d,
        MemberPtr member,
        double newVal)
        : data(d),
        memberPtr(member),
        oldValue(d.*member),
        newValue(newVal)
    {
    }

    bool perform() override
    {
        data.*memberPtr = newValue;
        return true;
    }

    bool undo() override
    {
        data.*memberPtr = oldValue;
        return true;
    }

private:
    EnvelopeData& data;
    MemberPtr memberPtr;
    double oldValue;
    double newValue;
};



class ChangeBoolMemberAction : public juce::UndoableAction
{
public:
    using MemberPtr = bool EnvelopeData::*;

    ChangeBoolMemberAction(EnvelopeData& d,
        MemberPtr member,
        bool newVal)
        : data(d),
        memberPtr(member),
        oldValue(d.*member),
        newValue(newVal)
    {
    }

    bool perform() override
    {
        data.*memberPtr = newValue;
        return true;
    }

    bool undo() override
    {
        data.*memberPtr = oldValue;
        return true;
    }

private:
    EnvelopeData& data;
    MemberPtr memberPtr;
    bool oldValue;
    bool newValue;
};
