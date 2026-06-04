#pragma once

#include <memory>

namespace spetra {

    class Scene;
    class Input;
    class Window;
    class World;

    class SceneManager {
    public:
        // Provide the global World; call once at startup
        void set_world(World& world);

        void set_window(Window& window);
        void change_scene(std::unique_ptr<Scene> scene);

        void handle_input(Input& input);
        void update(double delta_time);
        void render(Window& window);

        bool has_scene() const;

    private:
        std::unique_ptr<Scene> m_current_scene;
        World* m_world = nullptr;
        Window* m_window = nullptr;
        bool m_needs_enter = false;
    };

} // namespace spetra
