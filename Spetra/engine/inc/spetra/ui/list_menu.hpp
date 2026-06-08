#pragma once

#include <functional>
#include <string>
#include <vector>

#include "spetra/ui/label.hpp"
#include "spetra/ui/widget.hpp"

namespace spetra::ui {

    struct MenuItem {
        std::string text;
        bool enabled = true;
        std::string id; // optional stable key for the caller to switch on
    };

    class ListMenu : public Widget {
    public:
        std::function<void(int index)> on_select;
        std::function<void()> on_cancel_action;

        void set_items(const std::vector<MenuItem>& items);
        const std::vector<MenuItem>& items() const { return m_items; }

        void set_selected(int index);
        int selected() const { return m_selected; }
        const MenuItem* selected_item() const;

        // Vertical spacing per row. 0 = use Theme::line_height
        void set_item_height(int height);

        // Horizontal room reserved for the cursor marker
        int cursor_width = 8;

        void layout() override;

        void on_move(Direction direction) override;
        bool on_confirm() override;
        bool on_cancel() override;

    protected:
        void render_self(Window& window) override;
        bool on_click_self(int mx, int my) override;

    private:
        int row_height() const;
        void rebuild_labels();
        void refresh_colors();
        void step_selection(int delta);

        std::vector<MenuItem> m_items;
        std::vector<Label*> m_labels; // non-owning; owned by m_children
        int m_selected = 0;
        int m_item_height = 0;
    };

} // namespace spetra::ui
