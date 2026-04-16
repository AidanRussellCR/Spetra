#include "spetra/scene_manager.hpp"

#include <memory>

#include "spetra/scene.hpp"
#include "spetra/input.hpp"
#include "spetra/window.hpp"

namespace spetra {

    void SceneManager::change_scene(std::unique_ptr<Scene> scene) {
        if (m_current_scene) {
            m_current_scene->on_exit();
        }

        m_current_scene = std::move(scene);

        if (m_current_scene) {
            m_current_scene->on_enter();
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
