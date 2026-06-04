#pragma once

#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace spetra {

    // Global key/value state: story flags, counters, and named strings

    // Key convention (not enforced): namespace with dots
    // "story.chapter", "quest.rescued_cat", "demo.seen_intro"
    class Variables {
    public:
        // Flags (booleans); Unset keys read as false
        void set_flag(const std::string& key, bool value);
        bool flag(const std::string& key) const;
        bool has_flag(const std::string& key) const;

        // Integers; Unset keys read as the supplied fallback
        void set_int(const std::string& key, int value);
        int  get_int(const std::string& key, int fallback = 0) const;
        bool has_int(const std::string& key) const;

        // Strings; Unset keys read as the supplied fallback
        void set_str(const std::string& key, const std::string& value);
        std::string get_str(const std::string& key, const std::string& fallback = "") const;
        bool has_str(const std::string& key) const;

        // Wipe everything (like when starting a new game)
        void clear();

        // Serialization; Round-trips losslessly through JSON for save files
        nlohmann::json to_json() const;
        void load_json(const nlohmann::json& data);

    private:
        std::unordered_map<std::string, bool> m_flags;
        std::unordered_map<std::string, int> m_ints;
        std::unordered_map<std::string, std::string> m_strings;
    };

} // namespace spetra
