#include "spetra/text.hpp"

#include <iostream>

namespace spetra {

    Font::~Font() {
        destroy();
    }

    bool Font::load_from_file(const std::string& path, float point_size) {
        if (!TTF_Init()) {
            std::cerr << "TTF_Init failed: " << SDL_GetError() << '\n';
            return false;
        }

        destroy();

        m_font = TTF_OpenFont(path.c_str(), point_size);
        if (!m_font) {
            std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << '\n';
            return false;
        }

        return true;
    }

    void Font::destroy() {
        if (m_font) {
            TTF_CloseFont(m_font);
            m_font = nullptr;
        }
    }

    TTF_Font* Font::handle() const {
        return m_font;
    }

    bool Font::is_valid() const {
        return m_font != nullptr;
    }

    TextTexture::~TextTexture() {
        destroy();
    }

    bool TextTexture::set_text(SDL_Renderer* renderer,
                               Font& font,
                               const std::string& text,
                               const Color& color,
                               int wrap_width) {
        destroy();

        if (!renderer || !font.is_valid() || text.empty()) {
            return false;
        }

        SDL_Color fg{color.r, color.g, color.b, color.a};

        SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(
            font.handle(),
            text.c_str(),
            0,
            fg,
            wrap_width
        );

        if (!surface) {
            std::cerr << "TTF_RenderText_Blended_Wrapped failed: " << SDL_GetError() << '\n';
            return false;
        }

        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!m_texture) {
            std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << '\n';
            SDL_DestroySurface(surface);
            return false;
        }

        m_width = surface->w;
        m_height = surface->h;

        SDL_DestroySurface(surface);
        return true;
    }

    void TextTexture::destroy() {
        if (m_texture) {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
        }

        m_width = 0;
        m_height = 0;
    }

    SDL_Texture* TextTexture::handle() const {
        return m_texture;
    }

    int TextTexture::width() const {
        return m_width;
    }

    int TextTexture::height() const {
        return m_height;
    }

    bool TextTexture::is_valid() const {
        return m_texture != nullptr;
    }

} // namespace spetra
