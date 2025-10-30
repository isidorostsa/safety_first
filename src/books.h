//
// Created by ros on 10/13/25.
//

#pragma once

#include <map>
#include <vector>
#include <set>

#include "uuid.h"

inline void dfs(uuid_t const uuid, std::set<uuid_t>& reachables, std::map<uuid_t, std::set<uuid_t>>& ledger_value) {
    if (reachables.contains(uuid)) return;

    reachables.insert(uuid);

    for (uuid_t const other_uuid: ledger_value[uuid]) {
        dfs(other_uuid, reachables, ledger_value);
    }
}
