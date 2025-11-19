#pragma once

#include <algorithm>
#include <set>
#include <map>
#include <format>

#include "uuid.h"

class Ledger {

    // Ledgers internal to this case
    std::map<uuid_t, std::set<uuid_t> > ledger_stability;
    std::map<uuid_t, std::set<uuid_t> > ledger_substitutability;

public:
    struct discerned_point {
        uuid_t value;
        uuid_t point;
    };

    struct discerned_io {
        std::vector<discerned_point> inputs;
        std::vector<discerned_point> outputs;
    };
private:

    // map ( function name id, map ( function call id, { vector( preconditions ), vector ( postconditions ) } ) )
    std::map<uuid_t, std::map<uuid_t, discerned_io> > ledger_repeatability;

    static inline void dfs(uuid_t const uuid, std::set<uuid_t>& reachables, std::map<uuid_t, std::set<uuid_t>> const& ledger_value) {
        if (reachables.contains(uuid)) return;

        reachables.insert(uuid);

        if (!ledger_value.contains(uuid)) return;

        for (uuid_t const other_uuid: ledger_value.at(uuid)) {
            dfs(other_uuid, reachables, ledger_value);
        }
    }



public:
    void track_substitutability(uuid_t const t) {
        ledger_substitutability.try_emplace(t);
    }


    void set_substitutable(uuid_t t1, uuid_t t2) {
        if (not ledger_substitutability.contains(t1) or not ledger_substitutability.contains(t2)) {
            throw;
        }

        ledger_substitutability[t1].insert(t2);
        ledger_substitutability[t2].insert(t1);
    }

    std::set<uuid_t> reachable_nodes_from(uuid_t const uuid) const {
        std::set<uuid_t> result;
        dfs(uuid, result, ledger_substitutability);
        return result;
    }

    bool is_substitutable_with(uuid_t const source, uuid_t const dest) const {
        return reachable_nodes_from(source).contains(dest);
    }

    void clear_ledgers() {
        ledger_stability.clear();
        ledger_substitutability.clear();
        ledger_repeatability.clear();
    }

private:
    bool are_calls_equivalent(std::vector<discerned_point> const &p1, std::vector<discerned_point> const &p2) {
        if (p1.size() != p2.size()) {
            return false;
        }

        return std::ranges::all_of(
            std::views::zip(p1, p2),
            [&](std::pair<discerned_point, discerned_point> const &p) {
                return is_substitutable_with(p.first.value, p.second.value) && p.first.point == p.second.point;
            });
    }

    std::set<uuid_t> get_equivalent_calls(const uuid_t function_name, const uuid_t function_call) {
        auto const &current_inputs = ledger_repeatability[function_name][function_call].inputs;

        std::set<uuid_t> result;

        for (auto &[other_function_call, other_table]: ledger_repeatability[function_name]) {
            if (other_function_call == function_call) continue;

            if (are_calls_equivalent(current_inputs, other_table.inputs)) {
                result.insert(other_function_call);
            }
        }

        return result;
    }


public:
    void normalize_outputs_around(const uuid_t function_name, const uuid_t function_call) {
        auto current_table = ledger_repeatability[function_name][function_call];
        auto inputs = current_table.inputs;
        auto outputs_so_far = current_table.outputs;

        std::set<uuid_t> equivalent_calls = get_equivalent_calls(function_name, function_call);

        for (uuid_t const eq_call: equivalent_calls) {
            auto const& eq_outputs = ledger_repeatability[function_name][eq_call].outputs;

            if (outputs_so_far.size() <= eq_outputs.size()) {
                auto& current_last_discerned = outputs_so_far.back();
                auto& eq_discerned_element = eq_outputs[outputs_so_far.size() - 1];

                if (current_last_discerned.point != eq_discerned_element.point) {
                    std::println("Point mismatch: {} != {}", current_last_discerned.point, eq_discerned_element.point);
                }
                assert(current_last_discerned.point == eq_discerned_element.point);
                set_substitutable(current_last_discerned.value, eq_discerned_element.value);
            }

            for (int i = 0; i < std::min(outputs_so_far.size(), eq_outputs.size()); ++i) {
                assert(outputs_so_far[i].point == eq_outputs[i].point);
            }
        }
    }

    template<bool in_preconditions>
    void track_repeatability(const r &t, const uuid_t function_name, const uuid_t function_call,
                                    const uuid_t code_point) {
        if constexpr (in_preconditions) {
            ledger_repeatability[function_name][function_call].inputs.push_back({
                .value = t.get_uuid(), .point = code_point
            });
        } else {
            ledger_repeatability[function_name][function_call].outputs.push_back({
                .value = t.get_uuid(), .point = code_point
            });
        }
    }

};

template<>
struct std::formatter<Ledger::discerned_point> : std::formatter<std::string> {
    auto format(const Ledger::discerned_point& s, auto& ctx) const {
        return std::format_to(ctx.out(), "discerned_point({}, {})", s.point, s.value);
    }
};

template<>
struct std::formatter<Ledger::discerned_io> : std::formatter<std::string> {
    auto format(const Ledger::discerned_io& s, auto& ctx) const {
        return std::format_to(ctx.out(), "discerned_io({}, {})", s.inputs, s.outputs);
    }
};

