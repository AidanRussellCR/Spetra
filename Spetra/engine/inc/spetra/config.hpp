#pragma once

#include <string>
#include "spetra/color.hpp"

namespace spetra {

    struct AppConfig {
        std::string title = "Spetra";
        int width = 1280;
        int height = 720;
        Color clear_color{30, 40, 60, 255};
    };

} // namespace spetra
