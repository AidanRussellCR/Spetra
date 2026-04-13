#pragma once

namespace spetra {

    class Input;
    class Window;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void on_enter() {}
        virtual void on_exit() {}

        virtual void handle_input(Input& input) = 0;
        virtual void update(double delta_time) = 0;
        virtual void render(Window& window) = 0;
    };

} // namespace spetra
