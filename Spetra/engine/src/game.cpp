#include "spetra/game.hpp"

#include <SDL3/SDL.h>
#include <iostream>
#include <memory>

#include "spetra/scene.hpp"

namespace spetra {

    Game::Game(const AppConfig& config)
    : m_config(config) {
    }

    Game::~Game() {
        shutdown();
    }

    void Game::set_starting_scene(std::unique_ptr<Scene> scene) {
        m_scene_manager.change_scene(std::move(scene));
    }

    bool Game::startup() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
            return false;
        }

        if (!m_window.create(m_config.title, m_config.window_width, m_config.window_height, m_config.render_width, m_config.render_height)) {
            SDL_Quit();
            return false;
        }

        m_timer.start();
        m_running = true;
        return true;
    }

    void Game::shutdown() {
        m_window.destroy();
        SDL_Quit();
    }

    void Game::process_events() {
        m_input.begin_frame();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            m_input.process_event(event);

            if (event.type == SDL_EVENT_QUIT) {
                m_running = false;
            }
        }

        m_scene_manager.handle_input(m_input);
    }

    void Game::update() {
        m_timer.tick();

        if (m_input.was_pressed(SDL_SCANCODE_ESCAPE)) {
            m_running = false;
        }

        m_scene_manager.update(m_timer.delta_seconds());
    }

    void Game::render() {
        m_scene_manager.render(m_window);
    }

    int Game::run() {
        if (!startup()) {
            return 1;
        }

        if (!m_scene_manager.has_scene()) {
            std::cerr << "No starting scene set.\n";
            return 1;
        }

        while (m_running) {
            process_events();
            update();
            render();
        }

        return 0;
    }

} // namespace spetra
