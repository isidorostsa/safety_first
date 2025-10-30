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

    // Ledgers internal to this case
    std::map<uuid_t, std::set<uuid_t> > ledger_legal;
    std::map<uuid_t, std::set<uuid_t> > ledger_value;

    struct discerned_point {
        uuid_t value;
        uuid_t point;
    };

    struct discerned_io {
        std::vector<discerned_point> inputs;
        std::vector<discerned_point> outputs;

        auto inputs_view() {
            return std::views::transform(inputs, [](auto const &p) { return p.value; });
        }

        auto outputs_view() {
            return std::views::transform(outputs, [](auto const &p) { return p.value; });
        }
    };

    // map ( function name id, map ( function call id, { vector( preconditions ), vector ( postconditions ) } ) )
    std::map<uuid_t, std::map<uuid_t, discerned_io> > repeatability_table;

    // True and false nodes for this case
    static constexpr r _true = r(0);
    static constexpr r _false = r(1);

    explicit Case() : values(), current(values.begin()) {
        add_to_book(_true);
        add_to_book(_false);
    }

    void add_to_book(r const &t) {
        ledger_value.try_emplace(t.get_uuid());
    }

    std::set<uuid_t> reachable_nodes_from(uuid_t const uuid) {
        std::set<uuid_t> result;
        dfs(uuid, result, ledger_value);
        return result;
    }

    auto substitutable(r const &t1, r const &t2) {
        return substitutable(t1.get_uuid(), t2.get_uuid());
    }

    bool substitutable(uuid_t t1, uuid_t t2) {
        if (not ledger_value.contains(t1) or not ledger_value.contains(t2)) {
            throw;
        }

        ledger_value[t1].insert(t2);
        ledger_value[t2].insert(t1);

        // ReSharper disable once CppTooWideScopeInitStatement
        std::set<uuid_t> const reachables = reachable_nodes_from(t1);

        if (reachables.contains(_true.get_uuid()) and reachables.contains(_false.get_uuid())) {
            return false;
        }
        return true;
    }

    std::optional<bool> decided_direction(r const &g) {
        auto const reachables = reachable_nodes_from(g.get_uuid());
        if (reachables.contains(_true.get_uuid()) and reachables.contains(_false.get_uuid())) {
            throw;
        } else if (reachables.contains(_true.get_uuid())) {
            return true;
        } else if (reachables.contains(_false.get_uuid())) {
            return false;
        }
        return std::nullopt;
    }

    bool claim(r const &t) {
        return reachable_nodes_from(t.get_uuid()).contains(_true.get_uuid());
    }

    bool is_substitutable(uuid_t t1, uuid_t t2) {
        return reachable_nodes_from(t1).contains(t2);
    }

    bool are_calls_equivalent(std::vector<discerned_point> const &p1, std::vector<discerned_point> const &p2) {
        if (p1.size() != p2.size()) {
            return false;
        }

        return std::ranges::all_of(
            std::views::zip(p1, p2),
            [&](std::pair<discerned_point, discerned_point> const &p) {
                return is_substitutable(p.first.value, p.second.value) && p.first.point == p.second.point;
            });
    }

    std::set<uuid_t> get_equivalent_calls(const uuid_t function_name, const uuid_t function_call) {
        auto const &current_inputs = repeatability_table[function_name][function_call].inputs;

        std::set<uuid_t> result;

        for (auto &[other_function_call, other_table]: repeatability_table[function_name]) {
            if (other_function_call == function_call) continue;

            if (are_calls_equivalent(current_inputs, other_table.inputs)) {
                result.insert(other_function_call);
            }
        }

        return result;
    }

    void normalize_outputs_around(const uuid_t function_name, const uuid_t function_call) {
        auto current_table = repeatability_table[function_name][function_call];
        auto inputs = current_table.inputs;
        auto outputs_so_far = current_table.outputs;

        std::set<uuid_t> equivalent_calls = get_equivalent_calls(function_name, function_call);

        for (uuid_t const eq_call: equivalent_calls) {
            auto const& eq_outputs = repeatability_table[function_name][eq_call].outputs;

            if (outputs_so_far.size() <= eq_outputs.size()) {
                auto& current_last_discerned = outputs_so_far.back();
                auto& eq_discerned_element = eq_outputs[outputs_so_far.size() - 1];

                if (current_last_discerned.point != eq_discerned_element.point) {
                    std::println("Point mismatch: {} != {}", current_last_discerned.point, eq_discerned_element.point);
                }
                assert(current_last_discerned.point == eq_discerned_element.point);
                substitutable(current_last_discerned.value, eq_discerned_element.value);
            }

            for (int i = 0; i < std::min(outputs_so_far.size(), eq_outputs.size()); ++i) {
                assert(outputs_so_far[i].point == eq_outputs[i].point);
            }
        }
    }


    template<bool in_preconditions>
    void add_to_repeatability_table(const r &t, uuid_t function_name_uuid, uuid_t function_call_uuid,
                                    uuid_t code_point_uuid) {
        if constexpr (in_preconditions) {
            repeatability_table[function_name_uuid][function_call_uuid].inputs.push_back({
                .value = t.get_uuid(), .point = code_point_uuid
            });
        } else {
            repeatability_table[function_name_uuid][function_call_uuid].outputs.push_back({
                .value = t.get_uuid(), .point = code_point_uuid
            });
        }
    }

    template<bool in_preconditions>
    void discern(r const &d, uuid_t function_name, uuid_t function_call,
                 uuid_t code_point = get_code_point_uuid()) {
        add_to_book(d);
        add_to_repeatability_table<in_preconditions>(d, function_name, function_call, code_point);
        if constexpr (!in_preconditions) {
            normalize_outputs_around(function_name, function_call);
        }
    }

    r make_r(const uuid_t _uuid = get_new_uuid()) {
        r result(_uuid);
        add_to_book(result);
        return result;
    }

    r make_r_copy(r const &other, const uuid_t _uuid = get_new_uuid()) {
        r result(_uuid);
        add_to_book(result);
        substitutable(result, other);
        return result;
    }

    bool next(r const &guard) {
        add_to_book(guard);

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

    void clear_ledgers() {
        ledger_legal.clear();
        ledger_value.clear();
        repeatability_table.clear();
    }

    bool prepare_next_iteration() {
        clear_ledgers();
        add_to_book(_true);
        add_to_book(_false);
        return increment();
    }
};

template<>
struct std::formatter<Case::discerned_point> : std::formatter<std::string> {
    auto format(const Case::discerned_point& s, auto& ctx) const {
        return std::format_to(ctx.out(), "discerned_point({}, {})", s.point, s.value);
    }
};

template<>
struct std::formatter<Case::discerned_io> : std::formatter<std::string> {
    auto format(const Case::discerned_io& s, auto& ctx) const {
        return std::format_to(ctx.out(), "discerned_io({}, {})", s.inputs, s.outputs);
    }
};

template<>
struct std::formatter<Case::direction> : std::formatter<std::string_view> {
    auto format(Case::direction e, auto &ctx) const {
        constexpr std::string_view names[] = {"True", "False", "ForcedFalse"};
        return std::formatter<std::string_view>::format(names[static_cast<int>(e)], ctx);
    }
};
