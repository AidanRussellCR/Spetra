#pragma once

#include <string>
#include <vector>
#include <optional>

struct TileLayer {
    std::string name;
    bool visible = true;
    std::vector<int> tiles;
};

struct CollisionLayer {
    std::vector<int> cells;
};

struct MapData {
    std::string name = "Untitled";

    int width = 0;
    int height = 0;

    int default_tile_size = 16;
    std::optional<int> tile_size_override;

    std::string tileset_path;

    std::vector<TileLayer> layers;
    CollisionLayer collision;

    float spawn_x = 0.0f;
    float spawn_y = 0.0f;
};
