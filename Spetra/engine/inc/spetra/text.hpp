#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

#include "spetra/color.hpp"

namespace spetra {

    // TTF Font
    class Font {
    public:
        Font() = default;
        ~Font();

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;

        bool load_from_file(const std::string& path, float point_size);
        void destroy();

        TTF_Font* handle() const;
        bool is_valid() const;

    private:
        TTF_Font* m_font = nullptr;
    };

    // Texture generated from rendered text
    class TextTexture {
    public:
        TextTexture() = default;
        ~TextTexture();

        TextTexture(const TextTexture&) = delete;
        TextTexture& operator=(const TextTexture&) = delete;

        bool set_text(SDL_Renderer* renderer,
                      Font& font,
                      const std::string& text,
                      const Color& color,
                      int wrap_width);

        void destroy();

        SDL_Texture* handle() const;
        int width() const;
        int height() const;
        bool is_valid() const;

    private:
        SDL_Texture* m_texture = nullptr;
        int m_width = 0;
        int m_height = 0;
    };

} // namespace spetra
