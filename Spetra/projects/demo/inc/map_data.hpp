#pragma once

#include <string>
#include <vector>
#include <optional>

#include <nlohmann/json.hpp>

// Map format is triggers, actors, per-layer depth/parallax

// Layers

struct TileLayer {
    std::string name = "Layer";
    bool visible = true;

    // Sort key relative to the entity band (entities are at depth 0)
    // depth < 0 -> background, drawn behind entities
    // depth > 0 -> foreground, drawn over entities
    int depth = -10;

    // Camera scroll multiplier
    // 1.0 = moves with the camera
    // 0.0 = pinned to the screen, 0.5 = half speed (distant background)
    float parallax_x = 1.0f;
    float parallax_y = 1.0f;

    std::vector<int> tiles; // width * height; negative = empty cell
};

struct CollisionLayer {
    std::vector<int> cells; // width * height; 0 = open, non-zero = solid
};

// Conditions

// A predicate over the global variables store
// Recursive, so you can express "quest done AND has key" or "not yet opened".

// JSON forms:
// (absent) -> Always
// { "flag": "has_key" } -> flag is true
// { "flag": "has_key", "equals": false } -> flag is false
// { "int": "gold", "op": ">=", "value": 100 }
// { "all": [ {...}, {...} ] }
// { "any": [ {...}, {...} ] }
// { "not": {...} }
struct Condition {
    enum class Kind {
        Always,
        Flag,
        IntCompare,
        All,
        Any,
        Not
    };

    enum class Compare {
        Equal,
        NotEqual,
        Less,
        LessEqual,
        Greater,
        GreaterEqual
    };

    Kind kind = Kind::Always;

    std::string key; // Flag / IntCompare
    bool expect = true; // Flag; the value it must equal
    Compare compare = Compare::Equal; // IntCompare
    int value = 0; // IntCompare

    std::vector<Condition> children;  // All / Any / Not

    bool is_always() const { return kind == Kind::Always; }
};

// Actions

// An action is a type name plus a payload

// Known types:
// dialogue     { "lines": [ { "speaker": "...", "text": "..." } ] }
// set_flag     { "key": "...", "value": true }
// set_int      { "key": "...", "value": 3 }
// give_item    { "item": "potion", "count": 1 }
// start_battle { "encounter": "slime_pack" }
// change_map   { "map": "assets/maps/town.json", "spawn": {"x":0,"y":0} }
// move_actor   { "actor": "npc_guard", "to": {"x":5,"y":7} }
// wait         { "seconds": 0.5 }
struct TriggerAction {
    std::string type;
    nlohmann::json params = nlohmann::json::object();
};

// Triggers

enum class FireMode {
    OnEnter, // fires when the player steps into the region
    OnInteract // fires when the player faces the region and presses confirm
};

struct TriggerRegion {
    int x = 0;
    int y = 0;
    int w = 1;
    int h = 1;

    bool contains_tile(int tile_x, int tile_y) const {
        return tile_x >= x && tile_x < x + w &&
        tile_y >= y && tile_y < y + h;
    }
};

struct Trigger {
    std::string id;
    TriggerRegion region;
    FireMode fire_mode = FireMode::OnInteract;

    bool once = false;

    Condition condition; // Always by default
    std::vector<TriggerAction> actions;
};

// Actors (NPCs / placed entities)

enum class ActorBehavior {
    Static,
    Wander,
    Patrol,
    Follow
};

struct PathPoint {
    int x = 0;
    int y = 0;
};

struct ActorPlacement {
    std::string id;
    std::string sprite;

    // World pixels, matching entity's coordinate space
    float spawn_x = 0.0f;
    float spawn_y = 0.0f;

    ActorBehavior behavior = ActorBehavior::Static;
    std::vector<PathPoint> path; // tile waypoints, used by Patrol

    // Trigger id fired when the player interacts with this actor
    std::string interaction;

    // Entity setup; defaults mirror spetra::Entity
    int size = 16;
    float speed = 40.0f;
    int frame_width = 16;
    int frame_height = 16;
    int frame_count = 4;
};

// Map

struct MapData {
    std::string name = "Untitled";

    int width = 0;
    int height = 0;

    int default_tile_size = 16;
    std::optional<int> tile_size_override;

    std::string tileset_path;

    std::vector<TileLayer> layers;
    CollisionLayer collision;

    std::vector<Trigger> triggers;
    std::vector<ActorPlacement> actors;

    float spawn_x = 0.0f;
    float spawn_y = 0.0f;

    // Resolved tile size (override if present, else the default)
    int tile_size() const;

    int pixel_width() const;
    int pixel_height() const;

    bool in_bounds(int tile_x, int tile_y) const;

    // Collision lookup. Out-of-bounds reads as solid.
    bool is_tile_blocked(int tile_x, int tile_y) const;

    const Trigger* find_trigger(const std::string& id) const;
};

// Validation

// Errors mean the map is unusable
// Warnings mean it will load but something is probably wrong
struct MapValidation {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    bool ok() const { return errors.empty(); }
};

MapValidation validate_map(const MapData& map);
