#include "spetra/scene_manager.hpp"

#include <memory>

#include "spetra/scene.hpp"
#include "spetra/input.hpp"
#include "spetra/window.hpp"

namespace spetra {

    void SceneManager::set_world(World& world) {
        m_world = &world;

        // Keep an already-active scene's back-reference in sync regardless of call ordering
        if (m_current_scene) {
            m_current_scene->m_world = m_world;
        }
    }

    void SceneManager::set_window(Window& window) {
        m_window = &window;

        // A scene set before the window existed had its enter deferred
        if (m_current_scene && m_needs_enter) {
            m_current_scene->m_world = m_world;
            m_current_scene->on_enter(*m_window);
            m_needs_enter = false;
        }
    }

    void SceneManager::change_scene(std::unique_ptr<Scene> scene) {
        if (m_current_scene) {
            m_current_scene->on_exit();
        }

        m_current_scene = std::move(scene);

        if (m_current_scene) {
            if (m_window) {
                m_current_scene->on_enter(*m_window);
                m_needs_enter = false;
            }
            else {
                // No renderer yet; enter once set_window is called
                m_needs_enter = true;
            }
        }
    }

    void SceneManager::handle_input(Input& input) {
        if (m_current_scene) {
            m_current_scene->handle_input(input, *this);
        }
    }

    void SceneManager::update(double delta_time) {
        if (m_current_scene) {
            m_current_scene->update(delta_time, *this);
        }
    }

    void SceneManager::render(Window& window) {
        if (m_current_scene) {
            m_current_scene->render(window);
        }
    }

    bool SceneManager::has_scene() const {
        return m_current_scene != nullptr;
    }

} // namespace spetra
