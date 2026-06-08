#pragma once

#include <memory>
#include <vector>

#include "spetra/direction.hpp"

namespace spetra {

    class Window;

    namespace ui {

        class Theme;

        struct Rect {
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            bool contains(int px, int py) const {
                return px >= x && px < x + w &&
                py >= y && py < y + h;
            }
        };

        // Rendering: render() draws this widget (render_self) and then its children
        // Subclasses implement render_self() and never have to remember to draw children
        class Widget {
        public:
            virtual ~Widget() = default;

            Rect rect;
            bool visible = true;

            // The child inherits this widget's theme
            Widget* add_child(std::unique_ptr<Widget> child);

            // Sets the theme on this widget and everything beneath it
            void set_theme(Theme* theme);
            Theme* theme() const { return m_theme; }

            // Position children
            virtual void layout();

            // Tweens/animation
            virtual void update(double delta_time);

            // Draws self, then children
            virtual void render(Window& window);

            // Return true if handled; UIRoot uses this to decide whether to bubble
            virtual bool on_confirm() { return false; }
            virtual bool on_cancel()  { return false; }
            virtual void on_move(Direction direction) { (void)direction; }

            // Coordinates are in render (logical) space
            virtual bool on_click(int mx, int my);

        protected:
            virtual void render_self(Window& window) = 0;

            // Override to hide children
            virtual bool children_visible() const { return true; }

            // Override to claim a click that landed on this widget itself
            virtual bool on_click_self(int mx, int my) { (void)mx; (void)my; return false; }

            void update_children(double delta_time);
            void render_children(Window& window);
            void layout_children();

            std::vector<std::unique_ptr<Widget>> m_children;
            Theme* m_theme = nullptr;
        };

    } // namespace ui
} // namespace spetra
