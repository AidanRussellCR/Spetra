#pragma once

#include <memory>
#include <string>
#include <vector>

#include "spetra/ui/label.hpp"
#include "spetra/ui/panel.hpp"
#include "spetra/ui/theme.hpp"

namespace spetra {

    class Window;

    // One page of dialogue
    struct DialogueLine {
        std::string speaker;
        std::string text;
    };

    // Dialogue Box
    class DialogueBox {
    public:
        DialogueBox();

        void set_skin_path(const std::string& path, int border_size);
        void set_font_path(const std::string& path, float point_size);

        // Share an external theme (overrides the internal one)
        void set_theme(ui::Theme& theme);

        void start(const std::vector<DialogueLine>& lines);
        void advance();
        void close();

        void update(double delta_time);
        void render(Window& window);

        bool is_active() const;

    private:
        void apply_current_line();
        const DialogueLine* current_line() const;
        ui::Theme& active_theme();

    private:
        std::vector<DialogueLine> m_lines;
        int m_current_index = 0;
        bool m_active = false;

        ui::Theme m_own_theme;
        ui::Theme* m_theme = nullptr; // points at m_own_theme unless overridden

        ui::Panel m_panel;
        ui::Label* m_speaker_label = nullptr; // owned by m_panel
        ui::Label* m_body_label = nullptr; // owned by m_panel
    };

} // namespace spetra
