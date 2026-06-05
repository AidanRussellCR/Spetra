#pragma once

#include "spetra/scene.hpp"
#include "spetra/color.hpp"

class BattleStubScene : public spetra::Scene {
public:
    void on_enter(spetra::Window& window) override;
    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

    bool blocks_update_below() const override { return true; }
    bool blocks_render_below() const override { return true; }

private:
    int m_battle_count = 0;
};
