#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spetra/color.hpp"
#include "spetra/scene.hpp"
#include "spetra/texture.hpp"
#include "map_data.hpp"

class MapScene : public spetra::Scene {
public:
    struct Config {
        MapData map;
        spetra::Color clear_color{0, 0, 0, 255};
    };

public:
    explicit MapScene(const Config& config);

    void on_enter() override;
    void handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) override;
    void update(double delta_time, spetra::SceneManager& scene_manager) override;
    void render(spetra::Window& window) override;

private:
    int tile_size() const;
    bool is_valid_tile_size(int size) const;
    bool is_tile_blocked(int tile_x, int tile_y) const;

private:
    Config m_config;
    spetra::Texture m_tileset;
    bool m_loaded = false;

    float m_player_x = 0.0f;
    float m_player_y = 0.0f;
    int m_player_size = 12;
    float m_player_speed = 80.0f;
    spetra::Color m_player_color{255, 220, 120, 255};

    bool m_move_left = false;
    bool m_move_right = false;
    bool m_move_up = false;
    bool m_move_down = false;

    // DEBUG
    bool m_show_collision_debug = false;
};
