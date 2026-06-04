#include "spetra/world.hpp"

namespace spetra {

    nlohmann::json World::to_json() const {
        nlohmann::json data;
        data["variables"] = variables.to_json();
        // Future: data["party"] = party.to_json(); etc.
        return data;
    }

    void World::load_json(const nlohmann::json& data) {
        if (data.contains("variables")) {
            variables.load_json(data["variables"]);
        }
    }

} // namespace spetra
