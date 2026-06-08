#include "spetra/ui/list_menu.hpp"

#include <memory>

#include "spetra/ui/theme.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void ListMenu::set_items(const std::vector<MenuItem>& items) {
        m_items = items;
        m_selected = 0;

        for (std::size_t i = 0; i < m_items.size(); ++i) {
            if (m_items[i].enabled) {
                m_selected = static_cast<int>(i);
                break;
            }
        }

        rebuild_labels();
        layout();
    }

    void ListMenu::set_selected(int index) {
        if (m_items.empty()) {
            m_selected = 0;
            return;
        }

        if (index < 0 || index >= static_cast<int>(m_items.size())) {
            return;
        }

        m_selected = index;
        refresh_colors();
    }

    const MenuItem* ListMenu::selected_item() const {
        if (m_selected < 0 || m_selected >= static_cast<int>(m_items.size())) {
            return nullptr;
        }

        return &m_items[m_selected];
    }

    void ListMenu::set_item_height(int height) {
        m_item_height = height;
        layout();
    }

    int ListMenu::row_height() const {
        if (m_item_height > 0) {
            return m_item_height;
        }

        return m_theme ? m_theme->line_height : 12;
    }

    void ListMenu::rebuild_labels() {
        m_children.clear();
        m_labels.clear();

        for (const MenuItem& item : m_items) {
            auto label = std::make_unique<Label>();
            label->set_text(item.text);

            Label* raw = label.get();
            add_child(std::move(label)); // add_child propagates the theme
            m_labels.push_back(raw);
        }

        refresh_colors();
    }

    void ListMenu::refresh_colors() {
        if (!m_theme) {
            return;
        }

        for (std::size_t i = 0; i < m_labels.size(); ++i) {
            const MenuItem& item = m_items[i];

            if (!item.enabled) {
                m_labels[i]->set_color(m_theme->disabled_color);
            }
            else if (static_cast<int>(i) == m_selected) {
                m_labels[i]->set_color(m_theme->highlight_color);
            }
            else {
                m_labels[i]->set_color(m_theme->text_color);
            }
        }
    }

    void ListMenu::layout() {
        int h = row_height();

        for (std::size_t i = 0; i < m_labels.size(); ++i) {
            m_labels[i]->rect.x = rect.x + cursor_width;
            m_labels[i]->rect.y = rect.y + static_cast<int>(i) * h;
        }

        // Colors depend on the theme, which may have arrived after set_items
        refresh_colors();
    }

    void ListMenu::step_selection(int delta) {
        if (m_items.empty()) {
            return;
        }

        int count = static_cast<int>(m_items.size());
        int index = m_selected;

        for (int tries = 0; tries < count; ++tries) {
            index = (index + delta % count + count) % count;

            if (m_items[index].enabled) {
                m_selected = index;
                refresh_colors();
                return;
            }
        }
    }

    void ListMenu::on_move(Direction direction) {
        if (direction == Direction::Up) {
            step_selection(-1);
        }
        else if (direction == Direction::Down) {
            step_selection(1);
        }
        // Left/Right are unused here; a grid menu would override them
    }

    bool ListMenu::on_confirm() {
        const MenuItem* item = selected_item();

        if (!item || !item->enabled) {
            return false;
        }

        if (on_select) {
            on_select(m_selected);
        }

        return true;
    }

    bool ListMenu::on_cancel() {
        if (on_cancel_action) {
            on_cancel_action();
            return true;
        }

        return false;
    }

    bool ListMenu::on_click_self(int mx, int my) {
        int h = row_height();
        if (h <= 0 || m_items.empty()) {
            return false;
        }

        int row = (my - rect.y) / h;

        if (row < 0 || row >= static_cast<int>(m_items.size())) {
            return false;
        }

        if (!m_items[row].enabled) {
            return true; // consumed, but does nothing
        }

        (void)mx;
        set_selected(row);
        return on_confirm();
    }

    void ListMenu::render_self(Window& window) {
        if (m_items.empty()) {
            return;
        }

        int h = row_height();

        if (m_selected >= 0 && m_selected < static_cast<int>(m_items.size())) {
            int row_y = rect.y + m_selected * h;

            Color sel_bg = m_theme ? m_theme->selection_bg : Color{60, 60, 90, 200};
            Color cursor = m_theme ? m_theme->cursor_color : Color{255, 255, 255, 255};

            if (rect.w > 0) {
                window.draw_filled_rect(sel_bg, rect.x, row_y, rect.w, h);
            }

            // Small square cursor to the left of the text
            window.draw_filled_rect(cursor, rect.x + 2, row_y + h / 2 - 2, 4, 4);
        }
    }

} // namespace spetra::ui
