#pragma once

#include "spetra/variables.hpp"
#include "map_data.hpp"

namespace demo {

    // Evaluate a map Condition against the global variable store

    // Always     -> true
    // Flag       -> vars.flag(key) == expect
    // IntCompare -> vars.get_int(key) <op> value
    // All        -> every child true; true when empty
    // Any        -> some child true; false when empty
    // Not        -> negation of the first child; true when empty

    bool evaluate(const Condition& condition, const spetra::Variables& vars);

} // namespace demo
