#pragma once

#include "spetra/color.hpp"
#include "spetra/scene.hpp"

class BlueScene : public spetra::Scene {
public:
    BlueScene();

    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

private:
    spetra::Color m_color;
};
