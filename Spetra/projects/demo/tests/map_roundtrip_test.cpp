#include "map_loader.hpp"
#include "map_data.hpp"
#include "spetra/filesystem.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace spetra {
    std::string get_base_path() { return ""; }
    std::string get_asset_path(const std::string& relative_path) { return relative_path; }
}

#ifndef TEST_MAP_INPUT
#define TEST_MAP_INPUT "assets/maps/schema_test_map.json"
#endif
#ifndef TEST_MAP_OUT_DIR
#define TEST_MAP_OUT_DIR "."
#endif

using json = nlohmann::json;

namespace {

    int g_failures = 0;

    void check(bool cond, const std::string& what) {
        if (cond) {
            std::cout << "  ok   " << what << '\n';
        }
        else {
            std::cout << "  FAIL " << what << '\n';
            ++g_failures;
        }
    }

    std::string read_file(const std::string& path) {
        std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    std::string out_path(const std::string& name) {
        return std::string(TEST_MAP_OUT_DIR) + "/" + name;
    }

} // namespace

int main() {
    const std::string input = TEST_MAP_INPUT;
    const std::string out1 = out_path("roundtrip_pass1.json");
    const std::string out2 = out_path("roundtrip_pass2.json");

    std::cout << "Round-trip test\n  input: " << input << "\n\n";

    // Load the source map
    MapData m1 = load_map_from_file(input);

    check(m1.width > 0 && m1.height > 0, "source map loaded");
    if (m1.width <= 0) {
        std::cout << "\nCannot proceed: input map failed to load.\n";
        return 1;
    }
    check(validate_map(m1).ok(), "source map validates clean");

    // Disk cycle; save -> load -> save
    check(save_map_to_file(m1, out1), "save pass 1");

    MapData m2 = load_map_from_file(out1);
    check(m2.width > 0, "reload of saved map");

    check(save_map_to_file(m2, out2), "save pass 2");

    // The two canonical files are byte-identical
    std::string bytes1 = read_file(out1);
    std::string bytes2 = read_file(out2);
    check(!bytes1.empty(), "saved file is non-empty");
    check(bytes1 == bytes2, "load->save->load->save is byte-stable on disk");

    // Same thing at the JSON layer; the canonical form is a fixed point
    check(map_to_json(m1).dump(2) == map_to_json(m2).dump(2),
          "canonical JSON is a fixed point");

    // Field survival across the disk cycle
    check(m2.name == m1.name, "name survives");
    check(m2.width == m1.width && m2.height == m1.height, "dimensions survive");
    check(m2.tile_size() == m1.tile_size(), "tile size survives");
    check(m2.tileset_path == m1.tileset_path, "tileset path survives");
    check(m2.spawn_x == m1.spawn_x && m2.spawn_y == m1.spawn_y, "spawn survives");

    check(m2.layers.size() == m1.layers.size(), "layer count survives");
    if (m2.layers.size() == m1.layers.size()) {
        bool layers_ok = true;
        for (std::size_t i = 0; i < m2.layers.size(); ++i) {
            const TileLayer& a = m1.layers[i];
            const TileLayer& b = m2.layers[i];
            if (a.name != b.name || a.visible != b.visible ||
                a.depth != b.depth ||
                a.parallax_x != b.parallax_x || a.parallax_y != b.parallax_y ||
                a.tiles != b.tiles) {
                layers_ok = false;
                }
        }
        check(layers_ok, "layer name/depth/parallax/visible/tiles survive");
    }

    check(m2.collision.cells == m1.collision.cells, "collision cells survive");

    check(m2.triggers.size() == m1.triggers.size(), "trigger count survives");
    if (!m2.triggers.empty()) {
        const Trigger& t = m2.triggers[0];
        check(!t.id.empty(), "trigger id survives");
        check(t.once == m1.triggers[0].once, "trigger 'once' survives");
        check(t.fire_mode == m1.triggers[0].fire_mode, "trigger fire_mode survives");
        check(t.region.w == m1.triggers[0].region.w &&
        t.region.h == m1.triggers[0].region.h, "trigger region survives");
        check(!t.actions.empty() && !t.actions[0].type.empty(), "trigger actions survive");
    }

    // A nested all/any condition somewhere in the map survives its structure
    {
        bool found_nested = false;
        for (const Trigger& t : m2.triggers) {
            if (t.condition.kind == Condition::Kind::All ||
                t.condition.kind == Condition::Kind::Any) {
                found_nested = !t.condition.children.empty();
                }
        }
        check(found_nested, "nested all/any condition survives (if present)");
    }

    check(m2.actors.size() == m1.actors.size(), "actor count survives");
    if (!m2.actors.empty()) {
        const ActorPlacement& a = m2.actors[0];
        check(a.behavior == m1.actors[0].behavior, "actor behavior survives");
        check(a.path.size() == m1.actors[0].path.size(), "actor path survives");
        check(a.interaction == m1.actors[0].interaction, "actor interaction survives");
    }

    // Forward-compatibility; an unknown action type round-trips intact
    {
        MapData synthetic = m1;
        Trigger probe;
        probe.id = "roundtrip_probe";
        probe.region = TriggerRegion{0, 0, 1, 1};
        TriggerAction unknown;
        unknown.type = "mod.some_future_action";
        unknown.params["wibble"] = 42;
        unknown.params["nested"] = json::object();
        unknown.params["nested"]["deep"] = "value";
        probe.actions.push_back(unknown);
        synthetic.triggers.push_back(probe);

        json a = map_to_json(synthetic);
        MapData reparsed = map_from_json(a);
        json b = map_to_json(reparsed);

        check(a.dump(2) == b.dump(2), "map with unknown action is a fixed point");

        const Trigger* probe2 = reparsed.find_trigger("roundtrip_probe");
        bool preserved = probe2 && probe2->actions.size() == 1 &&
        probe2->actions[0].type == "mod.some_future_action" &&
        probe2->actions[0].params.value("wibble", 0) == 42;
        check(preserved, "unknown action type + payload preserved");
    }

    std::cout << '\n';
    if (g_failures == 0) {
        std::cout << "PASS: all round-trip checks succeeded\n";
        return 0;
    }

    std::cout << "FAIL: " << g_failures << " check(s) failed\n";
    return 1;
}
