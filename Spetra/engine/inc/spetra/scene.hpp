#pragma once

namespace spetra {

    class Input;
    class Window;
    class SceneManager;
    class World;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void on_enter(Window& window) { (void)window; }
        virtual void on_exit() {}

        // Stack visibility policy where "blocks below" means scenes beneath this one are skipped for that phase
        // Defaults suit a full opaque scene that both freezes and hides everything under it; overlays override as needed
        virtual bool blocks_update_below() const { return true; }
        virtual bool blocks_render_below() const { return true; }

        virtual void handle_input(Input& input, SceneManager& scene_manager) = 0;
        virtual void update(double delta_time, SceneManager& scene_manager) = 0;
        virtual void render(Window& window) = 0;

    protected:
        // Global state, injected by SceneManager before on_enter
        // Available in every scene method
        // Include "spetra/world.hpp" where used
        World& world() const { return *m_world; }

    private:
        friend class SceneManager;
        World* m_world = nullptr;
    };

} // namespace spetra
