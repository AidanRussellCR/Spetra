#include "spetra/window.hpp"

#include <iostream>

namespace spetra {

    Window::~Window() {
        destroy();
    }

    bool Window::create(const std::string& title, int width, int height) {
        m_window = SDL_CreateWindow(title.c_str(), width, height, 0);
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

    bool Window::is_valid() const {
        return m_window != nullptr && m_renderer != nullptr;
    }

} // namespace spetra
