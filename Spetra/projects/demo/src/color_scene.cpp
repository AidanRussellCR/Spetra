#include "color_scene.hpp"

#include "spetra/input.hpp"
#include "spetra/window.hpp"

ColorScene::ColorScene(const spetra::Color& color)
: m_color(color) {
}

void ColorScene::handle_input(spetra::Input& input) {
    (void)input;
}

void ColorScene::update(double delta_time) {
    (void)delta_time;
}

void ColorScene::render(spetra::Window& window) {
    window.clear(m_color);
    window.present();
}
