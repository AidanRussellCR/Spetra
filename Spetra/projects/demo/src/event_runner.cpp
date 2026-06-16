#include "event_runner.hpp"

#include <iostream>

#include "spetra/world.hpp"

namespace demo {

    EventRunner::EventRunner(EventHost& host)
    : m_host(host) {
    }

    void EventRunner::set_world(spetra::World& world) {
        m_world = &world;
    }

    std::string EventRunner::fired_key(const std::string& map_name, const std::string& trigger_id) {
        return "trigger." + map_name + "." + trigger_id;
    }

    void EventRunner::run(const std::vector<TriggerAction>& actions) {
        if (!m_world) {
            std::cerr << "[EventRunner] no World set; call set_world() first\n";
            return;
        }

        m_queue = actions;
        m_index = 0;
        m_running = !m_queue.empty();
        m_wait = Wait::None;
        m_timer = 0.0;
        m_wait_actor.clear();
        m_choice_branches.clear();
        m_choice_store_key.clear();

        if (m_running) {
            start_next_action();
        }
    }

    void EventRunner::run_trigger(const Trigger& trigger, const std::string& map_name) {
        // Mark a one-shot as fired on start; cutscene must not be able to re-trigger itself while it runs, and a quit mid-cutscene shouldn't replay it on reload
        if (trigger.once && !trigger.id.empty()) {
            m_world->variables.set_flag(fired_key(map_name, trigger.id), true);
        }

        run(trigger.actions);
    }

    bool EventRunner::is_running() const {
        return m_running;
    }

    void EventRunner::cancel() {
        m_queue.clear();
        m_index = 0;
        m_running = false;
        m_wait = Wait::None;
        m_choice_branches.clear();
    }

    void EventRunner::finish() {
        m_running = false;
        m_wait = Wait::None;
        m_queue.clear();
        m_index = 0;
    }

    void EventRunner::update(double delta_time) {
        if (!m_running) {
            return;
        }

        switch (m_wait) {
            case Wait::None:
                break;

            case Wait::Dialogue:
                if (m_host.is_dialogue_active()) {
                    return;
                }
                m_wait = Wait::None;
                break;

            case Wait::Choice: {
                if (m_host.is_choice_active()) {
                    return;
                }

                int selected = m_host.choice_result();
                m_wait = Wait::None;

                if (!m_choice_store_key.empty()) {
                    m_world->variables.set_int(m_choice_store_key, selected);
                }

                if (selected >= 0 &&
                    selected < static_cast<int>(m_choice_branches.size())) {
                    const std::vector<TriggerAction>& branch =
                        m_choice_branches[static_cast<std::size_t>(selected)];

                    if (!branch.empty()) {
                        m_queue.insert(m_queue.begin() + static_cast<std::ptrdiff_t>(m_index),
                                       branch.begin(), branch.end());
                    }
                }

                m_choice_branches.clear();
                m_choice_store_key.clear();
                break;
            }

            case Wait::Timer:
                m_timer -= delta_time;
                if (m_timer > 0.0) {
                    return;
                }
                m_wait = Wait::None;
                break;

            case Wait::ActorMove:
                if (m_host.is_actor_moving(m_wait_actor)) {
                    return;
                }
                m_wait = Wait::None;
                m_wait_actor.clear();
                break;
        }

        start_next_action();
    }

    void EventRunner::start_next_action() {
        // Run actions back-to-back until one blocks
        while (m_running && m_wait == Wait::None) {
            if (m_index >= m_queue.size()) {
                finish();
                return;
            }

            const TriggerAction action = m_queue[m_index];
            ++m_index;

            apply_action(action);
        }
    }

    void EventRunner::apply_action(const TriggerAction& action) {
        const nlohmann::json& p = action.params;

        if (action.type == "dialogue") {
            std::vector<spetra::DialogueLine> lines;

            if (p.contains("lines")) {
                for (const auto& line_json : p["lines"]) {
                    spetra::DialogueLine line;
                    line.speaker = line_json.value("speaker", std::string(""));
                    line.text = line_json.value("text", std::string(""));
                    lines.push_back(line);
                }
            }

            if (lines.empty()) {
                return; // nothing to show; fall through to the next action
            }

            m_host.start_dialogue(lines);
            m_wait = Wait::Dialogue;
            return;
        }

        if (action.type == "choice") {
            std::vector<std::string> options;
            m_choice_branches.clear();

            if (p.contains("options")) {
                for (const auto& opt : p["options"]) {
                    options.push_back(opt.value("text", std::string("...")));

                    std::vector<TriggerAction> branch;
                    if (opt.contains("actions")) {
                        for (const auto& sub : opt["actions"]) {
                            TriggerAction sa;
                            sa.type = sub.value("type", std::string(""));
                            sa.params = sub;
                            sa.params.erase("type");
                            branch.push_back(sa);
                        }
                    }
                    m_choice_branches.push_back(branch);
                }
            }

            if (options.empty()) {
                m_choice_branches.clear();
                return;
            }

            m_choice_store_key = p.value("store", std::string(""));
            m_host.start_choice(options);
            m_wait = Wait::Choice;
            return;
        }

        if (action.type == "set_flag") {
            m_world->variables.set_flag(p.value("key", std::string("")), p.value("value", true));
            return;
        }

        if (action.type == "set_int") {
            m_world->variables.set_int(p.value("key", std::string("")), p.value("value", 0));
            return;
        }

        if (action.type == "set_str") {
            m_world->variables.set_str(p.value("key", std::string("")), p.value("value", std::string("")));
            return;
        }

        if (action.type == "add_int") {
            std::string key = p.value("key", std::string(""));
            int delta = p.value("value", 0);
            m_world->variables.set_int(key, m_world->variables.get_int(key) + delta);
            return;
        }

        if (action.type == "give_item") {
            std::string item = p.value("item", std::string(""));
            int count = p.value("count", 1);
            if (!item.empty()) {
                std::string key = "item." + item;
                m_world->variables.set_int(key, m_world->variables.get_int(key) + count);
            }
            return;
        }

        if (action.type == "wait") {
            double seconds = p.value("seconds", 0.0);
            if (seconds <= 0.0) {
                return;
            }
            m_timer = seconds;
            m_wait = Wait::Timer;
            return;
        }

        if (action.type == "change_map") {
            std::string map_path = p.value("map", std::string(""));
            bool has_spawn = p.contains("spawn");
            float sx = 0.0f;
            float sy = 0.0f;
            if (has_spawn) {
                sx = p["spawn"].value("x", 0.0f);
                sy = p["spawn"].value("y", 0.0f);
            }

            m_host.request_change_map(map_path, has_spawn, sx, sy);
            finish(); // the scene is going away; stop here
            return;
        }

        if (action.type == "start_battle") {
            m_host.request_battle(p.value("encounter", std::string("")));
            finish(); // battle is pushed; remaining actions would run under it
            return;
        }

        if (action.type == "move_actor") {
            std::string actor_id = p.value("actor", std::string(""));
            int tx = 0;
            int ty = 0;
            if (p.contains("to")) {
                tx = p["to"].value("x", 0);
                ty = p["to"].value("y", 0);
            }

            if (m_host.move_actor(actor_id, tx, ty)) {
                m_wait_actor = actor_id;
                m_wait = Wait::ActorMove;
            }

            return;
        }

        if (action.type == "camera_mode") {
            m_host.set_camera_mode(p.value("mode", std::string("follow")));
            return;
        }

        // Warn and skip unknown types
        std::cerr << "[EventRunner] unknown action type '" << action.type << "' (skipped)\n";
    }

} // namespace demo
