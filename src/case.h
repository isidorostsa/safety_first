//
// Created by ros on 10/13/25.
//

#pragma once

#include <algorithm>
#include <vector>
#include <print>
#include <format>
#include <functional>
#include <map>
#include <ranges>
#include <stdexcept>

#include "util.h"
#include "r.h"
#include "books.h"
#include "ledger.h"
#include "proof_checker_defs.h"

struct r;

enum class direction : uint8_t {
    True,
    False,
    ForcedFalse
};

template<>
struct std::formatter<direction> : std::formatter<std::string_view> {
    auto format(direction e, auto &ctx) const {
        constexpr std::string_view names[] = {"True", "False", "ForcedFalse"};
        return std::formatter<std::string_view>::format(names[static_cast<int>(e)], ctx);
    }
};

struct Case {

    using enum direction;

    std::vector<direction> values;
    std::vector<direction>::iterator current;
    Ledger ledger;
    std::vector<std::source_location> branch_locations;

    // True and false nodes for this case
    static constexpr r _true  = r(value_uuid_t{0});
    static constexpr r _false = r(value_uuid_t{1});
    static constexpr r _void  = r(value_uuid_t{2});

    explicit Case() : values(), current(values.begin()) {
        ledger.track(_true.get_uuid());
        ledger.track(_false.get_uuid());
        ledger.track(_void.get_uuid());
    }

private:
    void track_variable(r const &t) {
        ledger.track(t.get_uuid());
    }


    void dump_contradiction(r const &g) {
        std::println("=== CONTRADICTION DETECTED ===");
        std::println("Value {} is substitutable with both _true and _false.",
                     ledger.describe_value(g.get_uuid()));
        std::println("");

        std::println("Current path: {}", values);
        std::println("Branch locations:");
        for (size_t i = 0; i < branch_locations.size(); ++i) {
            auto const& bl = branch_locations[i];
            std::println("  [{}] {} - {}:{}", i, values[i], bl.file_name(), bl.line());
        }
        std::println("");

        std::println("--- Path to _true ---");
        auto trace_true = ledger.trace_claim(g.get_uuid(), _true.get_uuid());
        ledger.dump_trace(trace_true);
        std::println("");

        std::println("--- Path to _false ---");
        auto trace_false = ledger.trace_claim(g.get_uuid(), _false.get_uuid());
        ledger.dump_trace(trace_false);
        std::println("");

        std::println("--- Equivalence classes ---");
        ledger.dump_graph();
        std::println("");

        std::println("--- Function call graph ---");
        ledger.dump_calls_graph();
    }

    [[nodiscard]] std::optional<bool> decided_direction(r const &g) {
        bool eq_true = ledger.is_substitutable_with(g.get_uuid(), _true.get_uuid());
        bool eq_false = ledger.is_substitutable_with(g.get_uuid(), _false.get_uuid());
        if (eq_true && eq_false) {
            dump_contradiction(g);
            throw std::runtime_error("Contradiction: value is substitutable with both _true and _false");
        } else if (eq_true) {
            return true;
        } else if (eq_false) {
            return false;
        }
        return std::nullopt;
    }

public:
    bool substitutable(r const &t1, r const &t2) {
        ledger.set_substitutable(t1.get_uuid(), t2.get_uuid());

        // Make sure the new addition did not make true == false
        return !ledger.is_substitutable_with(_true.get_uuid(), _false.get_uuid());
    }

    template<bool responsible>
    bool claim(r const &t) {
        if constexpr (responsible) {
            return ledger.is_substitutable_with(t.get_uuid(), _true.get_uuid());
        } else { // Other neighborhood responsible for this
            bool const not_false = not ledger.is_substitutable_with(t.get_uuid(), _false.get_uuid());

            if (not_false) {
                // This is definitely not sub with false given the condition,
                // so it should be valid to set to _true
                bool const valid = substitutable(t, _true);
                assert(valid);
            }

            return not_false;
        }
    }

    template<bool responsible>
    bool claim_false(r const &t) {
        if constexpr (responsible) {
            return ledger.is_substitutable_with(t.get_uuid(), _false.get_uuid());
        } else { // Other neighborhood responsible for this
            bool const not_true = not ledger.is_substitutable_with(t.get_uuid(), _true.get_uuid());

            if (not_true) {
                // This is definitely not sub with false given the condition,
                // so it should be valid to set to _true
                bool const valid = substitutable(t, _false);
                assert(valid);
            }

            return not_true;
        }
    }


    template<bool responsible>
    bool claim_equal_bool(r const& r1, r const& r2) {

        if (ledger.is_substitutable_with(r1.get_uuid(), r2.get_uuid())) {
            return true;
        }

        // They may be substitutable and we just haven't realized
        auto d1 = decided_direction(r1);
        auto d2 = decided_direction(r2);

        if constexpr (responsible) {
            // If either is unknown we can't show equality
            if (!d1 or !d2) { return false; }
            if (*d1 != *d2) { return false; }
        } else {
            // We don't have to show equality, just that there is no conflict
            if (d1 && d2) {
                if (*d1 != *d2) {
                    return false;
                }
            }
        }

        bool const valid = substitutable(r1, r2);
        // If this is not valid we should have returned false already
        assert(valid);

        return true;
    }


    template<bool in_preconditions>
    void discern(r const &d, function_point_t const& function_name, function_call_uuid_t const function_call,
                 code_point_uuid_t const code_point = get_code_point_uuid()) {
        ledger.track(d.get_uuid());

        // Map function_call_uuid_t to call_idx_t
        auto it = call_map.find(function_call);
        if (it == call_map.end()) {
            // First time seeing this call — register it
            auto idx = ledger.begin_call(function_name);
            call_map[function_call] = idx;
            it = call_map.find(function_call);
        }

        if constexpr (in_preconditions) {
            ledger.record_input(it->second, d.get_uuid());
        } else {
            ledger.record_output(it->second, d.get_uuid());
        }
    }

    r make_r(const char* var_name = "",
             const value_uuid_t _uuid = get_new_uuid(),
             std::source_location loc = std::source_location::current()) {
        r result(_uuid);
        ledger.track(result.get_uuid());
        ledger.set_definition_place(result.get_uuid(), loc, var_name);
        return result;
    }

    r make_r_copy(r const &other, const char* var_name = "",
                  const value_uuid_t _uuid = get_new_uuid(),
                  std::source_location loc = std::source_location::current()) {
        r result(_uuid);
        ledger.track(result.get_uuid());
        ledger.set_definition_place(result.get_uuid(), loc, var_name);
        ledger.set_substitutable(result.get_uuid(), other.get_uuid());
        return result;
    }

    bool next(r const &guard, std::source_location loc = std::source_location::current()) {
        if (current == values.end()) {
            expand(guard);
        }

        bool const dir = (*(current++) == True);

        substitutable(guard, dir ? _true : _false);

        branch_locations.push_back(loc);

        return dir;
    }

private:
    // Map from function_call_uuid_t to call_idx_t in the ledger
    std::map<function_call_uuid_t, call_idx_t> call_map;

    // If the direction is undecided, we decide false.
    void expand(r const &guard) {
        std::optional<bool> const known_dir = decided_direction(guard);

        values.emplace_back(
            known_dir.
            transform([](auto dir) {
                return dir ? True : ForcedFalse;
            }).value_or(False));
        current = values.end() - 1;
    }

public:
    /*
     * Returns false if we have reached the all-true path
     */
    bool increment() {
        auto i = std::ssize(values);
        while (--i >= 0 && values[i] != False) {
        }

        const bool not_finished = (i >= 0);

        if (not_finished) {
            values[i] = {True};
            values.resize(i + 1);
        }

        current = values.begin();

        return not_finished;
    }

    bool prepare_next_iteration() {
        ledger.clear_ledgers();
        ledger.track(_true.get_uuid());
        ledger.track(_false.get_uuid());
        ledger.track(_void.get_uuid());
        call_map.clear();
        reset_uuid_counter();
        reset_call_uuid_counter();
        branch_locations.clear();
        return increment();
    }
};
