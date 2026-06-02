#include "spetra/entity.hpp"

#include <cmath>
#include <iostream>

#include "spetra/window.hpp"

namespace spetra {

    bool Entity::load_sprite(SDL_Renderer* renderer, const std::string& path) {
        if (m_texture.load_from_file(renderer, path)) {
            m_texture_loaded = true;
            return true;
        }
        return false;
    }

    bool Entity::has_sprite() const {
        return m_texture_loaded;
    }

    void Entity::face_movement(float move_x, float move_y) {
        if (move_y > 0.0f) {
            direction = Direction::Down;
        }
        else if (move_x < 0.0f) {
            direction = Direction::Left;
        }
        else if (move_x > 0.0f) {
            direction = Direction::Right;
        }
        else if (move_y < 0.0f) {
            direction = Direction::Up;
        }
    }

    void Entity::animate(double delta_time, bool is_moving) {
        if (is_moving) {
            anim_timer += delta_time;

            if (anim_timer >= anim_frame_time) {
                anim_timer = 0.0;
                frame = (frame + 1) % frame_count;
            }
        }
        else {
            anim_timer = 0.0;
            frame = 0;
        }
    }

    void Entity::render(Window& window, float camera_x, float camera_y) const {
        int screen_x = static_cast<int>(std::lround(x - camera_x));
        int screen_y = static_cast<int>(std::lround(y - camera_y));

        if (m_texture_loaded) {
            int src_x = frame * frame_width;
            int src_y = static_cast<int>(direction) * frame_height;

            window.draw_texture_region(
                m_texture,
                src_x, src_y, frame_width, frame_height,
                screen_x, screen_y, frame_width, frame_height
            );
        }
        else {
            window.draw_filled_rect(fallback_color, screen_x, screen_y, size, size);
        }
    }

} // namespace spetra
