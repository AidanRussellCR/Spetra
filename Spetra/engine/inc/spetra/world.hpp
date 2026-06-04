#pragma once

#include <nlohmann/json.hpp>

#include "spetra/variables.hpp"

namespace spetra {

    // Holds global variables now; party, inventory, and other persistent systems will live here too
    // This whole object is what save/load reads and writes
    class World {
    public:
        Variables variables;

        // Future home for:
        //   Party party;
        //   Inventory inventory;

        // Serialize/deserialize the entire world
        // Expand as new sub-systems are added so the save format grows in one place
        nlohmann::json to_json() const;
        void load_json(const nlohmann::json& data);
    };

} // namespace spetra
