#include "spetra/filesystem.hpp"

#include <SDL3/SDL.h>
#include <iostream>

namespace spetra {

    std::string get_base_path() {
        const char* base = SDL_GetBasePath();

        if (!base) {
            std::cerr << "SDL_GetBasePath failed: " << SDL_GetError() << '\n';
            return "";
        }

        return std::string(base);
    }

    std::string get_asset_path(const std::string& relative_path) {
        return get_base_path() + relative_path;
    }

}
