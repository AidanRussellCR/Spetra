#pragma once

#include <memory>

#include "spetra/config.hpp"
#include "spetra/window.hpp"
#include "spetra/input.hpp"
#include "spetra/timer.hpp"
#include "spetra/scene_manager.hpp"

namespace spetra {

    class Scene;

    class Game {
    public:
        explicit Game(const AppConfig& config);
        ~Game();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        void set_starting_scene(std::unique_ptr<Scene> scene);
        int run();

    private:
        bool startup();
        void shutdown();
        void process_events();
        void update();
        void render();

    private:
        AppConfig m_config;
        Window m_window;
        Input m_input;
        Timer m_timer;
        SceneManager m_scene_manager;
        bool m_running = false;
    };

} // namespace spetra
