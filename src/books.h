//
// Created by ros on 10/13/25.
//

#ifndef TEST_FILES_BOOKS_H
#define TEST_FILES_BOOKS_H

#include <map>
#include <vector>
#include <set>

#include "uuid.h"

struct Case;

inline void dfs(uuid_t const uuid, std::set<uuid_t>& reachables, std::map<uuid_t, std::vector<uuid_t>>& ledger_value) {
    if (reachables.contains(uuid)) return;

    reachables.insert(uuid);

    for (uuid_t const other_uuid: ledger_value[uuid]) {
        dfs(other_uuid, reachables, ledger_value);
    }
}

inline std::set<uuid_t> reachable_nodes_from(uuid_t uuid, Case& c);

inline std::set<uuid_t> reachable_nodes_from(uuid_t const uuid, std::map<uuid_t, std::vector<uuid_t>>& ledger_value) {
    std::set<uuid_t> result;
    dfs(uuid, result, ledger_value);
    return result;
}

#endif //TEST_FILES_BOOKS_H