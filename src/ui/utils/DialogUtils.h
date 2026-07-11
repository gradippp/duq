#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <map>
#include <functional>

/**
    One shared modal text-entry dialog, replacing several hand-rolled
    AlertWindow + enterModalState + delete blocks. The AlertWindow owns its own
    lifetime; field values are extracted before it is destroyed, so callers pass
    a self-contained onAccept (capture a Component::SafePointer for any component
    it touches) and never hold a pointer to the window.
*/
namespace Dialogs
{
    struct Field
    {
        juce::String id;
        juce::String label;
        juce::String initial;
        bool enabled = true;
    };

    inline void showTextEntry(const juce::String& title,
                              const juce::String& message,
                              std::vector<Field> fields,
                              std::function<void(const std::map<juce::String, juce::String>&)> onAccept)
    {
        auto* window = new juce::AlertWindow(title, message, juce::MessageBoxIconType::NoIcon);

        for (const auto& f : fields)
        {
            window->addTextEditor(f.id, f.initial, f.label);
            if (!f.enabled)
                if (auto* ed = window->getTextEditor(f.id))
                    ed->setEnabled(false);
        }

        window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
        window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        window->enterModalState(true, juce::ModalCallbackFunction::create(
            [window, fields, onAccept](int result)
            {
                if (result == 1 && onAccept)
                {
                    std::map<juce::String, juce::String> values;
                    for (const auto& f : fields)
                        if (auto* ed = window->getTextEditor(f.id))
                            values[f.id] = ed->getText();
                    onAccept(values);
                }
                delete window;
            }));
    }
}
