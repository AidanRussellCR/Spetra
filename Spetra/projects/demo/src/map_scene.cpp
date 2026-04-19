#include "map_scene.hpp"

#include <algorithm>
#include <iostream>

#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/filesystem.hpp"

MapScene::MapScene(const Config& config)
: m_config(config) {
}

void MapScene::on_enter() {
    int resolved_tile_size = tile_size();
    if (!is_valid_tile_size(resolved_tile_size)) {
        std::cerr << "Invalid tile size. Must be positive and even.\n";
    }

    // Add player around the center of the map
    int map_pixel_width = m_config.map_width * resolved_tile_size;
    int map_pixel_height = m_config.map_height * resolved_tile_size;

    m_player_x = static_cast<float>((map_pixel_width - m_player_size) / 2);
    m_player_y = static_cast<float>((map_pixel_height - m_player_size) / 2);
}

void MapScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    (void)scene_manager;

    m_move_left = input.is_down(SDL_SCANCODE_LEFT) || input.is_down(SDL_SCANCODE_A);
    m_move_right = input.is_down(SDL_SCANCODE_RIGHT) || input.is_down(SDL_SCANCODE_D);
    m_move_up = input.is_down(SDL_SCANCODE_UP) || input.is_down(SDL_SCANCODE_W);
    m_move_down = input.is_down(SDL_SCANCODE_DOWN) || input.is_down(SDL_SCANCODE_S);
}

void MapScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)scene_manager;

    float move_x = 0.0f;
    float move_y = 0.0f;

    if (m_move_left) {
        move_x -= 1.0f;
    }
    if (m_move_right) {
        move_x += 1.0f;
    }
    if (m_move_up) {
        move_y -= 1.0f;
    }
    if (m_move_down) {
        move_y += 1.0f;
    }

    // Normalize diagonals so player isn't too fast
    if (move_x != 0.0f && move_y != 0.0f) {
        constexpr float diagonal_scale = 0.70710678f;
        move_x *= diagonal_scale;
        move_y *= diagonal_scale;
    }

    float dx = move_x * m_player_speed * static_cast<float>(delta_time);
    float dy = move_y * m_player_speed * static_cast<float>(delta_time);

    int resolved_tile_size = tile_size();

    // X Movement
    float new_x = m_player_x + dx;

    int left_tile = static_cast<int>(new_x) / resolved_tile_size;
    int right_tile = static_cast<int>(new_x + m_player_size - 1) / resolved_tile_size;
    int top_tile = static_cast<int>(m_player_y) / resolved_tile_size;
    int bottom_tile = static_cast<int>(m_player_y + m_player_size - 1) / resolved_tile_size;

    if (!is_tile_blocked(left_tile, top_tile) &&
        !is_tile_blocked(left_tile, bottom_tile) &&
        !is_tile_blocked(right_tile, top_tile) &&
        !is_tile_blocked(right_tile, bottom_tile)) {
        m_player_x = new_x;
    }

    // Y Movement
    float new_y = m_player_y + dy;

    left_tile = static_cast<int>(m_player_x) / resolved_tile_size;
    right_tile = static_cast<int>(m_player_x + m_player_size - 1) / resolved_tile_size;
    top_tile = static_cast<int>(new_y) / resolved_tile_size;
    bottom_tile = static_cast<int>(new_y + m_player_size - 1) / resolved_tile_size;

    if (!is_tile_blocked(left_tile, top_tile) &&
        !is_tile_blocked(left_tile, bottom_tile) &&
        !is_tile_blocked(right_tile, top_tile) &&
        !is_tile_blocked(right_tile, bottom_tile)) {
        m_player_y = new_y;
        }

    int map_pixel_width = m_config.map_width * resolved_tile_size;
    int map_pixel_height = m_config.map_height * resolved_tile_size;

    // Clamp player to map bounds
    float max_x = static_cast<float>(std::max(0, map_pixel_width - m_player_size));
    float max_y = static_cast<float>(std::max(0, map_pixel_height - m_player_size));

    m_player_x = std::clamp(m_player_x, 0.0f, max_x);
    m_player_y = std::clamp(m_player_y, 0.0f, max_y);
}

void MapScene::render(spetra::Window& window) {
    window.clear(m_config.clear_color);

    if (!m_loaded) {
        std::string full_path = spetra::get_asset_path(m_config.tileset_path);

        if (!m_tileset.load_from_file(window.renderer(), full_path)) {
            std::cerr << "Failed to load tileset: " << full_path << '\n';
            window.present();
            return;
        }

        std::cout << "Loaded tileset: " << full_path << " (" << m_tileset.width() << "x" << m_tileset.height() << ")\n";

        m_loaded = true;
    }

    int resolved_tile_size = tile_size();
    if (!is_valid_tile_size(resolved_tile_size)) {
        window.present();
        return;
    }

    if (m_config.map_width <= 0 || m_config.map_height <= 0) {
        window.present();
        return;
    }

    if (static_cast<int>(m_config.tiles.size()) != m_config.map_width * m_config.map_height) {
        window.present();
        return;
    }

    int tileset_columns = m_tileset.width() / resolved_tile_size;
    if (tileset_columns <= 0) {
        window.present();
        return;
    }

    int map_pixel_width = m_config.map_width * resolved_tile_size;
    int map_pixel_height = m_config.map_height * resolved_tile_size;

    int camera_x = std::max(0, (map_pixel_width - window.render_width()) / 2);
    int camera_y = std::max(0, (map_pixel_height - window.render_height()) / 2);

    for (int y = 0; y < m_config.map_height; ++y) {
        for (int x = 0; x < m_config.map_width; ++x) {
            int tile_index = m_config.tiles[y * m_config.map_width + x];
            if (tile_index < 0) {
                continue;
            }

            int src_x = (tile_index % tileset_columns) * resolved_tile_size;
            int src_y = (tile_index / tileset_columns) * resolved_tile_size;

            int dst_x = x * resolved_tile_size - camera_x;
            int dst_y = y * resolved_tile_size - camera_y;

            if (dst_x + resolved_tile_size <= 0 || dst_y + resolved_tile_size <= 0) {
                continue;
            }

            if (dst_x >= window.render_width() || dst_y >= window.render_height()) {
                continue;
            }

            window.draw_texture_region(
                m_tileset,
                src_x, src_y, resolved_tile_size, resolved_tile_size,
                dst_x, dst_y, resolved_tile_size, resolved_tile_size
            );
        }
    }

    // Draw player
    int player_screen_x = static_cast<int>(m_player_x) - camera_x;
    int player_screen_y = static_cast<int>(m_player_y) - camera_y;

    window.draw_filled_rect(
        m_player_color,
        player_screen_x,
        player_screen_y,
        m_player_size,
        m_player_size
    );

    window.present();
}

int MapScene::tile_size() const {
    if (m_config.tile_size_override.has_value()) {
        return *m_config.tile_size_override;
    }

    return m_config.default_tile_size;
}

bool MapScene::is_valid_tile_size(int size) const {
    return size > 0 && (size % 2 == 0);
}

bool MapScene::is_tile_blocked(int tile_x, int tile_y) const {
    if (tile_x < 0 || tile_y < 0 ||
        tile_x >= m_config.map_width ||
        tile_y >= m_config.map_height) {
        return true; // treat region outside map as solid
    }

    int index = tile_y * m_config.map_width + tile_x;

    if (index < 0 || index >= static_cast<int>(m_config.collision.size())) {
        return false;
    }

    return m_config.collision[index] != 0;
}
