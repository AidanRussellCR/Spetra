#pragma once

#include <vector>

namespace demo {

    // One item to draw in the map's single depth-sorted pass
    struct DrawItem {
        int depth = 0;
        int y_sort = 0;
        int index = 0;
        bool is_entity = false;
    };

    // depth ascending, then y_sort ascending
    // tile layers that share a depth keep their original (file) order
    // entities sharing a feet_y keep insertion order
    // entities land between background layers (depth < 0) and foreground layers (depth > 0)
    void sort_draw_order(std::vector<DrawItem>& items);

} // namespace demo
