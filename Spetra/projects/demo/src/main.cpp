#include <memory>

#include "project_settings.hpp"
#include "map_loader.hpp"
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

    MapData map = load_map_from_json("assets/maps/schema_test_map.json");

    MapScene::Config map_scene_config;
    map_scene_config.map = map;
    map_scene_config.clear_color = {0, 0, 0, 255};

    game.set_starting_scene(std::make_unique<MapScene>(map_scene_config));

    return game.run();
}
