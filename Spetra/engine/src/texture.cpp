#include "spetra/texture.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <utility>

namespace spetra {

    Texture::~Texture() {
        destroy();
    }

    Texture::Texture(Texture&& other) noexcept : m_texture(other.m_texture), m_width(other.m_width), m_height(other.m_height) {
        other.m_texture = nullptr;
        other.m_width = 0;
        other.m_height = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            destroy();

            m_texture = other.m_texture;
            m_width = other.m_width;
            m_height = other.m_height;

            other.m_texture = nullptr;
            other.m_width = 0;
            other.m_height = 0;
        }

        return *this;
    }

    bool Texture::load_from_file(SDL_Renderer* renderer, const std::string& path) {
        destroy();

        m_texture = IMG_LoadTexture(renderer, path.c_str());
        if (!m_texture) {
            std::cerr << "IMG_LoadTexture failed: " << SDL_GetError() << '\n';
            return false;
        }

        SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_NEAREST);

        float w = 0.0f;
        float h = 0.0f;
        if (!SDL_GetTextureSize(m_texture, &w, &h)) {
            std::cerr << "SDL_GetTextureSize failed: " << SDL_GetError() << '\n';
            destroy();
            return false;
        }

        m_width = static_cast<int>(w);
        m_height = static_cast<int>(h);

        return true;
    }

    void Texture::destroy() {
        if (m_texture) {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
        }

        m_width = 0;
        m_height = 0;
    }

    SDL_Texture* Texture::handle() const {
        return m_texture;
    }

    int Texture::width() const {
        return m_width;
    }

    int Texture::height() const {
        return m_height;
    }

    bool Texture::is_valid() const {
        return m_texture != nullptr;
    }

} // namespace spetra
