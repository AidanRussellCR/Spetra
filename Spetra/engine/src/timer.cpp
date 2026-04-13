#include "spetra/timer.hpp"

#include <SDL3/SDL.h>

namespace spetra {

    void Timer::start() {
        m_last_counter = SDL_GetTicks();
        m_delta_seconds = 0.0;
        m_elapsed_seconds = 0.0;
        m_started = true;
    }

    void Timer::tick() {
        if (!m_started) {
            start();
            return;
        }

        unsigned long long current_counter = SDL_GetTicks();
        unsigned long long delta_ms = current_counter - m_last_counter;
        m_last_counter = current_counter;

        m_delta_seconds = static_cast<double>(delta_ms) / 1000.0;
        m_elapsed_seconds += m_delta_seconds;
    }

    double Timer::delta_seconds() const {
        return m_delta_seconds;
    }

    double Timer::elapsed_seconds() const {
        return m_elapsed_seconds;
    }

} // namespace spetra
