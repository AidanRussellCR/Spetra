#pragma once

#include <SDL3/SDL.h>
#include <array>

namespace spetra {

    class Input {
    public:
        void begin_frame();
        void process_event(const SDL_Event& event);

        // Keyboard
        bool is_down(SDL_Scancode key) const;
        bool was_pressed(SDL_Scancode key) const;
        bool was_released(SDL_Scancode key) const;

        // Mouse
        bool mouse_is_down(int button) const;
        bool mouse_was_pressed(int button) const;
        bool mouse_was_released(int button) const;

        int mouse_x() const;
        int mouse_y() const;

    private:
        static constexpr int k_mouse_button_count = 8;

        std::array<bool, SDL_SCANCODE_COUNT> m_current{};
        std::array<bool, SDL_SCANCODE_COUNT> m_previous{};

        std::array<bool, k_mouse_button_count> m_mouse_current{};
        std::array<bool, k_mouse_button_count> m_mouse_previous{};

        int m_mouse_x = 0;
        int m_mouse_y = 0;
    };

} // namespace spetra
