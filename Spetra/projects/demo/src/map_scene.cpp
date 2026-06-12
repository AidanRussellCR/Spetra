#include "map_scene.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>

#include "pause_overlay_scene.hpp"
#include "battle_stub_scene.hpp"
#include "menu_scene.hpp"
#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/world.hpp"
#include "spetra/filesystem.hpp"

MapScene::MapScene(const Config& config)
: m_config(config) {
}

void MapScene::on_enter(spetra::Window& window) {
    int resolved_tile_size = tile_size();
    if (!is_valid_tile_size(resolved_tile_size)) {
        std::cerr << "Invalid tile size. Must be positive and even.\n";
    }

    // Configure the player entity and drop it at the map spawn
    m_player.x = m_config.map.spawn_x;
    m_player.y = m_config.map.spawn_y;
    m_player.size = 12; // collision box
    m_player.speed = 80.0f;
    m_player.frame_width = 16;
    m_player.frame_height = 16;
    m_player.frame_count = 4;
    m_player.anim_frame_time = 0.15;
    m_player.fallback_color = {255, 220, 120, 255};

    // Re-snap the camera
    m_camera_initialized = false;

    m_dialogue_box.set_skin_path("assets/ui/textbox.png", 6);
    m_dialogue_box.set_font_path("assets/fonts/dialogue.ttf", 12.0f);

    // Load GPU resources
    std::string tileset_path = spetra::get_asset_path(m_config.map.tileset_path);
    if (!m_tileset.load_from_file(window.renderer(), tileset_path)) {
        std::cerr << "Failed to load tileset: " << tileset_path << '\n';
    }
    else {
        std::cout << "Loaded tileset: " << tileset_path
        << " (" << m_tileset.width() << "x" << m_tileset.height() << ")\n";
        m_loaded = true;
    }

    std::string sprite_path = spetra::get_asset_path(m_player_sprite_path);
    if (!m_player.load_sprite(window.renderer(), sprite_path)) {
        std::cerr << "Failed to load player sprite: " << sprite_path << '\n';
    }
    else {
        std::cout << "Loaded player sprite: " << sprite_path << '\n';
    }
}

void MapScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    m_move_left = input.is_down(SDL_SCANCODE_LEFT) || input.is_down(SDL_SCANCODE_A);
    m_move_right = input.is_down(SDL_SCANCODE_RIGHT) || input.is_down(SDL_SCANCODE_D);
    m_move_up = input.is_down(SDL_SCANCODE_UP) || input.is_down(SDL_SCANCODE_W);
    m_move_down = input.is_down(SDL_SCANCODE_DOWN) || input.is_down(SDL_SCANCODE_S);

    if (input.was_pressed(SDL_SCANCODE_F1)) {
        m_show_collision_debug = !m_show_collision_debug;
    }

    // P pushes a translucent pause overlay
    // B pushes an opaque battle stub
    if (input.was_pressed(SDL_SCANCODE_P)) {
        scene_manager.push_scene(std::make_unique<PauseOverlayScene>());
        return;
    }
    if (input.was_pressed(SDL_SCANCODE_B)) {
        scene_manager.push_scene(std::make_unique<BattleStubScene>());
        return;
    }

    // M pushes the UI-framework pause menu
    if (input.was_pressed(SDL_SCANCODE_M)) {
        scene_manager.push_scene(std::make_unique<MenuScene>());
        return;
    }

    if (input.was_pressed(SDL_SCANCODE_E) || input.was_pressed(SDL_SCANCODE_RETURN)) {
        if (m_dialogue_box.is_active()) {
            m_dialogue_box.advance();
        }
        else if (!world().variables.flag("demo.seen_intro")) {
            // First time: play the intro and remember it in global state
            m_dialogue_box.start({
                {"Speaker 1", "This is the initial test of Spetra's dialogue system."},
                {"Speaker 2", "Speaker names can change between dialogue sections."},
                {"", "Narration can appear without a speaker name as well."}
            });
            world().variables.set_flag("demo.seen_intro", true);
        }
        else {
            // The flag persists for the rest of the run, so we branch on it
            m_dialogue_box.start({
                {"", "You've already seen the intro (demo.seen_intro is set)."}
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

    m_player.face_movement(move_x, move_y);
    m_player.animate(delta_time, is_moving);

    float dx = move_x * m_player.speed * static_cast<float>(delta_time);
    float dy = move_y * m_player.speed * static_cast<float>(delta_time);

    resolve_movement(m_player, dx, dy);

    int resolved_tile_size = tile_size();
    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    // Clamp player to map bounds
    float max_x = static_cast<float>(std::max(0, map_pixel_width - m_player.size));
    float max_y = static_cast<float>(std::max(0, map_pixel_height - m_player.size));

    m_player.x = std::clamp(m_player.x, 0.0f, max_x);
    m_player.y = std::clamp(m_player.y, 0.0f, max_y);
}

void MapScene::render(spetra::Window& window) {
    window.clear(m_config.clear_color);

    // If the tileset is missing bail after clearing
    if (!m_loaded) {
        return;
    }

    int resolved_tile_size = tile_size();
    if (!is_valid_tile_size(resolved_tile_size)) {
        return;
    }

    if (m_config.map.width <= 0 || m_config.map.height <= 0) {
        return;
    }

    for (const TileLayer& layer : m_config.map.layers) {
        if (static_cast<int>(layer.tiles.size()) != m_config.map.width * m_config.map.height) {
            std::cerr << "Invalid tile layer size: " << layer.name << '\n';
            return;
        }
    }

    int tileset_columns = m_tileset.width() / resolved_tile_size;
    if (tileset_columns <= 0) {
        return;
    }

    update_camera(window, m_last_delta_time);

    const float camera_x = m_camera_x;
    const float camera_y = m_camera_y;

    // Build a single depth-sorted draw order
    std::vector<const spetra::Entity*> entities;
    entities.push_back(&m_player);

    m_draw_list.clear();

    for (std::size_t i = 0; i < m_config.map.layers.size(); ++i) {
        const TileLayer& layer = m_config.map.layers[i];

        if (!layer.visible) {
            continue;
        }

        m_draw_list.push_back(demo::DrawItem{layer.depth, 0, static_cast<int>(i), false});
    }

    for (std::size_t i = 0; i < entities.size(); ++i) {
        // feet_y = bottom of the collision box
        int feet_y = static_cast<int>(entities[i]->y + entities[i]->size);
        m_draw_list.push_back(demo::DrawItem{0, feet_y, static_cast<int>(i), true});
    }

    demo::sort_draw_order(m_draw_list);

    for (const demo::DrawItem& item : m_draw_list) {
        if (item.is_entity) {
            // Entities render at true camera (parallax 1.0 = ground)
            entities[item.index]->render(window, camera_x, camera_y);
        }
        else {
            draw_tile_layer(window, m_config.map.layers[item.index], tileset_columns, resolved_tile_size, camera_x, camera_y);
        }
    }

    // DEBUG - show collision
    if (m_show_collision_debug) {
        draw_collision_debug(window, resolved_tile_size, camera_x, camera_y);
    }
    m_dialogue_box.render(window);
}

void MapScene::draw_tile_layer(spetra::Window& window, const TileLayer& layer, int tileset_columns, int resolved_tile_size, float camera_x, float camera_y) {
    // Parallax: scale the camera offset per layer
    // 1.0 = moves with the camera
    // 0.5 = half speed (distant background)
    // 0.0 = pinned to the screen
    const float cam_x = camera_x * layer.parallax_x;
    const float cam_y = camera_y * layer.parallax_y;

    for (int y = 0; y < m_config.map.height; ++y) {
        for (int x = 0; x < m_config.map.width; ++x) {
            int tile_index = layer.tiles[y * m_config.map.width + x];
            if (tile_index < 0) {
                continue;
            }

            int src_x = (tile_index % tileset_columns) * resolved_tile_size;
            int src_y = (tile_index / tileset_columns) * resolved_tile_size;

            int dst_x = static_cast<int>(std::lround(x * resolved_tile_size - cam_x));
            int dst_y = static_cast<int>(std::lround(y * resolved_tile_size - cam_y));

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

void MapScene::draw_collision_debug(spetra::Window& window, int resolved_tile_size, float camera_x, float camera_y) {
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

void MapScene::resolve_movement(spetra::Entity& entity, float dx, float dy) {
    int resolved_tile_size = tile_size();

    // X movement
    float new_x = entity.x + dx;

    int left_tile   = static_cast<int>(new_x) / resolved_tile_size;
    int right_tile  = static_cast<int>(new_x + entity.size - 1) / resolved_tile_size;
    int top_tile    = static_cast<int>(entity.y) / resolved_tile_size;
    int bottom_tile = static_cast<int>(entity.y + entity.size - 1) / resolved_tile_size;

    if (!is_tile_blocked(left_tile, top_tile) &&
        !is_tile_blocked(left_tile, bottom_tile) &&
        !is_tile_blocked(right_tile, top_tile) &&
        !is_tile_blocked(right_tile, bottom_tile)) {
        entity.x = new_x;
        }

        // Y movement
        float new_y = entity.y + dy;

    left_tile   = static_cast<int>(entity.x) / resolved_tile_size;
    right_tile  = static_cast<int>(entity.x + entity.size - 1) / resolved_tile_size;
    top_tile    = static_cast<int>(new_y) / resolved_tile_size;
    bottom_tile = static_cast<int>(new_y + entity.size - 1) / resolved_tile_size;

    if (!is_tile_blocked(left_tile, top_tile) &&
        !is_tile_blocked(left_tile, bottom_tile) &&
        !is_tile_blocked(right_tile, top_tile) &&
        !is_tile_blocked(right_tile, bottom_tile)) {
        entity.y = new_y;
        }
}

void MapScene::update_camera(const spetra::Window& window, double delta_time) {
    int resolved_tile_size = tile_size();

    int map_pixel_width = m_config.map.width * resolved_tile_size;
    int map_pixel_height = m_config.map.height * resolved_tile_size;

    float target_x = m_camera_x;
    float target_y = m_camera_y;

    if (m_camera_mode == CameraMode::FollowPlayer) {
        target_x = m_player.x
                   + static_cast<float>(m_player.size) / 2.0f
                   - static_cast<float>(window.render_width()) / 2.0f;

        target_y = m_player.y
                   + static_cast<float>(m_player.size) / 2.0f
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
    return m_config.map.tile_size();
}

bool MapScene::is_valid_tile_size(int size) const {
    return size > 0 && (size % 2 == 0);
}

bool MapScene::is_tile_blocked(int tile_x, int tile_y) const {
    return m_config.map.is_tile_blocked(tile_x, tile_y);
}
