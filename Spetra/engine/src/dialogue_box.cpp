#include "spetra/dialogue_box.hpp"

#include <memory>

#include "spetra/window.hpp"

namespace spetra {

    namespace {
        // Layout constants
        constexpr int k_box_margin_x = 16; // (render_width - 32) wide
        constexpr int k_box_height = 58;
        constexpr int k_box_margin_bottom = 12;
        constexpr int k_padding = 10;
        constexpr int k_wrap_inset = 52; // wrap width = render_width - 52
        constexpr int k_speaker_gap = 2;
    }

    DialogueBox::DialogueBox() {
        m_theme = &m_own_theme;

        m_own_theme.text_color = Color{255, 255, 255, 255};
        m_own_theme.name_color = Color{255, 230, 160, 255};
        m_own_theme.panel_fallback = Color{20, 20, 40, 230};

        m_panel.open_speed = 8.0;
        m_panel.set_theme(m_theme);

        auto speaker = std::make_unique<ui::Label>();
        auto body = std::make_unique<ui::Label>();

        m_speaker_label = static_cast<ui::Label*>(m_panel.add_child(std::move(speaker)));
        m_body_label = static_cast<ui::Label*>(m_panel.add_child(std::move(body)));
    }

    void DialogueBox::set_skin_path(const std::string& path, int border_size) {
        m_own_theme.set_skin(path, border_size);
    }

    void DialogueBox::set_font_path(const std::string& path, float point_size) {
        m_own_theme.set_font(path, point_size);
    }

    void DialogueBox::set_theme(ui::Theme& theme) {
        m_theme = &theme;
        m_panel.set_theme(m_theme); // propagate to the labels
    }

    ui::Theme& DialogueBox::active_theme() {
        return *m_theme;
    }

    void DialogueBox::start(const std::vector<DialogueLine>& lines) {
        m_lines = lines;
        m_current_index = 0;
        m_active = !m_lines.empty();

        if (m_active) {
            m_panel.open(); // restart the tween
            apply_current_line();
        }
    }

    void DialogueBox::advance() {
        if (!m_active) {
            return;
        }

        ++m_current_index;

        if (m_current_index >= static_cast<int>(m_lines.size())) {
            close();
            return;
        }

        apply_current_line();
    }

    void DialogueBox::close() {
        m_active = false;
        m_current_index = 0;
        m_lines.clear();

        m_speaker_label->set_text("");
        m_body_label->set_text("");
    }

    void DialogueBox::apply_current_line() {
        const DialogueLine* line = current_line();
        if (!line) {
            return;
        }

        m_speaker_label->set_text(line->speaker);
        m_speaker_label->set_color(active_theme().name_color);

        m_body_label->set_text(line->text);
        m_body_label->set_color(active_theme().text_color);
    }

    void DialogueBox::update(double delta_time) {
        if (!m_active) {
            return;
        }

        m_panel.update(delta_time);
    }

    void DialogueBox::render(Window& window) {
        if (!m_active) {
            return;
        }

        if (!active_theme().ensure_loaded(window)) {
            return;
        }

        int box_w = window.render_width() - k_box_margin_x * 2;
        int box_h = k_box_height;
        int box_x = (window.render_width() - box_w) / 2;
        int box_y = window.render_height() - box_h - k_box_margin_bottom;

        m_panel.rect = ui::Rect{box_x, box_y, box_w, box_h};

        int wrap_width = window.render_width() - k_wrap_inset;

        int text_x = box_x + k_padding;
        int text_y = box_y + k_padding;

        m_speaker_label->set_wrap_width(wrap_width);
        m_speaker_label->rect.x = text_x;
        m_speaker_label->rect.y = text_y - 2;

        int body_y = text_y;
        if (!m_speaker_label->text().empty()) {
            body_y = text_y - 2 + m_speaker_label->rect.h + k_speaker_gap;
        }

        m_body_label->set_wrap_width(wrap_width);
        m_body_label->rect.x = text_x;
        m_body_label->rect.y = body_y;

        // Panel draws the nine-slice, then its children
        m_panel.render(window);

        // Continue marker
        if (m_panel.is_open_complete()) {
            window.draw_filled_rect(
                Color{255, 255, 255, 255},
                box_x + box_w - 14,
                box_y + box_h - 12,
                5,
                5
            );
        }
    }

    bool DialogueBox::is_active() const {
        return m_active;
    }

    const DialogueLine* DialogueBox::current_line() const {
        if (!m_active || m_current_index < 0 ||
            m_current_index >= static_cast<int>(m_lines.size())) {
            return nullptr;
            }

            return &m_lines[m_current_index];
    }

} // namespace spetra
