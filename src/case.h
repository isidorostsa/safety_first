//
// Created by ros on 10/13/25.
//

#ifndef TEST_FILES_CASE_H
#define TEST_FILES_CASE_H

#include <vector>
#include <print>
#include <format>
#include <map>

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
    std::map<uuid_t, std::vector<uuid_t>> ledger_legal;
    std::map<uuid_t, std::vector<uuid_t>> ledger_value;

    // True and false nodes for this case
    static constexpr r _true = r(0);
    static constexpr r _false = r(1);

    explicit Case() : values(), current(values.begin()){
        add_to_book(_true);
        add_to_book(_false);
    }

    void add_to_book(r const& t) {
        ledger_value.try_emplace(t.uuid);
    }

    void substitutable(r const& t1, r const& t2) {
        if (not ledger_value.contains(t1.uuid) or not ledger_value.contains(t2.uuid)) {
            throw;
        }

        ledger_value[t1.uuid].push_back(t2.uuid);
        ledger_value[t2.uuid].push_back(t1.uuid);

        // ReSharper disable once CppTooWideScopeInitStatement
        std::set<uuid_t> const reachables = reachable_nodes_from(t1.uuid);

        if ( reachables.contains(_true.uuid) and reachables.contains(_false.uuid) ) {
            throw;
        }
    }

    std::optional<bool> decided_direction(r const& g) {
        auto const reachables = reachable_nodes_from(g.uuid);
        if (reachables.contains(_true.uuid) and reachables.contains(_false.uuid)) {
            throw;
        } else if (reachables.contains(_true.uuid)) {
            return true;
        } else if (reachables.contains(_false.uuid)) {
            return false;
        }
        return std::nullopt;
    }

    std::set<uuid_t> reachable_nodes_from(uuid_t const uuid) {
        return ::reachable_nodes_from(uuid, ledger_value);
    }

    bool claim(r const& t) {
        return reachable_nodes_from(t.uuid).contains(_true.uuid);
    }

    void discern(r const& d) {
        add_to_book(d);
    }

    r make_r(const uuid_t _uuid = var_uuid()) {
        r result(_uuid);
        add_to_book(result);
        return result;
    }
    r make_r_copy(r const& other, const uuid_t _uuid = var_uuid()) {
        r result(_uuid);
        add_to_book(result);
        substitutable(result, other);
        return result;
    }

    bool next(r const& guard) {

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
    void expand(r const& guard) {
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
        while (--i >= 0 && values[i] != False) {}

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
    }

    bool prepare_next_iteration() {
        clear_ledgers();
        add_to_book(_true);
        add_to_book(_false);
        return increment();
    }
};

template<>
struct std::formatter<Case::direction> : std::formatter<std::string_view> {
    auto format(Case::direction e, auto& ctx) const {
        constexpr std::string_view names[] = { "True", "False", "ForcedFalse" };
        return std::formatter<std::string_view>::format(names[static_cast<int>(e)], ctx);
    }
};

#endif //TEST_FILES_CASE_H
