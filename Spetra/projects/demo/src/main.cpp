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

    MapScene::Config map_config;
    map_config.tileset_path = "assets/tileset.png";
    map_config.map_width = 12;
    map_config.map_height = 8;
    map_config.default_tile_size = demo::ProjectSettings::default_tile_size;
    map_config.clear_color = {0, 0, 0, 255};

    map_config.tiles = {
        0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
        4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,
        0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
        4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,
        0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
        4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,
        0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
        4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7
    };

    game.set_starting_scene(std::make_unique<MapScene>(map_config));

    return game.run();
}
