#pragma once

#include <memory>
#include <vector>

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

        void push_scene(std::unique_ptr<Scene> scene);
        void pop_scene();
        void change_scene(std::unique_ptr<Scene> scene);

        void handle_input(Input& input);
        void update(double delta_time);
        void render(Window& window);

        bool has_scene() const;

    private:
        struct PendingOp {
            enum class Type { Push, Pop, Change } type;
            std::unique_ptr<Scene> scene; // used by Push & Change
        };

        void apply_pending();
        void enter_and_push(std::unique_ptr<Scene> scene);
        void clear_stack();

        std::vector<std::unique_ptr<Scene>> m_scenes;
        std::vector<PendingOp> m_pending;

        World* m_world = nullptr;
        Window* m_window = nullptr;
    };

} // namespace spetra
