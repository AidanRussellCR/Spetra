#include "red_scene.hpp"

#include <memory>

#include "blue_scene.hpp"
#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"

RedScene::RedScene()
: m_color{70, 20, 20, 255} {
}

void RedScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    if (input.was_pressed(SDL_SCANCODE_1)) {
        scene_manager.change_scene(std::make_unique<BlueScene>());
    }
}

void RedScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)delta_time;
    (void)scene_manager;
}

void RedScene::render(spetra::Window& window) {
    window.clear(m_color);
    window.present();
}
