#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace spetra {

    class Texture {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        bool load_from_file(SDL_Renderer* renderer, const std::string& path);
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
