#pragma once

#include <optional>

#include "spetra/color.hpp"
#include "spetra/ui/widget.hpp"

namespace spetra::ui {

    // A fill bar (HP/MP/XP); fraction is clamped to [0,1]
    // Colors fall back to Theme::bar_bg / Theme::bar_fill when not overridden
    class Bar : public Widget {
    public:
        void set_fraction(double fraction);
        double fraction() const { return m_fraction; }

        void set_values(int current, int max_value);

        void set_bg_color(const Color& color)   { m_bg = color; }
        void set_fill_color(const Color& color) { m_fill = color; }

    protected:
        void render_self(Window& window) override;

    private:
        double m_fraction = 1.0;
        std::optional<Color> m_bg;
        std::optional<Color> m_fill;
    };

} // namespace spetra::ui
