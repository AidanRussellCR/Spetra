#pragma once

#include "spetra/color.hpp"
#include "spetra/scene.hpp"

class ColorScene : public spetra::Scene {
public:
    explicit ColorScene(const spetra::Color& color);

    void handle_input(spetra::Input& input) override;
    void update(double delta_time) override;
    void render(spetra::Window& window) override;

private:
    spetra::Color m_color;
};
