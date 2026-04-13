#include "spetra/input.hpp"

namespace spetra {

    void Input::begin_frame() {
        m_previous = m_current;
    }

    void Input::process_event(const SDL_Event& event) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (!event.key.repeat) {
                m_current[event.key.scancode] = true;
            }
        }
        else if (event.type == SDL_EVENT_KEY_UP) {
            m_current[event.key.scancode] = false;
        }
    }

    bool Input::is_down(SDL_Scancode key) const {
        return m_current[key];
    }

    bool Input::was_pressed(SDL_Scancode key) const {
        return m_current[key] && !m_previous[key];
    }

    bool Input::was_released(SDL_Scancode key) const {
        return !m_current[key] && m_previous[key];
    }

} // namespace spetra
