#include "spetra/variables.hpp"

namespace spetra {

    void Variables::set_flag(const std::string& key, bool value) {
        m_flags[key] = value;
    }

    bool Variables::flag(const std::string& key) const {
        auto it = m_flags.find(key);
        return it != m_flags.end() ? it->second : false;
    }

    bool Variables::has_flag(const std::string& key) const {
        return m_flags.find(key) != m_flags.end();
    }

    void Variables::set_int(const std::string& key, int value) {
        m_ints[key] = value;
    }

    int Variables::get_int(const std::string& key, int fallback) const {
        auto it = m_ints.find(key);
        return it != m_ints.end() ? it->second : fallback;
    }

    bool Variables::has_int(const std::string& key) const {
        return m_ints.find(key) != m_ints.end();
    }

    void Variables::set_str(const std::string& key, const std::string& value) {
        m_strings[key] = value;
    }

    std::string Variables::get_str(const std::string& key, const std::string& fallback) const {
        auto it = m_strings.find(key);
        return it != m_strings.end() ? it->second : fallback;
    }

    bool Variables::has_str(const std::string& key) const {
        return m_strings.find(key) != m_strings.end();
    }

    void Variables::clear() {
        m_flags.clear();
        m_ints.clear();
        m_strings.clear();
    }

    nlohmann::json Variables::to_json() const {
        nlohmann::json data;
        data["flags"] = m_flags;
        data["ints"] = m_ints;
        data["strings"] = m_strings;
        return data;
    }

    void Variables::load_json(const nlohmann::json& data) {
        clear();

        if (data.contains("flags")) {
            m_flags = data["flags"].get<std::unordered_map<std::string, bool>>();
        }
        if (data.contains("ints")) {
            m_ints = data["ints"].get<std::unordered_map<std::string, int>>();
        }
        if (data.contains("strings")) {
            m_strings = data["strings"].get<std::unordered_map<std::string, std::string>>();
        }
    }

} // namespace spetra
