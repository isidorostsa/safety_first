#pragma once

#include <algorithm>
#include <set>
#include <map>
#include <format>
#include <vector>
#include <cassert>
#include <source_location>
#include <print>

#include <nlohmann/json.hpp>

#include "uuid.h"

struct TraceNode {
    value_uuid_t a, b;
    bool succeeded = false;
    bool direct = false;

    struct Attempt {
        function_point_t func;
        call_idx_t c1, c2;
        std::vector<TraceNode> input_checks;
        std::vector<std::pair<value_uuid_t, value_uuid_t>> new_unions;
        bool succeeded = false;
    };
    std::vector<Attempt> attempts;
};

class Ledger {

    // Union-find for value equivalence
    std::vector<value_uuid_t> parent;

    // Source location where each value was defined
    std::vector<std::source_location> definition_place;

    // Variable name at definition site (set via macro stringification)
    std::vector<std::string> variable_names;

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
        definition_place.resize(idx + 1);
        variable_names.resize(idx + 1);
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

    bool inputs_equivalent(CallRecord const& c1, CallRecord const& c2,
                           visited_set& visited,
                           TraceNode::Attempt* attempt = nullptr) {
        if (c1.inputs.size() != c2.inputs.size()) return false;

        for (size_t i = 0; i < c1.inputs.size(); ++i) {
            TraceNode* sub = nullptr;
            if (attempt) {
                attempt->input_checks.emplace_back();
                sub = &attempt->input_checks.back();
            }
            if (!propagate_provenance(c1.inputs[i], c2.inputs[i], visited, sub))
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
                            value_uuid_t target, visited_set& visited,
                            TraceNode* trace = nullptr) {
        if (target.raw >= produced_by.size()) return false;

        for (auto c2_idx : produced_by[target.raw]) {
            auto& c2 = calls[c2_idx.raw];
            auto it = calls_by_function.find(c2.func);
            if (it == calls_by_function.end()) continue;

            for (auto c1_idx : it->second) {
                if (c1_idx == c2_idx) continue;
                auto& c1 = calls[c1_idx.raw];

                if (c1.outputs.size() != c2.outputs.size()) continue;

                TraceNode::Attempt* attempt = nullptr;
                if (trace) {
                    trace->attempts.emplace_back();
                    attempt = &trace->attempts.back();
                    attempt->func = c2.func;
                    attempt->c1 = c1_idx;
                    attempt->c2 = c2_idx;
                }

                if (inputs_equivalent(c1, c2, visited, attempt)) {
                    unify_outputs(c1, c2);
                    if (attempt) {
                        attempt->succeeded = true;
                        for (size_t i = 0; i < std::min(c1.outputs.size(), c2.outputs.size()); ++i) {
                            attempt->new_unions.emplace_back(c1.outputs[i], c2.outputs[i]);
                        }
                    }
                    if (find(a) == find(b)) return true;
                }
            }
        }
        return false;
    }

    // Collect all values whose root matches `root`
    std::vector<value_uuid_t> equivalence_class_of(value_uuid_t root) const {
        std::vector<value_uuid_t> members;
        auto r = find_const(root);
        for (uint32_t i = 0; i < parent.size(); ++i) {
            if (find_const(value_uuid_t{i}) == r)
                members.push_back(value_uuid_t{i});
        }
        return members;
    }

    // Lazy provenance-based propagation
    // When direct find(a)==find(b) fails, check if a and b were produced
    // by calls to the same function with equivalent inputs
    bool propagate_provenance(value_uuid_t a, value_uuid_t b,
                              visited_set& visited,
                              TraceNode* trace = nullptr) {
        if (trace) { trace->a = a; trace->b = b; }

        auto ra = find(a), rb = find(b);
        if (ra == rb) {
            if (trace) { trace->succeeded = true; trace->direct = true; }
            return true;
        }

        auto key = std::pair{std::min(ra.raw, rb.raw), std::max(ra.raw, rb.raw)};
        if (visited.contains(key)) {
            if (trace) { trace->succeeded = false; }
            return false;
        }
        visited.insert(key);

        // Try all members of each equivalence class for provenance
        for (auto member : equivalence_class_of(b)) {
            if (try_propagate_from(a, b, member, visited, trace)) {
                if (trace) trace->succeeded = true;
                return true;
            }
        }
        for (auto member : equivalence_class_of(a)) {
            if (try_propagate_from(a, b, member, visited, trace)) {
                if (trace) trace->succeeded = true;
                return true;
            }
        }

        bool result = find(a) == find(b);
        if (trace) trace->succeeded = result;
        return result;
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
        definition_place.clear();
        variable_names.clear();
        produced_by.clear();
        calls.clear();
        calls_by_function.clear();
    }

    void set_definition_place(value_uuid_t t, std::source_location loc,
                              const char* var_name = "") {
        if (t.raw < definition_place.size()) {
            definition_place[t.raw] = loc;
            variable_names[t.raw] = var_name;
        }
    }

    value_uuid_t find_const(value_uuid_t t) const {
        if (t.raw >= parent.size()) return t;
        while (parent[t.raw] != t) {
            t = parent[t.raw];
        }
        return t;
    }

    static std::string_view basename(const char* path) {
        std::string_view sv(path);
        auto pos = sv.find_last_of('/');
        if (pos == std::string_view::npos) pos = sv.find_last_of('\\');
        return (pos == std::string_view::npos) ? sv : sv.substr(pos + 1);
    }

    void dump_graph() const {
        // Group nodes by their equivalence class root
        std::map<uint32_t, std::vector<uint32_t>> classes;
        for (size_t i = 0; i < parent.size(); ++i) {
            auto [raw] = find_const(value_uuid_t{static_cast<uint32_t>(i)});
            classes[raw].push_back(static_cast<uint32_t>(i));
        }

        auto func_for = [&](uint32_t i) -> std::string {
            if (i < definition_place.size()) {
                auto const& loc = definition_place[i];
                if (loc.function_name() != nullptr && loc.function_name()[0] != '\0')
                    return loc.function_name();
            }
            return "";
        };

        nlohmann::json j;
        auto& eq = j["equivalence_classes"] = nlohmann::json::array();
        for (const auto &members: classes | std::views::values) {
            auto cls = nlohmann::json::array();
            for (const auto id : members) {
                auto node = value_json(value_uuid_t{id});
                node["func"] = func_for(id);
                cls.push_back(std::move(node));
            }
            eq.push_back(std::move(cls));
        }
        std::print("{}\n", j.dump(2));
    }

    void dump_calls_graph() const {
        // Group calls into equivalence classes:
        // same function + pairwise-equivalent inputs
        std::vector<std::vector<call_idx_t>> classes;

        for (const auto &indices: calls_by_function | std::views::values) {
            std::map<std::vector<uint32_t>, std::vector<call_idx_t>> by_inputs;
            for (auto idx : indices) {
                auto const& call = calls[idx.raw];
                std::vector<uint32_t> key;
                key.reserve(call.inputs.size());
                for (auto inp : call.inputs) {
                    key.push_back(find_const(inp).raw);
                }
                by_inputs[key].push_back(idx);
            }
            for (auto &group: by_inputs | std::views::values) {
                classes.push_back(std::move(group));
            }
        }

        nlohmann::json j;
        auto& cc = j["call_equivalence_classes"] = nlohmann::json::array();
        for (auto const& group : classes) {
            auto cls = nlohmann::json::array();
            for (auto idx : group) {
                const auto&[
                    func,
                    inputs,
                    outputs
                ] = calls[idx.raw];

                nlohmann::json entry;
                entry["idx"] = idx.raw;
                entry["func"] = func_label(func);
                auto json_inputs = nlohmann::json::array();
                for (auto inp : inputs) json_inputs.push_back(value_json(inp));
                entry["inputs"] = std::move(json_inputs);
                auto json_outputs = nlohmann::json::array();
                for (auto out : outputs) json_outputs.push_back(value_json(out));
                entry["outputs"] = std::move(json_outputs);
                cls.push_back(std::move(entry));
            }
            cc.push_back(std::move(cls));
        }
        std::print("{}\n", j.dump(2));
    }

    TraceNode trace_claim(value_uuid_t a, value_uuid_t b) {
        TraceNode root;
        root.a = a;
        root.b = b;

        if (find(a) == find(b)) {
            root.succeeded = true;
            root.direct = true;
            return root;
        }

        visited_set visited;
        propagate_provenance(a, b, visited, &root);
        return root;
    }

private:
    nlohmann::json value_json(value_uuid_t v) const {
        auto id = v.raw;
        std::string label;
        if (id == 0) label = "_true";
        else if (id == 1) label = "_false";
        else if (id == 2) label = "_void";
        else if (id < definition_place.size()) {
            auto const& loc = definition_place[id];
            if (loc.file_name() != nullptr && loc.file_name()[0] != '\0')
                label = std::format("{}:{}", basename(loc.file_name()), loc.line());
            else
                label = std::format("r{}", id);
        } else {
            label = std::format("r{}", id);
        }

        std::string var = (id < variable_names.size() && !variable_names[id].empty())
            ? variable_names[id] : "";

        return {{"id", id}, {"label", label}, {"var", var}};
    }

    static std::string func_label(function_point_t const& fp) {
        if (fp.file_name.empty() && fp.function_name.empty())
            return "";
        // "basename:line (short_func)"
        std::string result;
        if (!fp.file_name.empty()) {
            result = std::format("{}:{}", basename(fp.file_name.data()), fp.line);
        }
        if (!fp.function_name.empty()) {
            std::string_view sv = fp.function_name;
            if (const auto paren = sv.find('('); paren != std::string_view::npos) sv = sv.substr(0, paren);
            if (const auto space = sv.rfind(' '); space != std::string_view::npos) sv = sv.substr(space + 1);
            if (!result.empty()) result += " ";
            result += sv;
        }
        return result;
    }

    nlohmann::json trace_to_json(TraceNode const& node) const {
        nlohmann::json j;
        j["a"] = value_json(node.a);
        j["b"] = value_json(node.b);
        j["succeeded"] = node.succeeded;
        j["direct"] = node.direct;

        auto attempts = nlohmann::json::array();
        for (auto const& att : node.attempts) {
            nlohmann::json a;
            a["func"] = func_label(att.func);
            a["succeeded"] = att.succeeded;

            auto const& c1 = calls[att.c1.raw];
            auto const& c2 = calls[att.c2.raw];

            auto c1_inputs = nlohmann::json::array();
            for (auto inp : c1.inputs) c1_inputs.push_back(value_json(inp));
            a["call1_inputs"] = std::move(c1_inputs);

            auto c2_inputs = nlohmann::json::array();
            for (auto inp : c2.inputs) c2_inputs.push_back(value_json(inp));
            a["call2_inputs"] = std::move(c2_inputs);

            auto input_checks = nlohmann::json::array();
            for (auto const& sub : att.input_checks)
                input_checks.push_back(trace_to_json(sub));
            a["input_checks"] = std::move(input_checks);

            auto new_unions = nlohmann::json::array();
            for (auto [u1, u2] : att.new_unions)
                new_unions.push_back(nlohmann::json::array({value_json(u1), value_json(u2)}));
            a["new_unions"] = std::move(new_unions);

            attempts.push_back(std::move(a));
        }
        j["attempts"] = std::move(attempts);
        return j;
    }

public:
    void dump_trace(TraceNode const& trace) const {
        std::print("{}\n", trace_to_json(trace).dump(2));
    }

    std::string describe_value(value_uuid_t v) const {
        auto id = v.raw;
        if (id == 0) return "_true (id=0)";
        if (id == 1) return "_false (id=1)";
        if (id == 2) return "_void (id=2)";

        std::string result = std::format("r{}", id);
        if (id < variable_names.size() && !variable_names[id].empty()) {
            result += std::format(" \"{}\"", variable_names[id]);
        }
        if (id < definition_place.size()) {
            auto const& loc = definition_place[id];
            if (loc.file_name() != nullptr && loc.file_name()[0] != '\0')
                result += std::format(" at {}:{}", basename(loc.file_name()), loc.line());
        }
        return result;
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
