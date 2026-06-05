#include "spetra/scene_manager.hpp"

#include <utility>

#include "spetra/scene.hpp"
#include "spetra/input.hpp"
#include "spetra/window.hpp"

namespace spetra {

    void SceneManager::set_world(World& world) {
        m_world = &world;

        // Keep an already-active scene's back-reference in sync regardless of call ordering
        for (auto& scene : m_scenes) {
            scene->m_world = m_world;
        }
    }

    void SceneManager::set_window(Window& window) {
        m_window = &window;

        apply_pending(); // enter scenes queued before window was made
    }

    void SceneManager::push_scene(std::unique_ptr<Scene> scene) {
        m_pending.push_back({PendingOp::Type::Push, std::move(scene)});
    }

    void SceneManager::pop_scene() {
        m_pending.push_back({PendingOp::Type::Pop, nullptr});
    }

    void SceneManager::change_scene(std::unique_ptr<Scene> scene) {
        m_pending.push_back({PendingOp::Type::Change, std::move(scene)});
    }

    void SceneManager::handle_input(Input& input) {
        // Only the top scene receives input; scenes beneath are paused
        if (!m_scenes.empty()) {
            m_scenes.back()->handle_input(input, *this);
        }
        apply_pending();
    }

    void SceneManager::update(double delta_time) {
        // Walk top-down, updating until a scene blocks the ones below it
        for (std::size_t i = m_scenes.size(); i-- > 0; ) {
            m_scenes[i]->update(delta_time, *this);
            if (m_scenes[i]->blocks_update_below()) {
                break;
            }
        }
        apply_pending();
    }

    void SceneManager::render(Window& window) {
        if (m_scenes.empty()) {
            return;
        }

        // Find the lowest visible scene; top one hides everything beneath it
        std::size_t start = 0;
        for (std::size_t i = m_scenes.size(); i-- > 0; ) {
            if (m_scenes[i]->blocks_render_below()) {
                start = i;
                break;
            }
        }

        for (std::size_t i = start; i < m_scenes.size(); ++i) {
            m_scenes[i]->render(window);
        }

        // Manager owns the single present() so stacked layers compose into one frame
        window.present();
    }

    bool SceneManager::has_scene() const {
        return !m_scenes.empty();
    }

    void SceneManager::apply_pending() {
        if (!m_window) {
            return; // can't enter scenes without a renderer yet
        }

        // Move the queue out so ops enqueued during on_enter/on_exit are handled next pass
        std::vector<PendingOp> ops = std::move(m_pending);
        m_pending.clear();

        for (auto& op : ops) {
            switch (op.type) {
                case PendingOp::Type::Push:
                    enter_and_push(std::move(op.scene));
                    break;

                case PendingOp::Type::Pop:
                    if (!m_scenes.empty()) {
                        m_scenes.back()->on_exit();
                        m_scenes.pop_back();
                    }
                    break;

                case PendingOp::Type::Change:
                    clear_stack();
                    enter_and_push(std::move(op.scene));
                    break;
            }
        }
    }

    void SceneManager::enter_and_push(std::unique_ptr<Scene> scene) {
        if (!scene) {
            return;
        }

        Scene* raw = scene.get();
        raw->m_world = m_world; // inject global state
        m_scenes.push_back(std::move(scene));
        raw->on_enter(*m_window);
    }

    void SceneManager::clear_stack() {
        // Exit top-down.
        while (!m_scenes.empty()) {
            m_scenes.back()->on_exit();
            m_scenes.pop_back();
        }
    }

} // namespace spetra
