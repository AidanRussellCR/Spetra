#include "menu_scene.hpp"

#include <iostream>
#include <memory>
#include <string>

#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/world.hpp"

using spetra::ui::Label;
using spetra::ui::ListMenu;
using spetra::ui::MenuItem;
using spetra::ui::Panel;
using spetra::ui::Rect;

void MenuScene::on_enter(spetra::Window& window) {
    m_theme.set_skin("assets/ui/textbox.png", 6);
    m_theme.set_font("assets/fonts/dialogue.ttf", 12.0f);
    m_theme.line_height = 14;

    m_ui.set_theme(&m_theme);

    build_ui(window);
    m_built = true;
}

void MenuScene::build_ui(spetra::Window& window) {
    int rw = window.render_width();
    int rh = window.render_height();

    // Main panel
    auto panel = std::make_unique<Panel>();
    panel->rect = Rect{16, 16, 120, rh - 32};
    panel->instant(); // no tween on menus; snap open

    m_panel = static_cast<Panel*>(m_ui.add(std::move(panel)));

    auto menu = std::make_unique<ListMenu>();
    menu->rect = Rect{m_panel->rect.x + 10, m_panel->rect.y + 10, 100, 0};

    menu->set_items({
        {"Items",   true,  "items"},
        {"Magic",   false, "magic"},
        {"Status",  true,  "status"},
        {"Save",    true,  "save"},
        {"Close",   true,  "close"},
    });

    m_main_menu = static_cast<ListMenu*>(m_panel->add_child(std::move(menu)));

    // Status label
    auto status = std::make_unique<Label>();
    status->rect.x = m_panel->rect.x + 10;
    status->rect.y = m_panel->rect.y + m_panel->rect.h - 24;
    status->set_wrap_width(100);
    status->set_text("Arrows: move\nZ/Enter: pick\nX: back");

    m_status = static_cast<Label*>(m_panel->add_child(std::move(status)));

    // Submenu panel
    auto sub_panel = std::make_unique<Panel>();
    sub_panel->rect = Rect{150, 16, rw - 166, 90};
    sub_panel->instant();
    sub_panel->visible = false;

    m_sub_panel = static_cast<Panel*>(m_ui.add(std::move(sub_panel)));

    auto sub_menu = std::make_unique<ListMenu>();
    sub_menu->rect = Rect{m_sub_panel->rect.x + 10, m_sub_panel->rect.y + 10, m_sub_panel->rect.w - 20, 0};
    sub_menu->set_items({
        {"Potion",  true, "potion"},
        {"Ether",   true, "ether"},
        {"Antidote", true, "antidote"},
    });

    m_sub_menu = static_cast<ListMenu*>(m_sub_panel->add_child(std::move(sub_menu)));

    // Wiring
    m_main_menu->on_select = [this](int index) {
        const MenuItem* item = m_main_menu->selected_item();
        if (!item) {
            return;
        }
        (void)index;

        if (item->id == "items") {
            // Open the submenu; show it and move focus
            m_sub_panel->visible = true;
            m_ui.push_focus(m_sub_menu);
        }
        else if (item->id == "status") {
            // Read global state written by the map scene
            bool seen = world().variables.flag("demo.seen_intro");
            int pauses = world().variables.get_int("demo.pause_count");
            m_status->set_text("intro: " + std::string(seen ? "yes" : "no") +
            "\npauses: " + std::to_string(pauses));
        }
        else if (item->id == "save") {
            // Prove the store is writable from UI callbacks
            int saves = world().variables.get_int("demo.save_count") + 1;
            world().variables.set_int("demo.save_count", saves);
            m_status->set_text("saved x" + std::to_string(saves));
            std::cout << "[Menu] save_count=" << saves << '\n';
        }
        else if (item->id == "close") {
            m_should_close = true;
        }
    };

    // Cancel on the main menu closes the whole scene
    m_main_menu->on_cancel_action = [this]() {
        m_should_close = true;
    };

    // Choosing an item logs it; cancel backs out to the main menu
    m_sub_menu->on_select = [this](int index) {
        (void)index;
        const MenuItem* item = m_sub_menu->selected_item();
        if (item) {
            m_status->set_text("used " + item->text);
            std::cout << "[Menu] used " << item->text << '\n';
        }
    };

    m_sub_menu->on_cancel_action = [this]() {
        m_sub_panel->visible = false;
        m_ui.pop_focus(); // focus returns to the main menu
    };

    m_ui.set_focus(m_main_menu);
    m_ui.layout();
}

void MenuScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    if (input.was_pressed(SDL_SCANCODE_M)) {
        scene_manager.pop_scene();
        return;
    }

    m_ui.handle_input(input);

    if (m_should_close) {
        scene_manager.pop_scene();
    }
}

void MenuScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)scene_manager;
    m_ui.update(delta_time);
}

void MenuScene::render(spetra::Window& window) {
    window.draw_filled_rect(spetra::Color{0, 0, 0, 120},
                            0, 0, window.render_width(), window.render_height());

    m_ui.layout();
    m_ui.render(window);
}
