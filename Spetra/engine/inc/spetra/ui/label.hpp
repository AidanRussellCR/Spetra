#pragma once

#include <optional>
#include <string>

#include "spetra/color.hpp"
#include "spetra/text.hpp"
#include "spetra/ui/widget.hpp"

namespace spetra::ui {

    // Wrapped text; owns a TextTexture and rebuilds it only when something changes (the m_text_dirty pattern, lifted out of DialogueBox)
    // The rect's w/h are updated to the rendered text size after each rebuild, so a Label auto-sizes to its content
    class Label : public Widget {
    public:
        void set_text(const std::string& text);
        const std::string& text() const { return m_text; }

        // Overrides the theme's text color
        // Unset = use Theme::text_color
        void set_color(const Color& color);
        void clear_color();

        // 0 = no wrapping
        void set_wrap_width(int wrap_width);

    protected:
        void render_self(Window& window) override;

    private:
        void rebuild(Window& window);

        std::string m_text;
        std::optional<Color> m_color;
        int m_wrap_width = 0;

        TextTexture m_texture;
        bool m_dirty = true;
    };

} // namespace spetra::ui
