#include "pause_overlay_scene.hpp"

#include <iostream>

#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/world.hpp"

void PauseOverlayScene::on_enter(spetra::Window& window) {
    (void)window;

    m_intro_seen = world().variables.flag("demo.seen_intro");
    m_pause_count = world().variables.get_int("demo.pause_count") + 1;
    world().variables.set_int("demo.pause_count", m_pause_count);

    std::cout << "[Pause] opened  seen_intro=" << (m_intro_seen ? "true" : "false")
    << "  pause_count=" << m_pause_count << '\n';
}

void PauseOverlayScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    if (input.was_pressed(SDL_SCANCODE_P) || input.was_pressed(SDL_SCANCODE_RETURN)) {
        scene_manager.pop_scene();
    }
}

void PauseOverlayScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)delta_time;
    (void)scene_manager;
}

void PauseOverlayScene::render(spetra::Window& window) {
    int w = window.render_width();
    int h = window.render_height();

    // Full screen dim
    window.draw_filled_rect(spetra::Color{0, 0, 0, 150}, 0, 0, w, h);

    // Centered panel
    int pw = w / 2;
    int ph = h / 3;
    int px = (w - pw) / 2;
    int py = (h - ph) / 2;
    window.draw_filled_rect(spetra::Color{30, 32, 48, 235}, px, py, pw, ph);

    // Accent bar green once the intro has been seen, red before
    spetra::Color accent = m_intro_seen
    ? spetra::Color{120, 200, 120, 255}
    : spetra::Color{200, 120, 120, 255};
    window.draw_filled_rect(accent, px + 6, py + 6, pw - 12, 8);
}
