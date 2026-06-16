#include "event_runner.hpp"
#include "condition_eval.hpp"
#include "spetra/world.hpp"
#include <cassert>
#include <iostream>
using namespace demo;
using json = nlohmann::json;

// Host test
struct HostTest : EventHost {
    std::vector<std::string> log;
    bool dialogue_open = false;
    bool choice_open = false;
    int  choice_pick = 0;
    std::string battle, map;
    bool actor_moving = false;
    bool actor_known = true;

    void start_dialogue(const std::vector<spetra::DialogueLine>& lines) override {
        log.push_back("dialogue:" + lines[0].text);
        dialogue_open = true;
    }
    bool is_dialogue_active() const override { return dialogue_open; }
    void start_choice(const std::vector<std::string>& opts) override {
        log.push_back("choice:" + std::to_string(opts.size()));
        choice_open = true;
    }
    bool is_choice_active() const override { return choice_open; }
    int  choice_result() const override { return choice_pick; }
    void request_change_map(const std::string& m, bool, float, float) override {
        map = m; log.push_back("change_map:" + m);
    }
    void request_battle(const std::string& e) override {
        battle = e; log.push_back("battle:" + e);
    }
    bool move_actor(const std::string& id, int, int) override {
        log.push_back("move:" + id);
        if (!actor_known) return false;
        actor_moving = true; return true;
    }
    bool is_actor_moving(const std::string&) const override { return actor_moving; }
    void set_camera_mode(const std::string& m) override { log.push_back("cam:" + m); }
};

static TriggerAction act(const std::string& type, json params = json::object()) {
    TriggerAction a; a.type = type; a.params = params; return a;
}
static std::string joined(const HostTest& h) {
    std::string s; for (auto& e : h.log) s += e + " "; return s;
}

int main() {
    // Condition evaluation
    {
        spetra::Variables v;
        Condition always;
        assert(evaluate(always, v));

        Condition flag; flag.kind = Condition::Kind::Flag; flag.key = "has_key";
        assert(!evaluate(flag, v));
        v.set_flag("has_key", true);
        assert(evaluate(flag, v));
        flag.expect = false;
        assert(!evaluate(flag, v));

        Condition gold; gold.kind = Condition::Kind::IntCompare;
        gold.key = "gold"; gold.compare = Condition::Compare::GreaterEqual; gold.value = 100;
        assert(!evaluate(gold, v));
        v.set_int("gold", 150);
        assert(evaluate(gold, v));
        gold.compare = Condition::Compare::Less;
        assert(!evaluate(gold, v));

        // AND/OR/NOT
        Condition all; all.kind = Condition::Kind::All;
        Condition f2; f2.kind = Condition::Kind::Flag; f2.key = "has_key";
        Condition g2; g2.kind = Condition::Kind::IntCompare; g2.key = "gold";
        g2.compare = Condition::Compare::GreaterEqual; g2.value = 100;
        all.children = {f2, g2};
        assert(evaluate(all, v));
        v.set_int("gold", 5);
        assert(!evaluate(all, v));

        Condition any; any.kind = Condition::Kind::Any; any.children = {f2, g2};
        assert(evaluate(any, v));

        Condition neg; neg.kind = Condition::Kind::Not; neg.children = {f2};
        assert(!evaluate(neg, v));

        Condition empty_all; empty_all.kind = Condition::Kind::All;
        Condition empty_any; empty_any.kind = Condition::Kind::Any;
        assert(evaluate(empty_all, v) && !evaluate(empty_any, v));
        std::cout << "ok: condition evaluation (flag/int/all/any/not, vacuous cases)\n";
    }

    // Sequencing
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        r.run({ act("dialogue", json{{"lines", json::array({ json{{"text","hello"}} })}}), act("set_flag", json{{"key","after"},{"value",true}}) });

        assert(r.is_running());
        assert(joined(h) == "dialogue:hello ");
        assert(!w.variables.flag("after"));
        r.update(0.1); r.update(0.1);
        assert(!w.variables.flag("after"));

        h.dialogue_open = false;
        r.update(0.016);
        assert(w.variables.flag("after"));
        assert(!r.is_running());
        std::cout << "ok: dialogue blocks the queue until dismissed\n";
    }

    // wait timer
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        r.run({ act("wait", json{{"seconds", 0.5}}), act("set_flag", json{{"key","done"},{"value",true}}) });
        r.update(0.2); assert(!w.variables.flag("done"));
        r.update(0.2); assert(!w.variables.flag("done"));
        r.update(0.2); assert(w.variables.flag("done"));
        std::cout << "ok: wait blocks for its duration\n";
    }

    // choice branching
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        json opts = json::array({
            json{{"text","Yes"}, {"actions", json::array({ json{{"type","set_flag"},{"key","said_yes"},{"value",true}} })}},
            json{{"text","No"},  {"actions", json::array({ json{{"type","set_flag"},{"key","said_no"},{"value",true}} })}}
        });
        r.run({ act("choice", json{{"options", opts}, {"store","last_choice"}}), act("set_flag", json{{"key","after_choice"},{"value",true}}) });

        assert(joined(h) == "choice:2 ");
        h.choice_pick = 1;
        h.choice_open = false;
        r.update(0.016);

        assert(w.variables.flag("said_no"));
        assert(!w.variables.flag("said_yes"));
        assert(w.variables.get_int("last_choice") == 1);
        assert(w.variables.flag("after_choice"));
        std::cout << "ok: choice splices the selected branch, then continues\n";
    }

    // once-flag persistence
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        Trigger t; t.id = "chest_01"; t.once = true;
        t.actions = { act("set_flag", json{{"key","opened"},{"value",true}}) };

        std::string key = EventRunner::fired_key("town", "chest_01");
        assert(!w.variables.flag(key));
        r.run_trigger(t, "town");
        assert(w.variables.flag(key));
        assert(w.variables.flag("opened"));
        std::cout << "ok: 'once' recorded in the store as " << key << "\n";
    }

    // unknown action is skipped, not stalled
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        r.run({ act("mod.future_thing", json{{"wibble",42}}),
              act("set_flag", json{{"key","reached"},{"value",true}}) });
        assert(w.variables.flag("reached"));
        assert(!r.is_running());
        std::cout << "ok: unknown action skipped without stalling\n";
    }

    // unsupported move_actor doesn't hang the queue
    {
        HostTest h; spetra::World w; h.actor_known = false;
        EventRunner r(h); r.set_world(w);
        r.run({ act("move_actor", json{{"actor","ghost"},{"to", json{{"x",1},{"y",2}}}}),
              act("set_flag", json{{"key","moved_on"},{"value",true}}) });
        assert(w.variables.flag("moved_on"));
        std::cout << "ok: unknown actor continues instead of stalling\n";
    }

    // battle/change_map end the queue
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        r.run({ act("start_battle", json{{"encounter","slime_pack"}}),
              act("set_flag", json{{"key","should_not_run"},{"value",true}}) });
        assert(h.battle == "slime_pack");
        assert(!w.variables.flag("should_not_run"));
        assert(!r.is_running());
        std::cout << "ok: start_battle ends the queue\n";
    }

    // give_item/add_int
    {
        HostTest h; spetra::World w; EventRunner r(h); r.set_world(w);
        r.run({ act("give_item", json{{"item","potion"},{"count",2}}), act("give_item", json{{"item","potion"},{"count",3}}), act("add_int",   json{{"key","gold"},{"value",50}}) });
        assert(w.variables.get_int("item.potion") == 5);
        assert(w.variables.get_int("gold") == 50);
        std::cout << "ok: give_item accumulates; add_int increments\n";
    }

    std::cout << "ALL EVENT SYSTEM ASSERTIONS PASSED\n";
    return 0;
}
