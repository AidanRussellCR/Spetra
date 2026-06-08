#include "spetra/ui/widget.hpp"

#include <utility>

#include "spetra/ui/theme.hpp"

namespace spetra::ui {

    Widget* Widget::add_child(std::unique_ptr<Widget> child) {
        if (!child) {
            return nullptr;
        }

        Widget* raw = child.get();
        raw->set_theme(m_theme); // inherit; propagates into the child's subtree
        m_children.push_back(std::move(child));
        return raw;
    }

    void Widget::set_theme(Theme* theme) {
        m_theme = theme;

        for (auto& child : m_children) {
            child->set_theme(theme);
        }
    }

    void Widget::layout() {
        layout_children();
    }

    void Widget::update(double delta_time) {
        update_children(delta_time);
    }

    void Widget::render(Window& window) {
        if (!visible) {
            return;
        }

        render_self(window);

        if (children_visible()) {
            render_children(window);
        }
    }

    bool Widget::on_click(int mx, int my) {
        if (!visible) {
            return false;
        }

        // Topmost child first (last drawn is on top).
        for (std::size_t i = m_children.size(); i-- > 0; ) {
            if (m_children[i]->on_click(mx, my)) {
                return true;
            }
        }

        if (rect.contains(mx, my)) {
            return on_click_self(mx, my);
        }

        return false;
    }

    void Widget::update_children(double delta_time) {
        for (auto& child : m_children) {
            child->update(delta_time);
        }
    }

    void Widget::render_children(Window& window) {
        for (auto& child : m_children) {
            child->render(window);
        }
    }

    void Widget::layout_children() {
        for (auto& child : m_children) {
            child->layout();
        }
    }

} // namespace spetra::ui
