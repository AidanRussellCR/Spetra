#include "blue_scene.hpp"

#include <memory>

#include "red_scene.hpp"
#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"

BlueScene::BlueScene()
: m_color{20, 24, 60, 255} {
}

void BlueScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    if (input.was_pressed(SDL_SCANCODE_2)) {
        scene_manager.change_scene(std::make_unique<RedScene>());
    }
}

void BlueScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)delta_time;
    (void)scene_manager;
}

void BlueScene::render(spetra::Window& window) {
    window.clear(m_color);
    window.present();
}
