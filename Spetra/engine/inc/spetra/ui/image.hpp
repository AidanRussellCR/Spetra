#pragma once

#include "spetra/texture.hpp"
#include "spetra/ui/widget.hpp"

namespace spetra::ui {

    class Image : public Widget {
    public:
        void set_texture(const Texture* texture,
                         int src_x = 0, int src_y = 0,
                         int src_w = 0, int src_h = 0);

        void size_to_source();

    protected:
        void render_self(Window& window) override;

    private:
        const Texture* m_texture = nullptr;
        int m_src_x = 0;
        int m_src_y = 0;
        int m_src_w = 0;
        int m_src_h = 0;
    };

} // namespace spetra::ui
