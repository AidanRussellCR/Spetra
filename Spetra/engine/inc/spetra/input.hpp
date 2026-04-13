#pragma once

#include <SDL3/SDL.h>
#include <array>

namespace spetra {

    class Input {
    public:
        void begin_frame();
        void process_event(const SDL_Event& event);

        bool is_down(SDL_Scancode key) const;
        bool was_pressed(SDL_Scancode key) const;
        bool was_released(SDL_Scancode key) const;

    private:
        std::array<bool, SDL_SCANCODE_COUNT> m_current{};
        std::array<bool, SDL_SCANCODE_COUNT> m_previous{};
    };

} // namespace spetra
