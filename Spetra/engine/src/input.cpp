#include "spetra/input.hpp"

namespace spetra {

    void Input::begin_frame() {
        m_previous = m_current;
        m_mouse_previous = m_mouse_current;
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
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button < k_mouse_button_count) {
                m_mouse_current[event.button.button] = true;
            }
            m_mouse_x = static_cast<int>(event.button.x);
            m_mouse_y = static_cast<int>(event.button.y);
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (event.button.button < k_mouse_button_count) {
                m_mouse_current[event.button.button] = false;
            }
            m_mouse_x = static_cast<int>(event.button.x);
            m_mouse_y = static_cast<int>(event.button.y);
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            m_mouse_x = static_cast<int>(event.motion.x);
            m_mouse_y = static_cast<int>(event.motion.y);
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

    bool Input::mouse_is_down(int button) const {
        if (button < 0 || button >= k_mouse_button_count) {
            return false;
        }

        return m_mouse_current[button];
    }

    bool Input::mouse_was_pressed(int button) const {
        if (button < 0 || button >= k_mouse_button_count) {
            return false;
        }

        return m_mouse_current[button] && !m_mouse_previous[button];
    }

    bool Input::mouse_was_released(int button) const {
        if (button < 0 || button >= k_mouse_button_count) {
            return false;
        }

        return !m_mouse_current[button] && m_mouse_previous[button];
    }

    int Input::mouse_x() const {
        return m_mouse_x;
    }

    int Input::mouse_y() const {
        return m_mouse_y;
    }

} // namespace spetra
