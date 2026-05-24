#include "spetra/dialogue_box.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>

#include "spetra/filesystem.hpp"
#include "spetra/window.hpp"

namespace spetra {

    void DialogueBox::set_skin_path(const std::string& path, int border_size) {
        m_skin_path = path;
        m_border_size = border_size;
        m_skin_loaded = false;
    }

    void DialogueBox::set_font_path(const std::string& path, float point_size) {
        m_font_path = path;
        m_font_point_size = point_size;
        m_font_loaded = false;
        m_text_dirty = true;
    }

    void DialogueBox::start(const std::vector<DialogueLine>& lines) {
        m_lines = lines;
        m_current_index = 0;
        m_active = !m_lines.empty();
        m_open_amount = 0.0;
        m_text_dirty = true;
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

        m_text_dirty = true;
    }

    void DialogueBox::close() {
        m_active = false;
        m_current_index = 0;
        m_lines.clear();
        m_speaker_texture.destroy();
        m_body_texture.destroy();
    }

    void DialogueBox::update(double delta_time) {
        if (!m_active) {
            return;
        }

        m_open_amount += delta_time * 8.0;
        m_open_amount = std::clamp(m_open_amount, 0.0, 1.0);
    }

    void DialogueBox::render(Window& window) {
        if (!m_active) {
            return;
        }

        if (!ensure_loaded(window)) {
            return;
        }

        if (m_text_dirty) {
            rebuild_textures(window);
        }

        int target_w = window.render_width() - 32;
        int target_h = 58;

        int target_x = (window.render_width() - target_w) / 2;
        int target_y = window.render_height() - target_h - 12;

        int current_w = static_cast<int>(target_w * m_open_amount);
        int current_h = static_cast<int>(target_h * m_open_amount);

        int box_x = target_x + (target_w - current_w) / 2;
        int box_y = target_y + (target_h - current_h) / 2;

        if (current_w <= 0 || current_h <= 0) {
            return;
        }

        if (m_skin_loaded) {
            draw_nine_slice(window, box_x, box_y, current_w, current_h);
        }
        else {
            window.draw_filled_rect(m_fallback_box_color, box_x, box_y, current_w, current_h);
        }

        if (m_open_amount < 1.0) {
            return;
        }

        int padding = 10;
        int text_x = box_x + padding;
        int text_y = box_y + padding;

        if (m_speaker_texture.is_valid()) {
            SDL_FRect speaker_dst{
                static_cast<float>(text_x),
                static_cast<float>(text_y - 2),
                static_cast<float>(m_speaker_texture.width()),
                static_cast<float>(m_speaker_texture.height())
            };

            SDL_RenderTexture(window.renderer(), m_speaker_texture.handle(), nullptr, &speaker_dst);
            text_y += m_speaker_texture.height() + 2;
        }

        if (m_body_texture.is_valid()) {
            SDL_FRect body_dst{
                static_cast<float>(text_x),
                static_cast<float>(text_y),
                static_cast<float>(m_body_texture.width()),
                static_cast<float>(m_body_texture.height())
            };

            SDL_RenderTexture(window.renderer(), m_body_texture.handle(), nullptr, &body_dst);
        }

        // Temporary continue marker; need to make it a sprite later
        window.draw_filled_rect(
            Color{255, 255, 255, 255},
            box_x + current_w - 14,
            box_y + current_h - 12,
            5,
            5
        );
    }

    bool DialogueBox::is_active() const {
        return m_active;
    }

    bool DialogueBox::ensure_loaded(Window& window) {
        if (!m_skin_loaded && !m_skin_path.empty()) {
            std::string full_path = get_asset_path(m_skin_path);

            if (!m_skin.load_from_file(window.renderer(), full_path)) {
                std::cerr << "Failed to load dialogue skin: " << full_path << '\n';
            }
            else {
                m_skin_loaded = true;
            }
        }

        if (!m_font_loaded && !m_font_path.empty()) {
            std::string full_path = get_asset_path(m_font_path);

            if (!m_font.load_from_file(full_path, m_font_point_size)) {
                std::cerr << "Failed to load dialogue font: " << full_path << '\n';
            }
            else {
                m_font_loaded = true;
                m_text_dirty = true;
            }
        }

        return m_font_loaded;
    }

    void DialogueBox::rebuild_textures(Window& window) {
        const DialogueLine* line = current_line();
        if (!line) {
            return;
        }

        int wrap_width = window.render_width() - 52;

        if (!line->speaker.empty()) {
            m_speaker_texture.set_text(
                window.renderer(),
                                       m_font,
                                       line->speaker,
                                       m_name_color,
                                       wrap_width
            );
        }
        else {
            m_speaker_texture.destroy();
        }

        m_body_texture.set_text(
            window.renderer(),
                                m_font,
                                line->text,
                                m_text_color,
                                wrap_width
        );

        m_text_dirty = false;
    }

    void DialogueBox::draw_nine_slice(Window& window, int x, int y, int w, int h) {
        int b = m_border_size;

        int tex_w = m_skin.width();
        int tex_h = m_skin.height();

        int center_src_w = tex_w - b * 2;
        int center_src_h = tex_h - b * 2;

        int center_dst_w = w - b * 2;
        int center_dst_h = h - b * 2;

        if (center_src_w <= 0 || center_src_h <= 0 ||
            center_dst_w <= 0 || center_dst_h <= 0) {
            window.draw_filled_rect(m_fallback_box_color, x, y, w, h);
        return;
            }

            // Corners
            window.draw_texture_region(m_skin, 0, 0, b, b, x, y, b, b);
            window.draw_texture_region(m_skin, tex_w - b, 0, b, b, x + w - b, y, b, b);
            window.draw_texture_region(m_skin, 0, tex_h - b, b, b, x, y + h - b, b, b);
            window.draw_texture_region(m_skin, tex_w - b, tex_h - b, b, b, x + w - b, y + h - b, b, b);

            // Edges
            window.draw_texture_region(m_skin, b, 0, center_src_w, b, x + b, y, center_dst_w, b);
            window.draw_texture_region(m_skin, b, tex_h - b, center_src_w, b, x + b, y + h - b, center_dst_w, b);
            window.draw_texture_region(m_skin, 0, b, b, center_src_h, x, y + b, b, center_dst_h);
            window.draw_texture_region(m_skin, tex_w - b, b, b, center_src_h, x + w - b, y + b, b, center_dst_h);

            // Center
            window.draw_texture_region(m_skin, b, b, center_src_w, center_src_h, x + b, y + b, center_dst_w, center_dst_h);
    }

    const DialogueLine* DialogueBox::current_line() const {
        if (!m_active || m_current_index < 0 ||
            m_current_index >= static_cast<int>(m_lines.size())) {
            return nullptr;
            }

            return &m_lines[m_current_index];
    }

} // namespace spetra
