#include "battle_stub_scene.hpp"

#include <iostream>

#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/world.hpp"

void BattleStubScene::on_enter(spetra::Window& window) {
    (void)window;

    m_battle_count = world().variables.get_int("demo.battle_count") + 1;
    world().variables.set_int("demo.battle_count", m_battle_count);

    std::cout << "[Battle] entered  battle_count=" << m_battle_count << '\n';
}

void BattleStubScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    if (input.was_pressed(SDL_SCANCODE_B) || input.was_pressed(SDL_SCANCODE_RETURN)) {
        // Record an outcome, then leave
        world().variables.set_flag("demo.last_battle_won", true);
        std::cout << "[Battle] won, returning to map\n";
        scene_manager.pop_scene();
    }
}

void BattleStubScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)delta_time;
    (void)scene_manager;
}

void BattleStubScene::render(spetra::Window& window) {
    window.clear(spetra::Color{40, 12, 16, 255});

    int w = window.render_width();
    int h = window.render_height();

    // A simple "arena" so it's visibly distinct from the map
    window.draw_filled_rect(spetra::Color{90, 30, 36, 255}, 16, 16, w - 32, h - 32);
    window.draw_filled_rect(spetra::Color{200, 180, 120, 255}, 24, 24, w - 48, 10);
}
