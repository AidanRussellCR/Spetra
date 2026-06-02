#pragma once

#include <string>

#include "spetra/color.hpp"
#include "spetra/texture.hpp"

namespace spetra {

    class Window;

    enum class Direction {
        Down = 0,
        Left = 1,
        Right = 2,
        Up = 3
    };

    // A general moving, animated, drawable thing on a map

    // Collision is decided by map, not entity

    class Entity {
    public:
        // Position is the top-left of the collision box, in world pixels
        // The collision box is square (size * size)
        // Drawn sprite frame may be larger (16px sprite over a 12px box)

        float x = 0.0f;
        float y = 0.0f;
        int   size = 16;
        float speed = 80.0f;

        // Sprite sheet: frames laid out as columns (frame) * rows (direction)
        int frame_width = 16;
        int frame_height = 16;

        // Animation state
        Direction direction = Direction::Down;
        int    frame = 0;
        int    frame_count = 4;
        double anim_timer = 0.0;
        double anim_frame_time = 0.15;

        // Drawn when no sprite is loaded
        Color fallback_color{255, 220, 120, 255};

    public:
        Entity() = default;

        bool load_sprite(SDL_Renderer* renderer, const std::string& path);
        bool has_sprite() const;

        // Set facing from a movement vector (signs only; magnitude ignored)
        void face_movement(float move_x, float move_y);

        // Advance the walk cycle while moving; reset to the idle frame otherwise
        void animate(double delta_time, bool is_moving);

        void render(Window& window, float camera_x, float camera_y) const;

    private:
        Texture m_texture;
        bool m_texture_loaded = false;
    };

} // namespace spetra
