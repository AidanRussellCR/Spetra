#include "spetra/ui/theme.hpp"

#include <iostream>

#include "spetra/filesystem.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void Theme::set_font(const std::string& path, float point_size) {
        m_font_path = path;
        m_font_point_size = point_size;
        m_font_loaded = false;
    }

    void Theme::set_skin(const std::string& path, int border_size) {
        m_skin_path = path;
        m_border_size = border_size;
        m_skin_loaded = false;
    }

    bool Theme::ensure_loaded(Window& window) {
        if (!m_skin_loaded && !m_skin_path.empty()) {
            std::string full_path = get_asset_path(m_skin_path);

            if (!m_skin.load_from_file(window.renderer(), full_path)) {
                std::cerr << "Theme: failed to load skin: " << full_path << '\n';
                m_skin_path.clear(); // don't retry every frame
            }
            else {
                m_skin_loaded = true;
            }
        }

        if (!m_font_loaded && !m_font_path.empty()) {
            std::string full_path = get_asset_path(m_font_path);

            if (!m_font.load_from_file(full_path, m_font_point_size)) {
                std::cerr << "Theme: failed to load font: " << full_path << '\n';
                m_font_path.clear(); // don't retry every frame
            }
            else {
                m_font_loaded = true;
            }
        }

        return m_font_loaded;
    }

    Font& Theme::font() {
        return m_font;
    }

    bool Theme::has_font() const {
        return m_font_loaded;
    }

    const Texture& Theme::skin() const {
        return m_skin;
    }

    bool Theme::has_skin() const {
        return m_skin_loaded;
    }

    int Theme::border_size() const {
        return m_border_size;
    }

    float Theme::font_point_size() const {
        return m_font_point_size;
    }

} // namespace spetra::ui
