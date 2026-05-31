#include "map_scene.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

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
    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    m_player_x = m_config.map.spawn_x;
    m_player_y = m_config.map.spawn_y;

    m_dialogue_box.set_skin_path("assets/ui/textbox.png", 6);
    m_dialogue_box.set_font_path("assets/fonts/dialogue.ttf", 12.0f);
}

void MapScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    (void)scene_manager;

    m_move_left = input.is_down(SDL_SCANCODE_LEFT) || input.is_down(SDL_SCANCODE_A);
    m_move_right = input.is_down(SDL_SCANCODE_RIGHT) || input.is_down(SDL_SCANCODE_D);
    m_move_up = input.is_down(SDL_SCANCODE_UP) || input.is_down(SDL_SCANCODE_W);
    m_move_down = input.is_down(SDL_SCANCODE_DOWN) || input.is_down(SDL_SCANCODE_S);

    if (input.was_pressed(SDL_SCANCODE_F1)) {
        m_show_collision_debug = !m_show_collision_debug;
    }

    if (input.was_pressed(SDL_SCANCODE_E) || input.was_pressed(SDL_SCANCODE_RETURN)) {
        if (m_dialogue_box.is_active()) {
            m_dialogue_box.advance();
        }
        else {
            m_dialogue_box.start({
                {"Speaker 1", "This is the initial test of Spetra's dialogue system."},
                {"Speaker 2", "Speaker names can change between dialogue sections."},
                {"", "Narration can appear without a speaker name as well."}
            });
        }
    }

    if (m_dialogue_box.is_active()) {
        m_move_left = false;
        m_move_right = false;
        m_move_up = false;
        m_move_down = false;
    }
}

void MapScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)scene_manager;

    m_last_delta_time = delta_time;

    m_dialogue_box.update(delta_time);

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

    bool is_moving = move_x != 0.0f || move_y != 0.0f;

    if (move_y > 0.0f) {
        m_player_direction = 0; // down
    }
    else if (move_x < 0.0f) {
        m_player_direction = 1; // left
    }
    else if (move_x > 0.0f) {
        m_player_direction = 2; // right
    }
    else if (move_y < 0.0f) {
        m_player_direction = 3; // up
    }

    if (is_moving) {
        m_walk_anim_timer += delta_time;

        if (m_walk_anim_timer >= m_walk_anim_frame_time) {
            m_walk_anim_timer = 0.0;
            m_player_frame = (m_player_frame + 1) % 4;
        }
    }
    else {
        m_walk_anim_timer = 0.0;
        m_player_frame = 0;
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

    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    // Clamp player to map bounds
    float max_x = static_cast<float>(std::max(0, map_pixel_width - m_player_size));
    float max_y = static_cast<float>(std::max(0, map_pixel_height - m_player_size));

    m_player_x = std::clamp(m_player_x, 0.0f, max_x);
    m_player_y = std::clamp(m_player_y, 0.0f, max_y);
}

void MapScene::render(spetra::Window& window) {
    window.clear(m_config.clear_color);

    if (!m_loaded) {
        std::string full_path = spetra::get_asset_path(m_config.map.tileset_path);

        if (!m_tileset.load_from_file(window.renderer(), full_path)) {
            std::cerr << "Failed to load tileset: " << full_path << '\n';
            window.present();
            return;
        }

        std::cout << "Loaded tileset: " << full_path << " (" << m_tileset.width() << "x" << m_tileset.height() << ")\n";

        m_loaded = true;
    }

    if (!m_player_texture_loaded) {
        std::string full_path = spetra::get_asset_path(m_player_sprite_path);

        if (!m_player_texture.load_from_file(window.renderer(), full_path)) {
            std::cerr << "Failed to load player sprite: " << full_path << '\n';
        }
        else {
            std::cout << "Loaded player sprite: " << full_path
            << " (" << m_player_texture.width()
            << "x" << m_player_texture.height() << ")\n";

            m_player_texture_loaded = true;
        }
    }

    int resolved_tile_size = tile_size();
    if (!is_valid_tile_size(resolved_tile_size)) {
        window.present();
        return;
    }

    if (m_config.map.width <= 0 || m_config.map.height <= 0) {
        window.present();
        return;
    }

    for (const TileLayer& layer : m_config.map.layers) {
        if (static_cast<int>(layer.tiles.size()) != m_config.map.width * m_config.map.height) {
            std::cerr << "Invalid tile layer size: " << layer.name << '\n';
            window.present();
            return;
        }
    }

    int tileset_columns = m_tileset.width() / resolved_tile_size;
    if (tileset_columns <= 0) {
        window.present();
        return;
    }

    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    update_camera(window, m_last_delta_time);

    const float camera_x = m_camera_x;
    const float camera_y = m_camera_y;

    // Draw tiles (layers)
    for (const TileLayer& layer : m_config.map.layers) {
        if (!layer.visible) {
            continue;
        }

        for (int y = 0; y < m_config.map.height; ++y) {
            for (int x = 0; x < m_config.map.width; ++x) {
                int tile_index = layer.tiles[y * m_config.map.width + x];
                if (tile_index < 0) {
                    continue;
                }

                int src_x = (tile_index % tileset_columns) * resolved_tile_size;
                int src_y = (tile_index / tileset_columns) * resolved_tile_size;

                int dst_x = static_cast<int>(std::lround(x * resolved_tile_size - camera_x));
                int dst_y = static_cast<int>(std::lround(y * resolved_tile_size - camera_y));

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
    }

    // DEBUG - show collision
    if (m_show_collision_debug) {
        for (int y = 0; y < m_config.map.height; ++y) {
            for (int x = 0; x < m_config.map.width; ++x) {
                int index = y * m_config.map.width + x;

                if (index < 0 || index >= static_cast<int>(m_config.map.collision.cells.size())) {
                    continue;
                }

                if (m_config.map.collision.cells[index] == 0) {
                    continue;
                }

                int dst_x = static_cast<int>(std::lround(x * resolved_tile_size - camera_x));
                int dst_y = static_cast<int>(std::lround(y * resolved_tile_size - camera_y));

                if (dst_x + resolved_tile_size <= 0 || dst_y + resolved_tile_size <= 0) {
                    continue;
                }

                if (dst_x >= window.render_width() || dst_y >= window.render_height()) {
                    continue;
                }

                window.draw_filled_rect(
                    spetra::Color{255, 0, 0, 100},
                    dst_x,
                    dst_y,
                    resolved_tile_size,
                    resolved_tile_size
                );
            }
        }
    }

    // Draw player
    int player_screen_x = static_cast<int>(std::lround(m_player_x - camera_x));
    int player_screen_y = static_cast<int>(std::lround(m_player_y - camera_y));

    if (m_player_texture_loaded) {
        int src_x = m_player_frame * m_player_frame_width;
        int src_y = m_player_direction * m_player_frame_height;

        window.draw_texture_region(
            m_player_texture,
            src_x,
            src_y,
            m_player_frame_width,
            m_player_frame_height,
            player_screen_x,
            player_screen_y,
            m_player_frame_width,
            m_player_frame_height
        );
    }
    else {
        window.draw_filled_rect(
            m_player_color,
            player_screen_x,
            player_screen_y,
            m_player_size,
            m_player_size
        );
    }

    m_dialogue_box.render(window);

    window.present();
}

void MapScene::update_camera(const spetra::Window& window, double delta_time) {
    int resolved_tile_size = tile_size();

    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    float target_x = m_camera_x;
    float target_y = m_camera_y;

    if (m_camera_mode == CameraMode::FollowPlayer) {
        target_x = m_player_x
                   + static_cast<float>(m_player_size) / 2.0f
                   - static_cast<float>(window.render_width()) / 2.0f;

        target_y = m_player_y
                   + static_cast<float>(m_player_size) / 2.0f
                   - static_cast<float>(window.render_height()) / 2.0f;
    }

    // Clamp camera to map bounds
    float max_camera_x = static_cast<float>(std::max(0, map_pixel_width - window.render_width()));
    float max_camera_y = static_cast<float>(std::max(0, map_pixel_height - window.render_height()));

    target_x = std::clamp(target_x, 0.0f, max_camera_x);
    target_y = std::clamp(target_y, 0.0f, max_camera_y);

    // Snap on the first frame
    if (!m_camera_initialized) {
        m_camera_x = target_x;
        m_camera_y = target_y;
        m_camera_initialized = true;
        return;
    }

    if (m_camera_smoothing > 0.0f) {
        // Frame-rate independent smoothing
        float t = 1.0f - std::exp(-m_camera_smoothing * static_cast<float>(delta_time));
        m_camera_x += (target_x - m_camera_x) * t;
        m_camera_y += (target_y - m_camera_y) * t;

        // Kill sub-pixel drift
        if (std::abs(target_x - m_camera_x) < 0.05f) m_camera_x = target_x;
        if (std::abs(target_y - m_camera_y) < 0.05f) m_camera_y = target_y;
    }
    else {
        m_camera_x = target_x;
        m_camera_y = target_y;
    }
}

int MapScene::tile_size() const {
    if (m_config.map.tile_size_override.has_value()) {
        return *m_config.map.tile_size_override;
    }

    return m_config.map.default_tile_size;
}

bool MapScene::is_valid_tile_size(int size) const {
    return size > 0 && (size % 2 == 0);
}

bool MapScene::is_tile_blocked(int tile_x, int tile_y) const {
    if (tile_x < 0 || tile_y < 0 ||
        tile_x >= m_config.map.width ||
        tile_y >= m_config.map.height) {
        return true; // treat region outside map as solid
    }

    int index = tile_y * m_config.map.width + tile_x;

    if (index < 0 || index >= static_cast<int>(m_config.map.collision.cells.size())) {
        return false;
    }

    return m_config.map.collision.cells[index] != 0;
}
