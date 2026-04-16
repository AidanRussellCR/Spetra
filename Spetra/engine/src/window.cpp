#include "spetra/window.hpp"

#include <iostream>

namespace spetra {

    Window::~Window() {
        destroy();
    }

    bool Window::create(const std::string& title, int window_width, int window_height, int render_width, int render_height) {
        m_window = SDL_CreateWindow(title.c_str(), window_width, window_height, 0);
        if (!m_window) {
            std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
            return false;
        }

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) {
            std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
            destroy();
            return false;
        }

        m_render_width = render_width;
        m_render_height = render_height;

        if (!SDL_SetRenderLogicalPresentation(m_renderer, render_width, render_height, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
            std::cerr << "SDL_SetRenderLogicalPresentation failed: " << SDL_GetError() << '\n';
            destroy();
            return false;
        }

        return true;
    }

    void Window::destroy() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        m_render_width = 0;
        m_render_height = 0;
    }

    void Window::clear(const Color& color) {
        if (!m_renderer) {
            return;
        }

        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(m_renderer);
    }

    void Window::present() {
        if (!m_renderer) {
            return;
        }

        SDL_RenderPresent(m_renderer);
    }

    bool Window::draw_texture_region(const Texture& texture, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h) {
        if (!m_renderer || !texture.is_valid()) {
            return false;
        }

        SDL_FRect src{
            static_cast<float>(src_x),
            static_cast<float>(src_y),
            static_cast<float>(src_w),
            static_cast<float>(src_h)
        };

        SDL_FRect dst{
            static_cast<float>(dst_x),
            static_cast<float>(dst_y),
            static_cast<float>(dst_w),
            static_cast<float>(dst_h)
        };

        return SDL_RenderTexture(m_renderer, texture.handle(), &src, &dst);
    }

    SDL_Renderer* Window::renderer() const {
        return m_renderer;
    }

    int Window::render_width() const {
        return m_render_width;
    }

    int Window::render_height() const {
        return m_render_height;
    }

    bool Window::is_valid() const {
        return m_window != nullptr && m_renderer != nullptr;
    }

} // namespace spetra
