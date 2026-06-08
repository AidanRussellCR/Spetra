#include "spetra/ui/bar.hpp"

#include <algorithm>

#include "spetra/ui/theme.hpp"
#include "spetra/window.hpp"

namespace spetra::ui {

    void Bar::set_fraction(double fraction) {
        m_fraction = std::clamp(fraction, 0.0, 1.0);
    }

    void Bar::set_values(int current, int max_value) {
        if (max_value <= 0) {
            m_fraction = 0.0;
            return;
        }

        set_fraction(static_cast<double>(current) / static_cast<double>(max_value));
    }

    void Bar::render_self(Window& window) {
        Color bg = m_bg.value_or(m_theme ? m_theme->bar_bg : Color{40, 40, 50, 255});
        Color fill = m_fill.value_or(m_theme ? m_theme->bar_fill : Color{200, 80, 80, 255});

        window.draw_filled_rect(bg, rect.x, rect.y, rect.w, rect.h);

        int fill_w = static_cast<int>(rect.w * m_fraction);
        if (fill_w > 0) {
            window.draw_filled_rect(fill, rect.x, rect.y, fill_w, rect.h);
        }
    }

} // namespace spetra::ui
