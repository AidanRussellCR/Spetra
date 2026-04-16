#pragma once

#include "spetra/color.hpp"
#include "spetra/scene.hpp"

class RedScene : public spetra::Scene {
public:
    RedScene();

    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

private:
    spetra::Color m_color;
};
