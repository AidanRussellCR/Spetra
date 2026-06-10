#include "map_loader.hpp"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "spetra/filesystem.hpp"

using json = nlohmann::json;

namespace {

    // Enum/String

    FireMode fire_mode_from_string(const std::string& s) {
        if (s == "on_enter") {
            return FireMode::OnEnter;
        }
        return FireMode::OnInteract; // default
    }

    const char* fire_mode_to_string(FireMode mode) {
        return (mode == FireMode::OnEnter) ? "on_enter" : "on_interact";
    }

    ActorBehavior behavior_from_string(const std::string& s) {
        if (s == "wander") return ActorBehavior::Wander;
        if (s == "patrol") return ActorBehavior::Patrol;
        if (s == "follow") return ActorBehavior::Follow;
        return ActorBehavior::Static;
    }

    const char* behavior_to_string(ActorBehavior behavior) {
        switch (behavior) {
            case ActorBehavior::Wander: return "wander";
            case ActorBehavior::Patrol: return "patrol";
            case ActorBehavior::Follow: return "follow";
            case ActorBehavior::Static: break;
        }
        return "static";
    }

    Condition::Compare compare_from_string(const std::string& s) {
        if (s == "!=") return Condition::Compare::NotEqual;
        if (s == "<")  return Condition::Compare::Less;
        if (s == "<=") return Condition::Compare::LessEqual;
        if (s == ">")  return Condition::Compare::Greater;
        if (s == ">=") return Condition::Compare::GreaterEqual;
        return Condition::Compare::Equal; // "==" and anything unknown
    }

    const char* compare_to_string(Condition::Compare compare) {
        switch (compare) {
            case Condition::Compare::NotEqual:     return "!=";
            case Condition::Compare::Less:         return "<";
            case Condition::Compare::LessEqual:    return "<=";
            case Condition::Compare::Greater:      return ">";
            case Condition::Compare::GreaterEqual: return ">=";
            case Condition::Compare::Equal:        break;
        }
        return "==";
    }

    // Conditions

    Condition condition_from_json(const json& data) {
        Condition condition; // Always by default

        if (data.is_null() || !data.is_object()) {
            return condition;
        }

        if (data.contains("all")) {
            condition.kind = Condition::Kind::All;
            for (const auto& child : data["all"]) {
                condition.children.push_back(condition_from_json(child));
            }
            return condition;
        }

        if (data.contains("any")) {
            condition.kind = Condition::Kind::Any;
            for (const auto& child : data["any"]) {
                condition.children.push_back(condition_from_json(child));
            }
            return condition;
        }

        if (data.contains("not")) {
            condition.kind = Condition::Kind::Not;
            condition.children.push_back(condition_from_json(data["not"]));
            return condition;
        }

        if (data.contains("flag")) {
            condition.kind = Condition::Kind::Flag;
            condition.key = data["flag"].get<std::string>();
            condition.expect = data.value("equals", true);
            return condition;
        }

        if (data.contains("int")) {
            condition.kind = Condition::Kind::IntCompare;
            condition.key = data["int"].get<std::string>();
            condition.compare = compare_from_string(data.value("op", std::string("==")));
            condition.value = data.value("value", 0);
            return condition;
        }

        return condition;
    }

    json condition_to_json(const Condition& condition) {
        json data = json::object();

        switch (condition.kind) {
            case Condition::Kind::Always:
                return nullptr; // omitted by the caller

            case Condition::Kind::Flag:
                data["flag"] = condition.key;
                data["equals"] = condition.expect;
                return data;

            case Condition::Kind::IntCompare:
                data["int"] = condition.key;
                data["op"] = compare_to_string(condition.compare);
                data["value"] = condition.value;
                return data;

            case Condition::Kind::All:
            case Condition::Kind::Any: {
                json children = json::array();
                for (const Condition& child : condition.children) {
                    json c = condition_to_json(child);
                    if (!c.is_null()) {
                        children.push_back(c);
                    }
                }
                data[condition.kind == Condition::Kind::All ? "all" : "any"] = children;
                return data;
            }

            case Condition::Kind::Not: {
                if (condition.children.empty()) {
                    return nullptr;
                }
                json inner = condition_to_json(condition.children.front());
                if (inner.is_null()) {
                    return nullptr;
                }
                data["not"] = inner;
                return data;
            }
        }

        return nullptr;
    }

} // namespace

// Map from JSON

MapData map_from_json(const json& data) {
    MapData map;

    map.name = data.value("name", std::string("Untitled"));
    map.width = data.value("width", 0);
    map.height = data.value("height", 0);
    map.default_tile_size = data.value("default_tile_size", 16);
    map.tileset_path = data.value("tileset_path", std::string(""));

    if (data.contains("tile_size_override") && !data["tile_size_override"].is_null()) {
        map.tile_size_override = data["tile_size_override"].get<int>();
    }

    if (data.contains("spawn")) {
        map.spawn_x = data["spawn"].value("x", 0.0f);
        map.spawn_y = data["spawn"].value("y", 0.0f);
    }

    // Layers
    if (data.contains("layers")) {
        for (const auto& layer_json : data["layers"]) {
            TileLayer layer;
            layer.name = layer_json.value("name", std::string("Layer"));
            layer.visible = layer_json.value("visible", true);

            // Default -10
            layer.depth = layer_json.value("depth", -10);
            layer.parallax_x = layer_json.value("parallax_x", 1.0f);
            layer.parallax_y = layer_json.value("parallax_y", 1.0f);

            layer.tiles = layer_json.value("tiles", std::vector<int>{});

            map.layers.push_back(layer);
        }
    }

    // Collision
    if (data.contains("collision")) {
        map.collision.cells = data["collision"].value("cells", std::vector<int>{});
    }

    // Triggers
    if (data.contains("triggers")) {
        for (const auto& trigger_json : data["triggers"]) {
            Trigger trigger;
            trigger.id = trigger_json.value("id", std::string(""));
            trigger.fire_mode = fire_mode_from_string(
                trigger_json.value("fire_mode", std::string("on_interact")));
            trigger.once = trigger_json.value("once", false);

            if (trigger_json.contains("region")) {
                const auto& r = trigger_json["region"];
                trigger.region.x = r.value("x", 0);
                trigger.region.y = r.value("y", 0);
                trigger.region.w = r.value("w", 1);
                trigger.region.h = r.value("h", 1);
            }

            if (trigger_json.contains("condition")) {
                trigger.condition = condition_from_json(trigger_json["condition"]);
            }

            if (trigger_json.contains("actions")) {
                for (const auto& action_json : trigger_json["actions"]) {
                    TriggerAction action;
                    action.type = action_json.value("type", std::string(""));
                    action.params = action_json;
                    action.params.erase("type");

                    trigger.actions.push_back(action);
                }
            }

            map.triggers.push_back(trigger);
        }
    }

    // Actors
    if (data.contains("actors")) {
        for (const auto& actor_json : data["actors"]) {
            ActorPlacement actor;
            actor.id = actor_json.value("id", std::string(""));
            actor.sprite = actor_json.value("sprite", std::string(""));

            if (actor_json.contains("spawn")) {
                actor.spawn_x = actor_json["spawn"].value("x", 0.0f);
                actor.spawn_y = actor_json["spawn"].value("y", 0.0f);
            }

            actor.behavior = behavior_from_string(
                actor_json.value("behavior", std::string("static")));
            actor.interaction = actor_json.value("interaction", std::string(""));

            actor.size = actor_json.value("size", 16);
            actor.speed = actor_json.value("speed", 40.0f);
            actor.frame_width = actor_json.value("frame_width", 16);
            actor.frame_height = actor_json.value("frame_height", 16);
            actor.frame_count = actor_json.value("frame_count", 4);

            if (actor_json.contains("path")) {
                for (const auto& point_json : actor_json["path"]) {
                    PathPoint point;
                    point.x = point_json.value("x", 0);
                    point.y = point_json.value("y", 0);
                    actor.path.push_back(point);
                }
            }

            map.actors.push_back(actor);
        }
    }

    return map;
}

// Map to JSON

json map_to_json(const MapData& map) {
    json data = json::object();

    data["name"] = map.name;
    data["width"] = map.width;
    data["height"] = map.height;
    data["default_tile_size"] = map.default_tile_size;
    data["tileset_path"] = map.tileset_path;

    if (map.tile_size_override.has_value()) {
        data["tile_size_override"] = *map.tile_size_override;
    }

    data["spawn"] = json::object();
    data["spawn"]["x"] = map.spawn_x;
    data["spawn"]["y"] = map.spawn_y;

    // Layers
    json layers = json::array();
    for (const TileLayer& layer : map.layers) {
        json l = json::object();
        l["name"] = layer.name;
        l["visible"] = layer.visible;
        l["depth"] = layer.depth;
        l["parallax_x"] = layer.parallax_x;
        l["parallax_y"] = layer.parallax_y;
        l["tiles"] = layer.tiles;
        layers.push_back(l);
    }
    data["layers"] = layers;

    // Collision
    data["collision"] = json::object();
    data["collision"]["cells"] = map.collision.cells;

    // Triggers
    json triggers = json::array();
    for (const Trigger& trigger : map.triggers) {
        json t = json::object();
        t["id"] = trigger.id;

        t["region"] = json::object();
        t["region"]["x"] = trigger.region.x;
        t["region"]["y"] = trigger.region.y;
        t["region"]["w"] = trigger.region.w;
        t["region"]["h"] = trigger.region.h;

        t["fire_mode"] = fire_mode_to_string(trigger.fire_mode);
        t["once"] = trigger.once;

        json condition = condition_to_json(trigger.condition);
        if (!condition.is_null()) {
            t["condition"] = condition;
        }

        json actions = json::array();
        for (const TriggerAction& action : trigger.actions) {
            // Re-attach the type to the preserved payload
            json a = action.params;
            if (!a.is_object()) {
                a = json::object();
            }
            a["type"] = action.type;
            actions.push_back(a);
        }
        t["actions"] = actions;

        triggers.push_back(t);
    }
    data["triggers"] = triggers;

    // Actors
    json actors = json::array();
    for (const ActorPlacement& actor : map.actors) {
        json a = json::object();
        a["id"] = actor.id;
        a["sprite"] = actor.sprite;

        a["spawn"] = json::object();
        a["spawn"]["x"] = actor.spawn_x;
        a["spawn"]["y"] = actor.spawn_y;

        a["behavior"] = behavior_to_string(actor.behavior);
        a["interaction"] = actor.interaction;

        a["size"] = actor.size;
        a["speed"] = actor.speed;
        a["frame_width"] = actor.frame_width;
        a["frame_height"] = actor.frame_height;
        a["frame_count"] = actor.frame_count;

        json path = json::array();
        for (const PathPoint& point : actor.path) {
            json p = json::object();
            p["x"] = point.x;
            p["y"] = point.y;
            path.push_back(p);
        }
        a["path"] = path;

        actors.push_back(a);
    }
    data["actors"] = actors;

    return data;
}

// File helpers

MapData load_map_from_json(const std::string& asset_path) {
    MapData map;

    std::string full_path = spetra::get_asset_path(asset_path);
    std::ifstream file(full_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << full_path << '\n';
        return map;
    }

    json data;

    try {
        // ignore_comments = true so hand-authored maps with // notes still load
        data = json::parse(file, nullptr, true, true);
    }
    catch (const json::parse_error& e) {
        std::cerr << "Failed to parse map '" << full_path << "': " << e.what() << '\n';
        return map;
    }

    map = map_from_json(data);

    MapValidation validation = validate_map(map);

    for (const std::string& warning : validation.warnings) {
        std::cerr << "Map warning [" << map.name << "]: " << warning << '\n';
    }

    for (const std::string& error : validation.errors) {
        std::cerr << "Map ERROR [" << map.name << "]: " << error << '\n';
    }

    return map;
}

bool save_map_to_file(const MapData& map, const std::string& full_path) {
    std::ofstream file(full_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open map file for writing: " << full_path << '\n';
        return false;
    }

    file << map_to_json(map).dump(2) << '\n';

    if (!file.good()) {
        std::cerr << "Failed to write map file: " << full_path << '\n';
        return false;
    }

    return true;
}

bool save_map_to_json(const MapData& map, const std::string& asset_path) {
    return save_map_to_file(map, spetra::get_asset_path(asset_path));
}
