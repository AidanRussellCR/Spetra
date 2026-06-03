#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spetra/color.hpp"
#include "spetra/scene.hpp"
#include "spetra/texture.hpp"
#include "spetra/dialogue_box.hpp"
#include "spetra/entity.hpp"
#include "map_data.hpp"

enum class CameraMode {
    FollowPlayer,
    Manual,
    Cutscene
};

class MapScene : public spetra::Scene {
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

private:
    int tile_size() const;
    bool is_valid_tile_size(int size) const;
    bool is_tile_blocked(int tile_x, int tile_y) const;
    void resolve_movement(spetra::Entity& entity, float dx, float dy);
    void update_camera(const spetra::Window& window, double delta_time);

private:
    Config m_config;
    spetra::Texture m_tileset;
    bool m_loaded = false;

    spetra::DialogueBox m_dialogue_box;

    spetra::Entity m_player;
    std::string m_player_sprite_path = "assets/sprites/player.png";

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
