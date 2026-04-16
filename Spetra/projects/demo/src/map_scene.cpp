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
}

void MapScene::handle_input(spetra::Input& input, spetra::SceneManager& scene_manager) {
    (void)input;
    (void)scene_manager;
}

void MapScene::update(double delta_time, spetra::SceneManager& scene_manager) {
    (void)delta_time;
    (void)scene_manager;
}

void MapScene::render(spetra::Window& window) {
    window.clear(m_config.clear_color);

    if (!m_loaded) {
        std::string full_path = spetra::get_asset_path(m_config.tileset_path);

        if (!m_tileset.load_from_file(window.renderer(), full_path)) {
            std::cerr << "Failed to load tileset: " << full_path << '\n';
            return;
        }

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
