#pragma once

#include <expected>
#include <variant>
#include "case.h"


#define JOIN_IMPL(x, y) x##y
#define JOIN(x, y) JOIN_IMPL(x, y)

#define NUM_ARGS_IMPL(_0,_1,_2,_3,_4,_5,COUNT,...) COUNT
#define NUM_ARGS(...) NUM_ARGS_IMPL(_, ##__VA_ARGS__, 5,4,3,2,1,0)

#define APPLY0(M, x)
#define APPLY1(M, x) M(x)
#define APPLY2(M, x, ...) M(x), APPLY1(M, __VA_ARGS__)
#define APPLY3(M, x, ...) M(x), APPLY2(M, __VA_ARGS__)
#define APPLY4(M, x, ...) M(x), APPLY3(M, __VA_ARGS__)
#define APPLY5(M, x, ...) M(x), APPLY4(M, __VA_ARGS__)

#define APPLY(M, ...) JOIN(APPLY, NUM_ARGS(__VA_ARGS__))(M, __VA_ARGS__)

#define MAKE_R_COPY(x) c.make_r_copy(x)
#define MAKE_R_IGNORE(x) c.make_r()

#define PRIMITIVE_IMPLEMENTATION(...) \
    IMPLEMENTATION(__VA_ARGS__) \
        return c.make_r(); \
    }

#define CHECK(...) \
    static auto check(Case& c) { \
        auto args = std::make_tuple(APPLY(MAKE_R_IGNORE, __VA_ARGS__)); \
        return std::apply([&](auto&... refs) { \
            return interface<true, true>(c, std::tie(refs...)); \
        }, args); \
    }

#define IF(x)                                                           \
    if(                                                                 \
        c.discern<in_preconditions>(x, function_name, function_call),   \
        c.next(x))

#define RESPONSIBLE \
    (in_preconditions && (care_about_this && !is_being_checked))    \
    ||                                                              \
    (!in_preconditions && (care_about_this && is_being_checked))

// NOTE: the `type` argument is currently ignored in CALL_INTERFACE macros
#define CALL_INTERFACE_IMPL(type, var_name, foo, temp_var_name, ...)\
    auto temp_var_name = foo::interface<RESPONSIBLE, false                 \
        >(c, {__VA_ARGS__});                    \
    if (not temp_var_name) {                                        \
        return temp_var_name;                                       \
    }                                                               \
    r var_name = std::move(temp_var_name.value());                  \
    (void)0

#define CALL_INTERFACE(type, var_name, foo, ...)            \
    CALL_INTERFACE_IMPL(type, var_name, foo,                \
    JOIN(__call_interface_temp_, __COUNTER__), __VA_ARGS__)

#define CALL_INTERFACE_TUPLE_IMPL(type, var_name, foo, temp_var_name, args_tuple)\
    auto temp_var_name = foo::interface<RESPONSIBLE, false          \
        >(c, args_tuple);                                          \
    if (not temp_var_name) {                                        \
        return temp_var_name;                                       \
    }                                                               \
    r var_name = std::move(temp_var_name.value());                  \
    (void)0

#define CALL_INTERFACE_TUPLE(type, var_name, foo, args_tuple)  \
    CALL_INTERFACE_TUPLE_IMPL(type, var_name, foo,             \
    JOIN(__call_interface_temp_, __COUNTER__), args_tuple)

#define CALL_PRIMITIVE_INTERFACE                                         \
    CALL_IMPLEMENTATION;                                                    \
    CALL_INTERFACE_TUPLE(bool, primitive_result, _primitive, args);  \
    SUBSTITUTABLE(result, primitive_result)

#define CALL_PRIMITIVE_INTERFACE_ON(var_name, ...)              \
    CALL_INTERFACE(bool, var_name, _primitive, __VA_ARGS__)

#define CALL_IMPLEMENTATION                                      \
_Pragma("clang diagnostic push")                            \
_Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    constexpr bool in_preconditions = false;                \
_Pragma("clang diagnostic pop")                             \
    auto __result_tmp = [&] {                               \
        if constexpr (is_being_checked) {                   \
            return implementation(c, args);                 \
        } else {                                            \
            return c.make_r();                              \
        }                                                   \
    }();                                                    \
    if constexpr (is_being_checked) {                       \
        if (not __result_tmp) {                             \
            return __result_tmp;                            \
        }                                                   \
    }                                                       \
    r result = [&] {                                        \
        if constexpr (is_being_checked) {                   \
            return std::move(__result_tmp.value());         \
        } else {                                            \
            return std::move(__result_tmp);                 \
        }                                                   \
    }();                                                    \
    (void)0

#define DISCERN(x)                                          \
    c.discern<in_preconditions>(x,                          \
        function_name, function_call)

#define SUBSTITUTABLE(x, y)                                 \
    c.ledger.track(x.get_uuid());                           \
    c.ledger.track(y.get_uuid());                           \
    if(not c.substitutable(x, y)) {                         \
        if constexpr (RESPONSIBLE) {                        \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0

#define CLAIM(x)                                            \
    DISCERN(x);                                             \
    if(not c.claim<RESPONSIBLE>(x)) {                       \
        if constexpr (RESPONSIBLE) {                        \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0

#define CLAIM_FALSE(x)                                      \
    DISCERN(x);                                             \
    if(not c.claim_false<RESPONSIBLE>(x)) {                 \
        if constexpr (RESPONSIBLE) {                        \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0

// primitive needed to avoid recursion when comparing booleans
#define CLAIM_EQUAL_BOOL(r1, r2)                            \
    DISCERN(r1);                                            \
    DISCERN(r2);                                            \
    if(not c.claim_equal_bool<RESPONSIBLE>(r1, r2)) {       \
        if constexpr (RESPONSIBLE) {                        \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0


#define INTERFACE_START                                         \
    static_assert(in_preconditions);                            \
    static_assert(!is_being_checked || care_about_this);        \
    auto [function_name, function_call] = get_call_uuid(loc)

#define IMPLEMENTATION_START                                \
    constexpr bool care_about_this = true;                  \
    constexpr bool is_being_checked = true;                 \
_Pragma("clang diagnostic push")                            \
_Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    constexpr bool in_preconditions = false;                \
_Pragma("clang diagnostic pop")                             \
    auto [function_name, function_call] = get_call_uuid(loc)

#define TURN_TO_R(...) r
#define TURN_TO_R_ref(...) r&

#define KILL_BRANCH                                         \
    if constexpr (RESPONSIBLE) {                            \
        return std::unexpected(erroneous_branch_t{});       \
    } else {                                                \
        return std::unexpected(impossible_branch_t{});      \
    }

#define RETURN_RESULT                   \
    static_assert(!in_preconditions);   \
    return std::move(result)

#define RETURN_VOID                     \
    static_assert(!in_preconditions);   \
    return c.make_r_copy(Case::_void)


#define INTERFACE(...) \
    template<bool care_about_this, bool is_being_checked = false>           \
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<APPLY(TURN_TO_R_ref, __VA_ARGS__)> args)   { \
    INTERFACE_START;\
    auto&& [__VA_ARGS__] = args;

#define IMPLEMENTATION(...) \
    constexpr static std::source_location loc = std::source_location::current();\
    static propagate_errors_if_t<true> implementation(Case& c, std::tuple<APPLY(TURN_TO_R_ref, __VA_ARGS__)> args) {\
        IMPLEMENTATION_START;\
        auto&& [__VA_ARGS__] = args;

#define INTERFACE0() \
    template<bool care_about_this, bool is_being_checked = false>                        \
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<> args) { \
    INTERFACE_START;

#define IMPLEMENTATION0() \
    constexpr static std::source_location loc = std::source_location::current();         \
    static propagate_errors_if_t<true> implementation(Case& c, std::tuple<> args) {      \
        IMPLEMENTATION_START;

// Test macros — eliminate boilerplate for testing function interfaces.
// Write preconditions before CALL_IMPLEMENTATION (failures prune the path),
// and postconditions after (failures are errors).
//
// Usage:
//   TEST(test_name, r1, r2)
//       SUBSTITUTABLE(r1, r2);       // precondition
//       CALL_IMPLEMENTATION;
//       CALL_INTERFACE(bool, eq, _equals, r1, r2);
//       CLAIM(eq);                    // postcondition
//   END_TEST(r1, r2)

#define TEST(name, ...) \
    struct name { \
        IMPLEMENTATION(__VA_ARGS__) \
            RETURN_VOID; \
        } \
        INTERFACE(__VA_ARGS__)

#define END_TEST(...) \
            RETURN_VOID; \
        } \
        CHECK(__VA_ARGS__) \
    }

constexpr bool in_preconditions = true;
struct erroneous_branch_t {
    std::source_location loc;
    erroneous_branch_t(std::source_location loc = std::source_location::current()) : loc(loc) {}
};
struct impossible_branch_t {
    std::source_location loc;
    impossible_branch_t(std::source_location loc = std::source_location::current()) : loc(loc) {}
    impossible_branch_t(std::variant<impossible_branch_t, erroneous_branch_t> const&) : loc() {};
};


template<bool b>
using propagate_errors_if_t = std::conditional_t<
    b,
    std::expected<r, std::variant<impossible_branch_t, erroneous_branch_t>>,
    std::expected<r, impossible_branch_t>
>;

template<typename Foo>
bool verify_interface() {
    Case c;
    bool found_successful_case = false;

    do {
        std::println("------");
        std::println("Going to try path: {}", c.values);

        if (auto res = Foo::check(c)) {
            std::println("Passed path: {}", c.values);
            found_successful_case = true;
        } else {
            auto const& err = res.error();
            if (std::holds_alternative<impossible_branch_t>(err)) {
                std::println("Pruned path: {}", c.values);
            } else {
                auto const& loc = std::get<erroneous_branch_t>(err).loc;
                std::println("Failed path: {} (at {}:{})", c.values, loc.file_name(), loc.line());
                std::println("  Branches:");
                for (size_t i = 0; i < c.branch_locations.size(); ++i) {
                    auto const& bl = c.branch_locations[i];
                    std::println("    [{}] -\t{}\t- {}:{}", i, c.values[i], bl.file_name(), bl.line());
                }
                std::println("    [x] -\tFAILED\t- {}:{}", loc.file_name(), loc.line());
                return false;
            }
        }
    } while (c.prepare_next_iteration());

    if (!found_successful_case) {
        std::println("No successful case found");
    }

    std::println("------");
    std::println("Finished");
    return true;
}
