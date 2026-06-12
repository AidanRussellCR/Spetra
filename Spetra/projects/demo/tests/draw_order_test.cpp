#include "draw_order.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using demo::DrawItem;

// Mirror the renderer's parallax math for the assertions below
static int dst(int world, float camera, float parallax) {
    return static_cast<int>(std::lround(world - camera * parallax));
}

int main() {
    // Scene like schema_test_map
    std::vector<DrawItem> v = {
        {-100, 0, 0, false}, // far_bg (layer 0)
        {-10,  0, 1, false}, // ground (layer 1)
        {10,   0, 2, false}, // overhang (layer 2)
        {0,    50, 0, true}, // player
    };
    demo::sort_draw_order(v);

    // Expected order: far_bg, ground, player, overhang
    assert(!v[0].is_entity && v[0].index == 0); // far_bg
    assert(!v[1].is_entity && v[1].index == 1); // ground
    assert( v[2].is_entity);                    // player between ground and overhang
    assert(!v[3].is_entity && v[3].index == 2); // overhang over the player
    std::cout << "ok: background < entity < foreground\n";

    // Entity y-sort: lower entity draws later
    std::vector<DrawItem> ents = {
        {0, 80, 0, true},  // A, feet-Y 80
        {0, 40, 1, true},  // B, feet-Y 40 (higher up)
        {0, 60, 2, true},  // C
    };
    demo::sort_draw_order(ents);
    assert(ents[0].index == 1 && ents[1].index == 2 && ents[2].index == 0);
    std::cout << "ok: entities sorted by feet_y (higher behind lower)\n";

    // Same-depth layers keep file order
    std::vector<DrawItem> layers = {
        {-10, 0, 0, false}, {-10, 0, 1, false}, {-10, 0, 2, false},
    };
    demo::sort_draw_order(layers);
    assert(layers[0].index == 0 && layers[1].index == 1 && layers[2].index == 2);
    std::cout << "ok: same-depth layers keep file order\n";

    // Parallax
    int off_bg     = 0 - dst(0, 100.0f, 0.5f); // = 50
    int off_ground = 0 - dst(0, 100.0f, 1.0f); // = 100
    int off_static = 0 - dst(0, 100.0f, 0.0f); // = 0
    assert(off_bg == 50 && off_ground == 100 && off_static == 0);
    assert(off_bg < off_ground);
    std::cout << "ok: parallax 0.5 scrolls at half the ground's speed; 0.0 is pinned\n";

    std::cout << "ALL DRAW-ORDER ASSERTIONS PASSED\n";
    return 0;
}
