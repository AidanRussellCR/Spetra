#include "spetra/ui/label.hpp"

#include <SDL3/SDL.h>

#include "spetra/ui/theme.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void Label::set_text(const std::string& text) {
        if (m_text == text) {
            return;
        }

        m_text = text;
        m_dirty = true;
    }

    void Label::set_color(const Color& color) {
        // Avoid a pointless texture rebuild if the color is unchanged
        if (m_color.has_value()) {
            const Color& c = *m_color;
            if (c.r == color.r && c.g == color.g && c.b == color.b && c.a == color.a) {
                return;
            }
        }

        m_color = color;
        m_dirty = true;
    }

    void Label::clear_color() {
        if (m_color.has_value()) {
            m_color.reset();
            m_dirty = true;
        }
    }

    void Label::set_wrap_width(int wrap_width) {
        if (m_wrap_width == wrap_width) {
            return;
        }

        m_wrap_width = wrap_width;
        m_dirty = true;
    }

    void Label::render_self(Window& window) {
        if (m_dirty) {
            rebuild(window);
        }

        if (!m_texture.is_valid()) {
            return;
        }

        SDL_FRect dst{
            static_cast<float>(rect.x),
            static_cast<float>(rect.y),
            static_cast<float>(m_texture.width()),
            static_cast<float>(m_texture.height())
        };

        SDL_RenderTexture(window.renderer(), m_texture.handle(), nullptr, &dst);
    }

    void Label::rebuild(Window& window) {
        m_dirty = false;

        if (!m_theme || !m_theme->has_font() || m_text.empty()) {
            m_texture.destroy();
            return;
        }

        Color color = m_color.value_or(m_theme->text_color);

        m_texture.set_text(
            window.renderer(),
                           m_theme->font(),
                           m_text,
                           color,
                           m_wrap_width
        );

        // Auto-size to the rendered text
        if (m_texture.is_valid()) {
            rect.w = m_texture.width();
            rect.h = m_texture.height();
        }
    }

} // namespace spetra::ui
