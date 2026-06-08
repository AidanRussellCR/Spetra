#include "spetra/ui/ui_root.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <utility>

#include "spetra/input.hpp"
#include "spetra/ui/theme.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void UIRoot::set_theme(Theme* theme) {
        m_theme = theme;

        for (auto& widget : m_widgets) {
            widget->set_theme(theme);
        }
    }

    Widget* UIRoot::add(std::unique_ptr<Widget> widget) {
        if (!widget) {
            return nullptr;
        }

        Widget* raw = widget.get();
        raw->set_theme(m_theme);
        m_widgets.push_back(std::move(widget));
        return raw;
    }

    void UIRoot::clear() {
        m_focus_stack.clear();
        m_widgets.clear();
    }

    void UIRoot::set_focus(Widget* widget) {
        m_focus_stack.clear();

        if (widget) {
            m_focus_stack.push_back(widget);
        }
    }

    void UIRoot::push_focus(Widget* widget) {
        if (widget) {
            m_focus_stack.push_back(widget);
        }
    }

    void UIRoot::pop_focus() {
        if (!m_focus_stack.empty()) {
            m_focus_stack.pop_back();
        }
    }

    Widget* UIRoot::focused() const {
        return m_focus_stack.empty() ? nullptr : m_focus_stack.back();
    }

    void UIRoot::layout() {
        for (auto& widget : m_widgets) {
            widget->layout();
        }
    }

    void UIRoot::update(double delta_time) {
        for (auto& widget : m_widgets) {
            widget->update(delta_time);
        }
    }

    void UIRoot::render(Window& window) {
        if (m_theme) {
            m_theme->ensure_loaded(window);
        }

        for (auto& widget : m_widgets) {
            widget->render(window);
        }
    }

    bool UIRoot::handle_input(Input& input) {
        if (m_focus_stack.empty()) {
            return false;
        }

        bool handled = false;

        // Movement (arrows)
        if (input.was_pressed(SDL_SCANCODE_UP)) {
            dispatch_move(Direction::Up);
            handled = true;
        }
        if (input.was_pressed(SDL_SCANCODE_DOWN)) {
            dispatch_move(Direction::Down);
            handled = true;
        }
        if (input.was_pressed(SDL_SCANCODE_LEFT)) {
            dispatch_move(Direction::Left);
            handled = true;
        }
        if (input.was_pressed(SDL_SCANCODE_RIGHT)) {
            dispatch_move(Direction::Right);
            handled = true;
        }

        // Confirm
        if (input.was_pressed(SDL_SCANCODE_RETURN) ||
            input.was_pressed(SDL_SCANCODE_Z) ||
            input.was_pressed(SDL_SCANCODE_SPACE)) {
            handled = dispatch_confirm() || handled;
        }

        // Cancel
        if (input.was_pressed(SDL_SCANCODE_X) ||
            input.was_pressed(SDL_SCANCODE_BACKSPACE)) {
            handled = dispatch_cancel() || handled;
        }

        return handled;
    }

    bool UIRoot::handle_mouse(Input& input, Window& window) {
        if (!input.mouse_was_pressed(SDL_BUTTON_LEFT)) {
            return false;
        }

        float rx = 0.0f;
        float ry = 0.0f;

        if (!SDL_RenderCoordinatesFromWindow(window.renderer(),
                                             static_cast<float>(input.mouse_x()),
                                             static_cast<float>(input.mouse_y()),
                                             &rx, &ry)) {
            return false;
        }

        int mx = static_cast<int>(rx);
        int my = static_cast<int>(ry);

        // Topmost widget first
        for (std::size_t i = m_widgets.size(); i-- > 0; ) {
            if (m_widgets[i]->on_click(mx, my)) {
                return true;
            }
        }

        return false;
    }

    bool UIRoot::dispatch_confirm() {
        // Bubble down the focus stack until something consumes it
        for (std::size_t i = m_focus_stack.size(); i-- > 0; ) {
            if (m_focus_stack[i]->on_confirm()) {
                return true;
            }
        }

        return false;
    }

    bool UIRoot::dispatch_cancel() {
        for (std::size_t i = m_focus_stack.size(); i-- > 0; ) {
            if (m_focus_stack[i]->on_cancel()) {
                return true;
            }
        }

        return false;
    }

    void UIRoot::dispatch_move(Direction direction) {
        if (Widget* target = focused()) {
            target->on_move(direction);
        }
    }

} // namespace spetra::ui
