#pragma once

#include <memory>
#include <vector>

#include "spetra/ui/widget.hpp"

namespace spetra {

    class Input;
    class Window;

    namespace ui {

        class Theme;

        // Widget tree and routes input into it; a scene holds one UIRoot

        // FOCUS:
        // a stack of widgets; confirm/cancel/move go to the top of the stack
        // Opening a submenu = push_focus(submenu); backing out = pop_focus()
        // Unhandled events bubble down the focus stack

        // INPUT DRIVERS:
        // handle_input(input)         -> cursor/keyboard (the game path)
        // handle_mouse(input, window) -> mouse hit-testing (the editor path)
        // Both operate on the same tree; call whichever suits the context

        // KEY BINDINGS (game path):
        // move    : arrow keys
        // confirm : Enter / Z / Space
        // cancel  : X / Backspace

        class UIRoot {
        public:
            void set_theme(Theme* theme);
            Theme* theme() const { return m_theme; }

            // Takes ownership; returns a raw pointer for wiring
            Widget* add(std::unique_ptr<Widget> widget);
            void clear();

            // Focus management
            void set_focus(Widget* widget); // replaces the whole stack
            void push_focus(Widget* widget); // submenu
            void pop_focus(); // back out
            Widget* focused() const;

            void layout();
            void update(double delta_time);

            // Loads theme resources if needed, then draws the tree
            void render(Window& window);

            // Cursor input; returns true if anything consumed the event
            bool handle_input(Input& input);

            // Mouse input (editor); converts window coords to render (logical) coords, then hit-tests the tree top-down
            bool handle_mouse(Input& input, Window& window);

        private:
            bool dispatch_confirm();
            bool dispatch_cancel();
            void dispatch_move(Direction direction);

            std::vector<std::unique_ptr<Widget>> m_widgets;
            std::vector<Widget*> m_focus_stack; // non-owning
            Theme* m_theme = nullptr;
        };

    } // namespace ui
} // namespace spetra
