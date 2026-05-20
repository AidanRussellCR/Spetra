#include "map_loader.hpp"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "spetra/filesystem.hpp"

using json = nlohmann::json;

MapData load_map_from_json(const std::string& asset_path) {
    MapData map;

    std::string full_path = spetra::get_asset_path(asset_path);
    std::ifstream file(full_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << full_path << '\n';
        return map;
    }

    json data;
    file >> data;

    map.name = data.value("name", "Untitled");
    map.width = data.value("width", 0);
    map.height = data.value("height", 0);
    map.default_tile_size = data.value("default_tile_size", 16);
    map.tileset_path = data.value("tileset_path", "");

    if (data.contains("tile_size_override") && !data["tile_size_override"].is_null()) {
        map.tile_size_override = data["tile_size_override"].get<int>();
    }

    if (data.contains("spawn")) {
        map.spawn_x = data["spawn"].value("x", 0.0f);
        map.spawn_y = data["spawn"].value("y", 0.0f);
    }

    if (data.contains("layers")) {
        for (const auto& layer_json : data["layers"]) {
            TileLayer layer;
            layer.name = layer_json.value("name", "Layer");
            layer.visible = layer_json.value("visible", true);
            layer.tiles = layer_json.value("tiles", std::vector<int>{});

            map.layers.push_back(layer);
        }
    }

    if (data.contains("collision")) {
        map.collision.cells = data["collision"].value("cells", std::vector<int>{});
    }

    return map;
}
