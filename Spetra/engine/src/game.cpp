#include "spetra/game.hpp"

#include <SDL3/SDL.h>
#include <iostream>

namespace spetra {

    Game::Game(const AppConfig& config)
    : m_config(config) {
    }

    Game::~Game() {
        shutdown();
    }

    bool Game::startup() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
            return false;
        }

        if (!m_window.create(m_config.title, m_config.width, m_config.height)) {
            SDL_Quit();
            return false;
        }

        m_running = true;
        return true;
    }

    void Game::shutdown() {
        m_window.destroy();
        SDL_Quit();
    }

    void Game::process_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                m_running = false;
            }
        }
    }

    void Game::render() {
        m_window.clear(m_config.clear_color);
        m_window.present();
    }

    int Game::run() {
        if (!startup()) {
            return 1;
        }

        while (m_running) {
            process_events();
            render();
        }

        return 0;
    }

} // namespace spetra
