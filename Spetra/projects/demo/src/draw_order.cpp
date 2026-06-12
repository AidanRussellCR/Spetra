#include "draw_order.hpp"

#include <algorithm>

namespace demo {

    void sort_draw_order(std::vector<DrawItem>& items) {
        std::stable_sort(items.begin(), items.end(),
            [](const DrawItem& a, const DrawItem& b) {
                if (a.depth != b.depth) {
                    return a.depth < b.depth;
                }
                return a.y_sort < b.y_sort;
            });
    }

} // namespace demo
