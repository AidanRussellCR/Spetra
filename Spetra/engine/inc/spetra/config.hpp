#pragma once

#include <string>
#include "spetra/color.hpp"

namespace spetra {

    struct AppConfig {
        std::string title = "Spetra";

        // Window size on desktop
        int window_width = 1280;
        int window_height = 720;

        // Viewport in-game
        int render_width = 320;
        int render_height = 180;

        Color clear_color{30, 40, 60, 255};
    };

} // namespace spetra
