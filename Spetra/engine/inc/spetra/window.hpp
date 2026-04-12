#pragma once

#include <SDL3/SDL.h>
#include <string>
#include "spetra/color.hpp"

namespace spetra {

    class Window {
    public:
        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool create(const std::string& title, int width, int height);
        void destroy();

        void clear(const Color& color);
        void present();

        bool is_valid() const;

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
    };

} // namespace spetra
