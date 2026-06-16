#include "map_scene.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>

#include "pause_overlay_scene.hpp"
#include "battle_stub_scene.hpp"
#include "menu_scene.hpp"
#include "condition_eval.hpp"
#include "map_loader.hpp"
#include "spetra/input.hpp"
#include "spetra/scene_manager.hpp"
#include "spetra/window.hpp"
#include "spetra/world.hpp"
#include "spetra/filesystem.hpp"

MapScene::MapScene(const Config& config)
: m_config(config), m_events(*this) {
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

    // Global state for events
    m_events.set_world(world());

    // Choice menu UI
    m_ui_theme.set_skin("assets/ui/textbox.png", 6);
    m_ui_theme.set_font("assets/fonts/dialogue.ttf", 12.0f);
    m_ui_theme.line_height = 14;
    m_ui.set_theme(&m_ui_theme);

    // Prevent firing on load
    m_tile_initialized = false;

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

    // Choice menu owns input while it's open
    if (m_choice_active) {
        if (m_choice_ui_built) {
            m_ui.handle_input(input);
        }
        m_move_left = m_move_right = m_move_up = m_move_down = false;
        return;
    }

    if (input.was_pressed(SDL_SCANCODE_E) || input.was_pressed(SDL_SCANCODE_RETURN)) {
        if (m_dialogue_box.is_active()) {
            m_dialogue_box.advance();
        }
        else if (!m_events.is_running()) {
            try_fire_on_interact();
        }
    }

    // Lock player input during event
    if (events_busy()) {
        m_move_left = false;
        m_move_right = false;
        m_move_up = false;
        m_move_down = false;
    }
}

void MapScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    m_last_delta_time = delta_time;

    m_dialogue_box.update(delta_time);

    m_ui.update(delta_time);

    // Advance any running event first
    m_events.update(delta_time);

    // Apply transitions the runner requested
    if (m_pending_battle) {
        m_pending_battle = false;
        std::cout << "[MapScene] start_battle encounter='" << m_pending_encounter << "'\n";
        scene_manager.push_scene(std::make_unique<BattleStubScene>());
        return;
    }

    if (m_pending_change_map) {
        m_pending_change_map = false;

        MapData next = load_map_from_json(m_pending_map_path);
        if (next.width > 0 && next.height > 0) {
            MapScene::Config cfg;
            cfg.map = next;
            cfg.clear_color = m_config.clear_color;
            if (m_pending_spawn) {
                cfg.map.spawn_x = m_pending_spawn_x;
                cfg.map.spawn_y = m_pending_spawn_y;
            }
            scene_manager.change_scene(std::make_unique<MapScene>(cfg));
        }
        else {
            std::cerr << "[MapScene] change_map failed: " << m_pending_map_path << '\n';
        }
        return;
    }

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

    // on_enter triggers
    int tx = 0;
    int ty = 0;
    player_tile(tx, ty);

    if (!m_tile_initialized) {
        m_prev_tile_x = tx;
        m_prev_tile_y = ty;
        m_tile_initialized = true;
    }
    else if ((tx != m_prev_tile_x || ty != m_prev_tile_y)) {
        m_prev_tile_x = tx;
        m_prev_tile_y = ty;

        if (!events_busy()) {
            try_fire_on_enter(tx, ty);
        }
    }

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

    // Choice menu show above dialogue box
    if (m_choice_pending) {
        m_choice_pending = false;
        build_choice_ui(window);
    }
    if (m_choice_active && m_choice_ui_built) {
        m_ui.render(window);
    }

    if (!m_choice_active && m_choice_ui_built) {
        m_ui.clear();
        m_choice_panel = nullptr;
        m_choice_menu = nullptr;
        m_choice_ui_built = false;
    }

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

// ---------------------------------------------------------------------------
// demo::EventHost
// ---------------------------------------------------------------------------

void MapScene::start_dialogue(const std::vector<spetra::DialogueLine>& lines) {
    m_dialogue_box.start(lines);
}

bool MapScene::is_dialogue_active() const {
    return m_dialogue_box.is_active();
}

void MapScene::start_choice(const std::vector<std::string>& options) {
    std::vector<spetra::ui::MenuItem> items;
    items.reserve(options.size());
    for (const std::string& text : options) {
        items.push_back(spetra::ui::MenuItem{text, true, ""});
    }

    m_choice_items = items;
    m_choice_pending = true;   // UI is built on the next render (needs a Window)
    m_choice_active = true;
    m_choice_result = -1;
}

bool MapScene::is_choice_active() const {
    return m_choice_active;
}

int MapScene::choice_result() const {
    return m_choice_result;
}

void MapScene::request_change_map(const std::string& map_path,
                                  bool has_spawn, float spawn_x, float spawn_y) {
    m_pending_change_map = true;
    m_pending_map_path = map_path;
    m_pending_spawn = has_spawn;
    m_pending_spawn_x = spawn_x;
    m_pending_spawn_y = spawn_y;
}

void MapScene::request_battle(const std::string& encounter) {
    m_pending_battle = true;
    m_pending_encounter = encounter;
}

void MapScene::set_camera_mode(const std::string& mode) {
    if (mode == "cutscene") {
        m_camera_mode = CameraMode::Cutscene;
    }
    else if (mode == "manual") {
        m_camera_mode = CameraMode::Manual;
    }
    else {
        m_camera_mode = CameraMode::FollowPlayer;
    }
}

// Trigger firing

bool MapScene::events_busy() const {
    return m_events.is_running() || m_dialogue_box.is_active() || m_choice_active;
}

void MapScene::player_tile(int& tile_x, int& tile_y) const {
    int ts = tile_size();
    if (ts <= 0) { tile_x = 0; tile_y = 0; return; }

    // Centre of the collision box is player location
    tile_x = static_cast<int>(m_player.x + m_player.size / 2.0f) / ts;
    tile_y = static_cast<int>(m_player.y + m_player.size / 2.0f) / ts;
}

void MapScene::facing_tile(int& tile_x, int& tile_y) const {
    player_tile(tile_x, tile_y);

    switch (m_player.direction) {
        case spetra::Direction::Up:    --tile_y; break;
        case spetra::Direction::Down:  ++tile_y; break;
        case spetra::Direction::Left:  --tile_x; break;
        case spetra::Direction::Right: ++tile_x; break;
    }
}

bool MapScene::trigger_available(const Trigger& trigger) const {
    if (trigger.once && !trigger.id.empty()) {
        if (world().variables.flag(
                demo::EventRunner::fired_key(m_config.map.name, trigger.id))) {
            return false;
        }
    }

    return demo::evaluate(trigger.condition, world().variables);
}

bool MapScene::try_fire_on_enter(int tile_x, int tile_y) {
    for (const Trigger& trigger : m_config.map.triggers) {
        if (trigger.fire_mode != FireMode::OnEnter) {
            continue;
        }
        if (!trigger.region.contains_tile(tile_x, tile_y)) {
            continue;
        }
        if (!trigger_available(trigger)) {
            continue;
        }

        m_events.run_trigger(trigger, m_config.map.name);
        return true;
    }

    return false;
}

bool MapScene::try_fire_on_interact() {
    int tx = 0;
    int ty = 0;
    facing_tile(tx, ty);

    for (const Trigger& trigger : m_config.map.triggers) {
        if (trigger.fire_mode != FireMode::OnInteract) {
            continue;
        }
        if (!trigger.region.contains_tile(tx, ty)) {
            continue;
        }
        if (!trigger_available(trigger)) {
            continue;
        }

        m_events.run_trigger(trigger, m_config.map.name);
        return true;
    }

    return false;
}

void MapScene::build_choice_ui(spetra::Window& window) {
    m_ui.clear();
    m_choice_panel = nullptr;
    m_choice_menu = nullptr;

    int rw = window.render_width();

    // Show choice box above the dialogue box
    int pw = 110;
    int ph = static_cast<int>(m_choice_items.size()) * m_ui_theme.line_height + 16;
    int px = rw - pw - 16;
    int py = window.render_height() - 58 - 12 - ph - 4;

    auto panel = std::make_unique<spetra::ui::Panel>();
    panel->rect = spetra::ui::Rect{px, py, pw, ph};
    panel->instant();
    m_choice_panel = static_cast<spetra::ui::Panel*>(m_ui.add(std::move(panel)));

    auto menu = std::make_unique<spetra::ui::ListMenu>();
    menu->rect = spetra::ui::Rect{px + 8, py + 8, pw - 16, 0};
    menu->set_items(m_choice_items);

    m_choice_menu = static_cast<spetra::ui::ListMenu*>(
        m_choice_panel->add_child(std::move(menu)));

    m_choice_menu->on_select = [this](int index) {
        m_choice_result = index;
        m_choice_active = false;
    };

    m_ui.set_focus(m_choice_menu);
    m_ui.layout();
    m_choice_ui_built = true;
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
