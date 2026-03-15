#pragma once

#include <algorithm>
#include <set>
#include <map>
#include <format>
#include <vector>
#include <cassert>

#include "uuid.h"

class Ledger {

    // Union-find for value equivalence
    std::vector<value_uuid_t> parent;

    // Provenance: which calls produced a given value as output
    std::vector<std::vector<call_idx_t>> produced_by;

    // Flat call table
    struct CallRecord {
        function_point_t func;
        std::vector<value_uuid_t> inputs;
        std::vector<value_uuid_t> outputs;
    };
    std::vector<CallRecord> calls;
    std::map<function_point_t, std::vector<call_idx_t>> calls_by_function;

public:
    void track(value_uuid_t const t) {
        auto idx = t.raw;
        if (idx < parent.size()) return; // already tracked

        auto old_size = parent.size();
        parent.resize(idx + 1);
        produced_by.resize(idx + 1);
        // Set self-parent for all newly added entries
        for (size_t i = old_size; i <= idx; ++i) {
            parent[i] = value_uuid_t{static_cast<uint32_t>(i)};
        }
    }

    value_uuid_t find(value_uuid_t t) {
        // Ensure tracked
        if (t.raw >= parent.size()) return t;

        while (parent[t.raw] != t) {
            // Path compression
            parent[t.raw] = parent[parent[t.raw].raw];
            t = parent[t.raw];
        }
        return t;
    }

    void union_values(value_uuid_t a, value_uuid_t b) {
        a = find(a);
        b = find(b);
        if (a != b) {
            parent[a.raw] = b;
        }
    }

private:
    using visited_set = std::set<std::pair<uint32_t, uint32_t>>;

    bool inputs_equivalent(CallRecord const& c1, CallRecord const& c2, visited_set& visited) {
        if (c1.inputs.size() != c2.inputs.size()) return false;

        for (size_t i = 0; i < c1.inputs.size(); ++i) {
            if (!propagate_provenance(c1.inputs[i], c2.inputs[i], visited))
                return false;
        }
        return true;
    }

    void unify_outputs(CallRecord const& c1, CallRecord const& c2) {
        for (size_t i = 0; i < std::min(c1.outputs.size(), c2.outputs.size()); ++i) {
            union_values(c1.outputs[i], c2.outputs[i]);
        }
    }

    // Try to prove a ~ b by finding a call that produced one of them
    // whose inputs match another call to the same function
    bool try_propagate_from(value_uuid_t a, value_uuid_t b,
                            value_uuid_t target, visited_set& visited) {
        if (target.raw >= produced_by.size()) return false;

        for (auto c2_idx : produced_by[target.raw]) {
            auto& c2 = calls[c2_idx.raw];
            auto it = calls_by_function.find(c2.func);
            if (it == calls_by_function.end()) continue;

            for (auto c1_idx : it->second) {
                if (c1_idx == c2_idx) continue;
                auto& c1 = calls[c1_idx.raw];

                if (c1.outputs.size() != c2.outputs.size()) continue;

                if (inputs_equivalent(c1, c2, visited)) {
                    unify_outputs(c1, c2);
                    if (find(a) == find(b)) return true;
                }
            }
        }
        return false;
    }

    // Lazy provenance-based propagation
    // When direct find(a)==find(b) fails, check if a and b were produced
    // by calls to the same function with equivalent inputs
    bool propagate_provenance(value_uuid_t a, value_uuid_t b, visited_set& visited) {
        auto ra = find(a), rb = find(b);
        if (ra == rb) return true;

        auto key = std::pair{std::min(ra.raw, rb.raw), std::max(ra.raw, rb.raw)};
        if (visited.contains(key)) return false;
        visited.insert(key);

        if (try_propagate_from(a, b, b, visited)) return true;
        if (try_propagate_from(a, b, a, visited)) return true;

        return find(a) == find(b);
    }

public:
    bool is_substitutable_with(value_uuid_t a, value_uuid_t b) {
        if (find(a) == find(b)) return true;

        // Try lazy provenance propagation
        std::set<std::pair<uint32_t,uint32_t>> visited;
        return propagate_provenance(a, b, visited);
    }

    void set_substitutable(value_uuid_t a, value_uuid_t b) {
        if (a.raw >= parent.size()) track(a);
        if (b.raw >= parent.size()) track(b);
        union_values(a, b);
    }

    void clear_ledgers() {
        parent.clear();
        produced_by.clear();
        calls.clear();
        calls_by_function.clear();
    }

    // Call recording for repeatability
    call_idx_t begin_call(function_point_t const& func) {
        call_idx_t idx{static_cast<uint32_t>(calls.size())};
        calls.push_back({func, {}, {}});
        calls_by_function[func].push_back(idx);
        return idx;
    }

    void record_input(call_idx_t call, value_uuid_t value) {
        calls[call.raw].inputs.push_back(value);
    }

    void record_output(call_idx_t call, value_uuid_t value) {
        calls[call.raw].outputs.push_back(value);
        // Register provenance
        if (value.raw >= produced_by.size()) {
            produced_by.resize(value.raw + 1);
        }
        produced_by[value.raw].push_back(call);
    }
};

template<uuid_kind K>
struct std::formatter<typed_uuid<K>> : std::formatter<uint32_t> {
    auto format(const typed_uuid<K> &u, auto &ctx) const {
        return std::formatter<uint32_t>::format(u.raw, ctx);
    }
};
