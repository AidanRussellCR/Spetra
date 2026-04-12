#pragma once

#include "spetra/config.hpp"
#include "spetra/window.hpp"

namespace spetra {

    class Game {
    public:
        explicit Game(const AppConfig& config);
        ~Game();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        int run();

    private:
        bool startup();
        void shutdown();
        void process_events();
        void render();

    private:
        AppConfig m_config;
        Window m_window;
        bool m_running = false;
    };

} // namespace spetra
