#include "spetra/ui/image.hpp"

#include "spetra/window.hpp"

namespace spetra::ui {

    void Image::set_texture(const Texture* texture,
                            int src_x, int src_y,
                            int src_w, int src_h) {
        m_texture = texture;
        m_src_x = src_x;
        m_src_y = src_y;

        if (texture && texture->is_valid()) {
            m_src_w = (src_w > 0) ? src_w : texture->width();
            m_src_h = (src_h > 0) ? src_h : texture->height();
        }
        else {
            m_src_w = src_w;
            m_src_h = src_h;
        }
    }

    void Image::size_to_source() {
        rect.w = m_src_w;
        rect.h = m_src_h;
    }

    void Image::render_self(Window& window) {
        if (!m_texture || !m_texture->is_valid()) {
            return;
        }

        window.draw_texture_region(
            *m_texture,
            m_src_x, m_src_y, m_src_w, m_src_h,
            rect.x, rect.y, rect.w, rect.h
        );
    }

} // namespace spetra::ui
