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

#include "util.h"
#include "r.h"
#include "books.h"
#include "ledger.h"
#include "proof_checker_defs.h"

struct r;

struct Case {
    enum class direction : uint8_t {
        True,
        False,
        ForcedFalse
    };

    using enum direction;

    std::vector<direction> values;
    std::vector<direction>::iterator current;
    Ledger ledger;

    // True and false nodes for this case
    static constexpr r _true = r(0);
    static constexpr r _false = r(1);
    static constexpr std::array true_and_false = {_true.get_uuid(), _false.get_uuid()};

    explicit Case() : values(), current(values.begin()) {
        ledger.track_substitutability(_true.get_uuid());
        ledger.track_substitutability(_false.get_uuid());
    }

private:
    void track_variable(r const &t) {
        ledger.track_substitutability(t.get_uuid());
    }


    std::optional<bool> decided_direction(r const &g) {
        auto const reachables = ledger.reachable_nodes_from(g.get_uuid());
        if (reachables.contains(_true.get_uuid()) and reachables.contains(_false.get_uuid())) {
            throw;
        } else if (reachables.contains(_true.get_uuid())) {
            return true;
        } else if (reachables.contains(_false.get_uuid())) {
            return false;
        }
        return std::nullopt;
    }

public:
    bool substitutable(r const &t1, r const &t2) {
        ledger.set_substitutable(t1.get_uuid(), t2.get_uuid());

        // Make sure the new addition did not make true == false
        return !std::ranges::includes(ledger.reachable_nodes_from(t1.get_uuid()), true_and_false);
    }

    bool claim(r const &t) const {
        return ledger.is_substitutable_with(t.get_uuid(), _true.get_uuid());
    }


    template<bool in_preconditions>
    void discern(r const &d, uuid_t function_name, uuid_t function_call,
                 uuid_t code_point = get_code_point_uuid()) {
        ledger.track_substitutability(d.get_uuid());
        ledger.track_repeatability<in_preconditions>(d, function_name, function_call, code_point);
        if constexpr (!in_preconditions) {
            ledger.normalize_outputs_around(function_name, function_call);
        }
    }

    r make_r(const uuid_t _uuid = get_new_uuid()) {
        r result(_uuid);
        ledger.track_substitutability(result.get_uuid());
        return result;
    }

    r make_r_copy(r const &other, const uuid_t _uuid = get_new_uuid()) {
        r result(_uuid);
        ledger.track_substitutability(result.get_uuid());
        ledger.set_substitutable(result.get_uuid(), other.get_uuid());
        return result;
    }

    template<bool in_preconditions>
    bool next(r const &guard, std::pair<uuid_t, uuid_t> const call_info) {
        auto const [function_name, function_call] = call_info;

        discern<in_preconditions>(guard, function_name, function_call);

        if (current == values.end()) {
            expand(guard);
        }
        bool const dir = (*(current++) == True);

        if (dir) {
            substitutable(guard, _true);
        } else {
            substitutable(guard, _false);
        }

        std::println("dir: {}", dir);

        return dir;
    }

private:
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
        ledger.track_substitutability(_true.get_uuid());
        ledger.track_substitutability(_false.get_uuid());
        return increment();
    }
};


template<>
struct std::formatter<Case::direction> : std::formatter<std::string_view> {
    auto format(Case::direction e, auto &ctx) const {
        constexpr std::string_view names[] = {"True", "False", "ForcedFalse"};
        return std::formatter<std::string_view>::format(names[static_cast<int>(e)], ctx);
    }
};
