#pragma once

#include "spetra/ui/widget.hpp"

namespace spetra::ui {

    // While opening, the box scales from its center and children stay hidden; once fully open, children render normally
    class Panel : public Widget {
    public:
        // Tween speed in "open fraction per second"
        // Set instant() for no animation (menus, editor)
        double open_speed = 8.0;

        void open(); // begin expanding from 0
        void close(); // collapse (snaps shut immediately)
        void instant(); // fully open, no animation

        bool is_open_complete() const;
        double open_amount() const;

        void update(double delta_time) override;

    protected:
        void render_self(Window& window) override;
        bool children_visible() const override;

    private:
        void draw_nine_slice(Window& window, int x, int y, int w, int h);

        double m_open_amount = 1.0;
    };

} // namespace spetra::ui
