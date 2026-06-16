#pragma once

#include <string>
#include <vector>

#include "spetra/dialogue_box.hpp"
#include "spetra/variables.hpp"
#include "spetra/world.hpp"
#include "map_data.hpp"

namespace demo {

    class EventHost {
    public:
        virtual ~EventHost() = default;

        // Dialogue
        virtual void start_dialogue(const std::vector<spetra::DialogueLine>& lines) = 0;
        virtual bool is_dialogue_active() const = 0;

        // Choice
        virtual void start_choice(const std::vector<std::string>& options) = 0;
        virtual bool is_choice_active() const = 0;
        virtual int  choice_result() const = 0; // index; -1 if cancelled

        // Scene transitions
        virtual void request_change_map(const std::string& map_path, bool has_spawn, float spawn_x, float spawn_y) = 0;
        virtual void request_battle(const std::string& encounter) = 0;

        // Cutscene primitives; return false if the actor is unknown or actors aren't implemented yet
        virtual bool move_actor(const std::string& actor_id, int tile_x, int tile_y) {
            (void)actor_id; (void)tile_x; (void)tile_y;
            return false;
        }
        virtual bool is_actor_moving(const std::string& actor_id) const {
            (void)actor_id;
            return false;
        }
        virtual void set_camera_mode(const std::string& mode) { (void)mode; }
    };

    // Executes a trigger's actions in sequence, across frames
    class EventRunner {
    public:
        explicit EventRunner(EventHost& host);

        void set_world(spetra::World& world);

        // Begin running a list of actions
        void run(const std::vector<TriggerAction>& actions);

        void run_trigger(const Trigger& trigger, const std::string& map_name);

        void update(double delta_time);

        bool is_running() const;
        void cancel();

        static std::string fired_key(const std::string& map_name, const std::string& trigger_id);

    private:
        enum class Wait {
            None, // ready to start the next action
            Dialogue, // until the dialogue box closes
            Choice, // until the menu returns a selection
            Timer, // until m_timer runs out
            ActorMove // until the actor stops moving
        };

        void start_next_action();
        void apply_action(const TriggerAction& action);
        void finish();

        EventHost& m_host;
        spetra::World* m_world = nullptr;

        std::vector<TriggerAction> m_queue;
        std::size_t m_index = 0;
        bool m_running = false;

        Wait m_wait = Wait::None;
        double m_timer = 0.0;
        std::string m_wait_actor;

        std::vector<std::vector<TriggerAction>> m_choice_branches;
        std::string m_choice_store_key;
    };

} // namespace demo
