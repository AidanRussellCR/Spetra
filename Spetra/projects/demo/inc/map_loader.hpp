#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "map_data.hpp"

// Map serialization

MapData map_from_json(const nlohmann::json& data);
nlohmann::json map_to_json(const MapData& map);

// asset_path is relative to the executable's asset directory
MapData load_map_from_json(const std::string& asset_path);
bool save_map_to_json(const MapData& map, const std::string& asset_path);

// Save to an absolute/relative filesystem path (used by tests and the editor)
bool save_map_to_file(const MapData& map, const std::string& full_path);
