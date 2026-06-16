#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spetra/color.hpp"
#include "spetra/scene.hpp"
#include "spetra/texture.hpp"
#include "spetra/dialogue_box.hpp"
#include "spetra/entity.hpp"
#include "spetra/ui/list_menu.hpp"
#include "spetra/ui/panel.hpp"
#include "spetra/ui/theme.hpp"
#include "spetra/ui/ui_root.hpp"
#include "draw_order.hpp"
#include "event_runner.hpp"
#include "map_data.hpp"

enum class CameraMode {
    FollowPlayer,
    Manual,
    Cutscene
};

class MapScene : public spetra::Scene, public demo::EventHost {
public:
    struct Config {
        MapData map;
        spetra::Color clear_color{0, 0, 0, 255};
    };

public:
    explicit MapScene(const Config& config);

    void on_enter(spetra::Window& window) override;
    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

    // demo::EventHost
    void start_dialogue(const std::vector<spetra::DialogueLine>& lines) override;
    bool is_dialogue_active() const override;
    void start_choice(const std::vector<std::string>& options) override;
    bool is_choice_active() const override;
    int  choice_result() const override;
    void request_change_map(const std::string& map_path, bool has_spawn, float spawn_x, float spawn_y) override;
    void request_battle(const std::string& encounter) override;
    void set_camera_mode(const std::string& mode) override;

private:
    int tile_size() const;
    bool is_valid_tile_size(int size) const;
    bool is_tile_blocked(int tile_x, int tile_y) const;
    void resolve_movement(spetra::Entity& entity, float dx, float dy);
    void update_camera(const spetra::Window& window, double delta_time);

    void draw_tile_layer(spetra::Window& window, const TileLayer& layer, int tileset_columns, int resolved_tile_size, float camera_x, float camera_y);
    void draw_collision_debug(spetra::Window& window, int resolved_tile_size, float camera_x, float camera_y);

    // Trigger firing
    // Tile the player currently occupies
    void player_tile(int& tile_x, int& tile_y) const;

    // Tile directly in front of the player, per Entity::direction
    void facing_tile(int& tile_x, int& tile_y) const;

    bool trigger_available(const Trigger& trigger) const;
    bool try_fire_on_enter(int tile_x, int tile_y);
    bool try_fire_on_interact();
    void build_choice_ui(spetra::Window& window);
    bool events_busy() const; // an event or dialogue is holding player input

private:
    Config m_config;
    spetra::Texture m_tileset;
    bool m_loaded = false;

    spetra::DialogueBox m_dialogue_box;

    spetra::Entity m_player;
    std::string m_player_sprite_path = "assets/sprites/player.png";

    std::vector<demo::DrawItem> m_draw_list;

    // Events
    demo::EventRunner m_events;

    int  m_prev_tile_x = -1;
    int  m_prev_tile_y = -1;
    bool m_tile_initialized = false;

    // Deferred scene transitions
    bool m_pending_change_map = false;
    std::string m_pending_map_path;
    bool m_pending_spawn = false;
    float m_pending_spawn_x = 0.0f;
    float m_pending_spawn_y = 0.0f;
    bool m_pending_battle = false;
    std::string m_pending_encounter;

    // Choice menu
    spetra::ui::Theme m_ui_theme;
    spetra::ui::UIRoot m_ui;
    spetra::ui::Panel* m_choice_panel = nullptr;
    spetra::ui::ListMenu* m_choice_menu = nullptr;
    std::vector<spetra::ui::MenuItem> m_choice_items;
    bool m_choice_pending = false;
    bool m_choice_active = false;
    bool m_choice_ui_built = false;
    int  m_choice_result = -1;

    float m_camera_x = 0.0f;
    float m_camera_y = 0.0f;
    CameraMode m_camera_mode = CameraMode::FollowPlayer;
    float m_camera_smoothing = 0.0f; // easing rate; higher = snappier, 0 = instant
    bool m_camera_initialized = false;
    double m_last_delta_time = 0.0;

    bool m_move_left = false;
    bool m_move_right = false;
    bool m_move_up = false;
    bool m_move_down = false;

    // DEBUG
    bool m_show_collision_debug = false;
};
