#pragma once

#include <string>

#include "spetra/color.hpp"
#include "spetra/text.hpp"
#include "spetra/texture.hpp"

namespace spetra {

    class Window;

    namespace ui {

        // Owns the shared UI resources (font, panel skin) and every color the widgets draw with
        // Swap the Theme to restyle the whole tree: battle packages and the editor do exactly that
        class Theme {
        public:
            Theme() = default;

            Theme(const Theme&) = delete;
            Theme& operator=(const Theme&) = delete;

            // Asset-relative paths (resolved through get_asset_path)
            void set_font(const std::string& path, float point_size);
            void set_skin(const std::string& path, int border_size);

            // Loads whatever isn't loaded yet
            // Returns true if a font is usable
            bool ensure_loaded(Window& window);

            Font& font();
            bool has_font() const;

            const Texture& skin() const;
            bool has_skin() const;

            int border_size() const;
            float font_point_size() const;

            // Colors
            Color text_color{255, 255, 255, 255};
            Color name_color{255, 230, 160, 255}; // speaker names
            Color highlight_color{255, 230, 160, 255}; // selected menu item text
            Color disabled_color{130, 130, 140, 255};
            Color cursor_color{255, 255, 255, 255};
            Color selection_bg{60, 60, 90, 200};
            Color panel_fallback{20, 20, 40, 230}; // when no skin texture loaded
            Color bar_bg{40, 40, 50, 255};
            Color bar_fill{200, 80, 80, 255};

            // Default vertical spacing for list items
            int line_height = 12;

        private:
            std::string m_font_path;
            std::string m_skin_path;

            float m_font_point_size = 12.0f;
            int m_border_size = 6;

            Font m_font;
            Texture m_skin;

            bool m_font_loaded = false;
            bool m_skin_loaded = false;
        };

    } // namespace ui
} // namespace spetra
