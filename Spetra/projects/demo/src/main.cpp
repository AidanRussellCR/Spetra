#include "spetra/game.hpp"
#include "spetra/config.hpp"

int main() {
    spetra::AppConfig config;
    config.title = "Spetra Demo";
    config.width = 1280;
    config.height = 720;
    config.clear_color = {20, 24, 40, 255};

    spetra::Game game(config);
    return game.run();
}
