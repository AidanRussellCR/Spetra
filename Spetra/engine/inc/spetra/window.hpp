#pragma once

#include <SDL3/SDL.h>
#include <string>

#include "spetra/color.hpp"
#include "spetra/texture.hpp"

namespace spetra {

    class Window {
    public:
        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool create(const std::string& title, int window_width, int window_height, int render_width, int render_height);
        void destroy();

        void clear(const Color& color);
        void present();

        bool draw_filled_rect(const Color& color, int x, int y, int w, int h);

        bool draw_texture_region(const Texture& texture, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);

        SDL_Renderer* renderer() const;

        int render_width() const;
        int render_height() const;

        bool is_valid() const;

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        int m_render_width = 0;
        int m_render_height = 0;
    };

} // namespace spetra
