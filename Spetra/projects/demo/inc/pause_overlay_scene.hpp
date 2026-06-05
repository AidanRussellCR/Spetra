#pragma once

#include "spetra/scene.hpp"
#include "spetra/color.hpp"

// Translucent pause overlay
// Demonstrates: push/pop, the update/render blocking policy, and reading + writing global state from an overlay
// Pops itself on P / Enter
class PauseOverlayScene : public spetra::Scene {
public:
    void on_enter(spetra::Window& window) override;
    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

    bool blocks_update_below() const override { return true; } // freeze the world
    bool blocks_render_below() const override { return false; } // let it show through

private:
    bool m_intro_seen = false;
    int  m_pause_count = 0;
};
