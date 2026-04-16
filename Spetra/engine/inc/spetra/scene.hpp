#pragma once

namespace spetra {

    class Input;
    class Window;
    class SceneManager;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void on_enter() {}
        virtual void on_exit() {}

        virtual void handle_input(Input& input, SceneManager& scene_manager) = 0;
        virtual void update(double delta_time, SceneManager& scene_manager) = 0;
        virtual void render(Window& window) = 0;
    };

} // namespace spetra
