#include "spetra/ui/panel.hpp"

#include <algorithm>

#include "spetra/ui/theme.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void Panel::open() {
        m_open_amount = 0.0;
    }

    void Panel::close() {
        m_open_amount = 0.0;
    }

    void Panel::instant() {
        m_open_amount = 1.0;
    }

    bool Panel::is_open_complete() const {
        return m_open_amount >= 1.0;
    }

    double Panel::open_amount() const {
        return m_open_amount;
    }

    void Panel::update(double delta_time) {
        if (m_open_amount < 1.0) {
            m_open_amount += delta_time * open_speed;
            m_open_amount = std::clamp(m_open_amount, 0.0, 1.0);
        }

        update_children(delta_time);
    }

    bool Panel::children_visible() const {
        // Keep contents hidden until box finished expanding
        return is_open_complete();
    }

    void Panel::render_self(Window& window) {
        // Scale from the center as tween runs
        int current_w = static_cast<int>(rect.w * m_open_amount);
        int current_h = static_cast<int>(rect.h * m_open_amount);

        if (current_w <= 0 || current_h <= 0) {
            return;
        }

        int box_x = rect.x + (rect.w - current_w) / 2;
        int box_y = rect.y + (rect.h - current_h) / 2;

        if (m_theme && m_theme->has_skin()) {
            draw_nine_slice(window, box_x, box_y, current_w, current_h);
        }
        else {
            Color fallback = m_theme ? m_theme->panel_fallback
            : Color{20, 20, 40, 230};
            window.draw_filled_rect(fallback, box_x, box_y, current_w, current_h);
        }
    }

    void Panel::draw_nine_slice(Window& window, int x, int y, int w, int h) {
        const Texture& skin = m_theme->skin();
        int b = m_theme->border_size();

        int tex_w = skin.width();
        int tex_h = skin.height();

        int center_src_w = tex_w - b * 2;
        int center_src_h = tex_h - b * 2;

        int center_dst_w = w - b * 2;
        int center_dst_h = h - b * 2;

        if (center_src_w <= 0 || center_src_h <= 0 ||
            center_dst_w <= 0 || center_dst_h <= 0) {
            window.draw_filled_rect(m_theme->panel_fallback, x, y, w, h);
        return;
            }

            // Corners
            window.draw_texture_region(skin, 0, 0, b, b, x, y, b, b);
            window.draw_texture_region(skin, tex_w - b, 0, b, b, x + w - b, y, b, b);
            window.draw_texture_region(skin, 0, tex_h - b, b, b, x, y + h - b, b, b);
            window.draw_texture_region(skin, tex_w - b, tex_h - b, b, b, x + w - b, y + h - b, b, b);

            // Edges
            window.draw_texture_region(skin, b, 0, center_src_w, b, x + b, y, center_dst_w, b);
            window.draw_texture_region(skin, b, tex_h - b, center_src_w, b, x + b, y + h - b, center_dst_w, b);
            window.draw_texture_region(skin, 0, b, b, center_src_h, x, y + b, b, center_dst_h);
            window.draw_texture_region(skin, tex_w - b, b, b, center_src_h, x + w - b, y + b, b, center_dst_h);

            // Center
            window.draw_texture_region(skin, b, b, center_src_w, center_src_h,
                                       x + b, y + b, center_dst_w, center_dst_h);
    }

} // namespace spetra::ui
