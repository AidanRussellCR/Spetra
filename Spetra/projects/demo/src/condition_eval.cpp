#include "condition_eval.hpp"

namespace demo {

    namespace {

        bool compare_ints(int lhs, Condition::Compare op, int rhs) {
            switch (op) {
                case Condition::Compare::Equal: return lhs == rhs;
                case Condition::Compare::NotEqual: return lhs != rhs;
                case Condition::Compare::Less: return lhs <  rhs;
                case Condition::Compare::LessEqual: return lhs <= rhs;
                case Condition::Compare::Greater: return lhs >  rhs;
                case Condition::Compare::GreaterEqual: return lhs >= rhs;
            }
            return false;
        }

    } // namespace

    bool evaluate(const Condition& condition, const spetra::Variables& vars) {
        switch (condition.kind) {
            case Condition::Kind::Always:
                return true;

            case Condition::Kind::Flag:
                return vars.flag(condition.key) == condition.expect;

            case Condition::Kind::IntCompare:
                return compare_ints(vars.get_int(condition.key), condition.compare, condition.value);

            case Condition::Kind::All:
                for (const Condition& child : condition.children) {
                    if (!evaluate(child, vars)) {
                        return false;
                    }
                }
                return true; // true when empty

            case Condition::Kind::Any:
                for (const Condition& child : condition.children) {
                    if (evaluate(child, vars)) {
                        return true;
                    }
                }
                return false; // false when empty

            case Condition::Kind::Not:
                if (condition.children.empty()) {
                    return true; // nothing to negate
                }
                return !evaluate(condition.children.front(), vars);
        }

        return true;
    }

} // namespace demo
