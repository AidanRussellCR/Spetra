#pragma once

#include "spetra/scene.hpp"
#include "spetra/ui/list_menu.hpp"
#include "spetra/ui/panel.hpp"
#include "spetra/ui/theme.hpp"
#include "spetra/ui/ui_root.hpp"

// A pause menu built entirely from UI framework widgets, pushed onto the scene stack
// Demonstrates Panel + ListMenu + Label, cursor navigation, a submenu via the focus stack, theming, and reading/writing global state
class MenuScene : public spetra::Scene {
public:
    void on_enter(spetra::Window& window) override;
    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

    bool blocks_update_below() const override { return true; } // freeze the map
    bool blocks_render_below() const override { return false; } // show it through

private:
    void build_ui(spetra::Window& window);

    spetra::ui::Theme m_theme;
    spetra::ui::UIRoot m_ui;

    spetra::ui::Panel* m_panel = nullptr;
    spetra::ui::ListMenu* m_main_menu = nullptr;
    spetra::ui::Panel* m_sub_panel = nullptr;
    spetra::ui::ListMenu* m_sub_menu = nullptr;
    spetra::ui::Label* m_status = nullptr;

    bool m_should_close = false;
    bool m_built = false;
};
