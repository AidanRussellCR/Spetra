#include <memory>

#include "project_settings.hpp"
#include "map_scene.hpp"
#include "spetra/game.hpp"
#include "spetra/config.hpp"

int main() {
    spetra::AppConfig config;
    config.title = "Spetra Demo";

    config.render_width = demo::ProjectSettings::base_viewport_width;
    config.render_height = demo::ProjectSettings::base_viewport_height;

    config.window_width =
    demo::ProjectSettings::base_viewport_width *
    demo::ProjectSettings::display_scale;

    config.window_height =
    demo::ProjectSettings::base_viewport_height *
    demo::ProjectSettings::display_scale;

    config.clear_color = {20, 24, 40, 255};

    spetra::Game game(config);

    MapData map;

    map.width = 12;
    map.height = 8;
    map.tileset_path = "assets/tileset.png";
    map.default_tile_size = demo::ProjectSettings::default_tile_size;

    // Tile Layer
    TileLayer ground;
    ground.name = "Ground";

    ground.tiles = {
        0,1,2,3,0,1,2,3,0,1,2,3,
        4,5,6,7,4,5,6,7,4,5,6,7,
        0,1,2,3,0,1,2,3,0,1,2,3,
        4,5,6,7,4,5,6,7,4,5,6,7,
        0,1,2,3,0,1,2,3,0,1,2,3,
        4,5,6,7,4,5,6,7,4,5,6,7,
        0,1,2,3,0,1,2,3,0,1,2,3,
        4,5,6,7,4,5,6,7,4,5,6,7
    };

    map.layers.push_back(ground);

    // Collision
    map.collision.cells = {
        1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,0,0,1,1,
        1,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,1,
        1,1,0,0,0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1
    };

    map.spawn_x = 64;
    map.spawn_y = 64;

    // MapScene config
    MapScene::Config map_scene_config;
    map_scene_config.map = map;
    map_scene_config.clear_color = {0,0,0,255};

    game.set_starting_scene(std::make_unique<MapScene>(map_scene_config));

    return game.run();
}
