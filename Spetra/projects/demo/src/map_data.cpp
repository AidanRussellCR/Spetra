#include "map_data.hpp"

#include <set>
#include <string>

int MapData::tile_size() const {
    if (tile_size_override.has_value()) {
        return *tile_size_override;
    }

    return default_tile_size;
}

int MapData::pixel_width() const {
    return width * tile_size();
}

int MapData::pixel_height() const {
    return height * tile_size();
}

bool MapData::in_bounds(int tile_x, int tile_y) const {
    return tile_x >= 0 && tile_y >= 0 &&
    tile_x < width && tile_y < height;
}

bool MapData::is_tile_blocked(int tile_x, int tile_y) const {
    if (!in_bounds(tile_x, tile_y)) {
        return true; // outside the map is solid
    }

    int index = tile_y * width + tile_x;

    if (index < 0 || index >= static_cast<int>(collision.cells.size())) {
        return false; // no collision data for this cell
    }

    return collision.cells[index] != 0;
}

const Trigger* MapData::find_trigger(const std::string& id) const {
    for (const Trigger& trigger : triggers) {
        if (trigger.id == id) {
            return &trigger;
        }
    }

    return nullptr;
}

MapValidation validate_map(const MapData& map) {
    MapValidation result;

    // Dimensions and tile size
    if (map.width <= 0 || map.height <= 0) {
        result.errors.push_back("Map dimensions must be positive (got " +
        std::to_string(map.width) + "x" +
        std::to_string(map.height) + ").");
        return result; // everything below depends on valid dimensions
    }

    int ts = map.tile_size();
    if (ts <= 0 || ts % 2 != 0) {
        result.errors.push_back("Tile size must be positive and even (got " +
        std::to_string(ts) + ").");
    }

    if (map.tileset_path.empty()) {
        result.warnings.push_back("No tileset_path set.");
    }

    const std::size_t expected = static_cast<std::size_t>(map.width) *
    static_cast<std::size_t>(map.height);

    // Layers
    if (map.layers.empty()) {
        result.warnings.push_back("Map has no tile layers.");
    }

    for (const TileLayer& layer : map.layers) {
        if (layer.tiles.size() != expected) {
            result.errors.push_back("Layer '" + layer.name + "' has " +
            std::to_string(layer.tiles.size()) +
            " tiles; expected " + std::to_string(expected) + ".");
        }
    }

    // Collision
    if (!map.collision.cells.empty() && map.collision.cells.size() != expected) {
        result.errors.push_back("Collision layer has " +
        std::to_string(map.collision.cells.size()) +
        " cells; expected " + std::to_string(expected) + ".");
    }

    // Spawn
    if (map.spawn_x < 0.0f || map.spawn_y < 0.0f ||
        map.spawn_x >= static_cast<float>(map.pixel_width()) ||
        map.spawn_y >= static_cast<float>(map.pixel_height())) {
        result.warnings.push_back("Spawn point lies outside the map bounds.");
        }

        // Triggers
        std::set<std::string> trigger_ids;

    for (const Trigger& trigger : map.triggers) {
        if (trigger.id.empty()) {
            result.warnings.push_back("A trigger has no id (it can't be referenced).");
        }
        else if (!trigger_ids.insert(trigger.id).second) {
            result.errors.push_back("Duplicate trigger id: '" + trigger.id + "'.");
        }

        const TriggerRegion& r = trigger.region;

        if (r.w <= 0 || r.h <= 0) {
            result.errors.push_back("Trigger '" + trigger.id +
            "' has a non-positive region size.");
        }
        else if (!map.in_bounds(r.x, r.y) ||
            !map.in_bounds(r.x + r.w - 1, r.y + r.h - 1)) {
            result.warnings.push_back("Trigger '" + trigger.id +
            "' region extends outside the map.");
            }

            if (trigger.actions.empty()) {
                result.warnings.push_back("Trigger '" + trigger.id + "' has no actions.");
            }

            for (const TriggerAction& action : trigger.actions) {
                if (action.type.empty()) {
                    result.errors.push_back("Trigger '" + trigger.id +
                    "' has an action with no type.");
                }
            }
    }

    // Actors
    std::set<std::string> actor_ids;

    for (const ActorPlacement& actor : map.actors) {
        if (actor.id.empty()) {
            result.warnings.push_back("An actor has no id.");
        }
        else if (!actor_ids.insert(actor.id).second) {
            result.errors.push_back("Duplicate actor id: '" + actor.id + "'.");
        }

        if (actor.sprite.empty()) {
            result.warnings.push_back("Actor '" + actor.id +
            "' has no sprite (will draw a fallback box).");
        }

        int tile_x = static_cast<int>(actor.spawn_x) / (ts > 0 ? ts : 1);
        int tile_y = static_cast<int>(actor.spawn_y) / (ts > 0 ? ts : 1);

        if (!map.in_bounds(tile_x, tile_y)) {
            result.warnings.push_back("Actor '" + actor.id +
            "' spawns outside the map.");
        }

        // A dangling interaction reference is a warning
        if (!actor.interaction.empty() && !map.find_trigger(actor.interaction)) {
            result.warnings.push_back("Actor '" + actor.id +
            "' references unknown trigger '" +
            actor.interaction + "'.");
        }

        if (actor.behavior == ActorBehavior::Patrol && actor.path.empty()) {
            result.warnings.push_back("Actor '" + actor.id +
            "' has Patrol behavior but no path.");
        }

        for (const PathPoint& point : actor.path) {
            if (!map.in_bounds(point.x, point.y)) {
                result.warnings.push_back("Actor '" + actor.id +
                "' has a path waypoint outside the map.");
                break;
            }
        }
    }

    return result;
}
