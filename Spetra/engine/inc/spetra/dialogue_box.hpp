#pragma once

#include <string>
#include <vector>

#include "spetra/color.hpp"
#include "spetra/text.hpp"
#include "spetra/texture.hpp"

namespace spetra {

    class Input;
    class Window;

    // One page of dialogue
    struct DialogueLine {
        std::string speaker;
        std::string text;
    };

    // Dialogue Box
    class DialogueBox {
    public:
        DialogueBox() = default;

        void set_skin_path(const std::string& path, int border_size);
        void set_font_path(const std::string& path, float point_size);

        void start(const std::vector<DialogueLine>& lines);
        void advance();
        void close();

        void update(double delta_time);
        void render(Window& window);

        bool is_active() const;

    private:
        bool ensure_loaded(Window& window);
        void rebuild_textures(Window& window);
        void draw_nine_slice(Window& window, int x, int y, int w, int h);
        const DialogueLine* current_line() const;

    private:
        std::vector<DialogueLine> m_lines;
        int m_current_index = 0;

        std::string m_skin_path;
        std::string m_font_path;

        int m_border_size = 6;
        float m_font_point_size = 12.0f;

        Texture m_skin;
        Font m_font;
        TextTexture m_speaker_texture;
        TextTexture m_body_texture;

        bool m_skin_loaded = false;
        bool m_font_loaded = false;
        bool m_text_dirty = true;
        bool m_active = false;

        double m_open_amount = 0.0;

        Color m_text_color{255, 255, 255, 255};
        Color m_name_color{255, 230, 160, 255};
        Color m_fallback_box_color{20, 20, 40, 230};
    };

} // namespace spetra
