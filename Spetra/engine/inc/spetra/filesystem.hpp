#pragma once

#include <string>

namespace spetra {

    std::string get_base_path();
    std::string get_asset_path(const std::string& relative_path);

}
