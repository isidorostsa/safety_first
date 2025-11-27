#pragma once

#include <expected>
#include <variant>
#include "case.h"


#define JOIN_IMPL(x, y) x##y
#define JOIN(x, y) JOIN_IMPL(x, y)

#define NUM_ARGS_IMPL(_1, _2, _3, _4, _5, COUNT, ...) COUNT
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

#define APPLY1(M, x) M(x)
#define APPLY2(M, x, ...) M(x), APPLY1(M, __VA_ARGS__)
#define APPLY3(M, x, ...) M(x), APPLY2(M, __VA_ARGS__)
#define APPLY4(M, x, ...) M(x), APPLY3(M, __VA_ARGS__)
#define APPLY5(M, x, ...) M(x), APPLY4(M, __VA_ARGS__)

#define APPLY(M, ...) JOIN(APPLY, NUM_ARGS(__VA_ARGS__))(M, __VA_ARGS__)

#define MAKE_R_COPY(x) c.make_r_copy(x)

#define IF(x)                                                           \
    if(                                                                 \
        c.discern<in_preconditions>(x, function_name, function_call),   \
        c.next(x))

#define RESPONSIBLE \
    (in_preconditions && (care_about_this && !is_being_checked))    \
    ||                                                              \
    (!in_preconditions && (care_about_this && is_being_checked))

#define CALL_INTERFACE_IMPL(type, var_name, foo, temp_var_name, ...)\
    auto temp_var_name = foo::interface<RESPONSIBLE, false                 \
        >(c, {APPLY(MAKE_R_COPY, __VA_ARGS__)});                    \
    if (not temp_var_name) {                                        \
        return temp_var_name;                                       \
    }                                                               \
    r var_name = std::move(temp_var_name.value());                  \
    (void)0

#define CALL_INTERFACE(type, var_name, foo, ...)            \
    CALL_INTERFACE_IMPL(type, var_name, foo,                \
    JOIN(__call_interface_temp_, __COUNTER__), __VA_ARGS__)

#define IMPLEMENTATION                                      \
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
    if(not c.substitutable(x, y)) {                         \
        if constexpr (RESPONSIBLE) {                               \
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


#define RETURN_RESULT                   \
    static_assert(!in_preconditions);   \
    return std::move(result)

constexpr bool in_preconditions = true;
struct erroneous_branch_t {};
struct impossible_branch_t {
    impossible_branch_t() = default;
    impossible_branch_t(std::variant<impossible_branch_t, erroneous_branch_t> const&) {};
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
                std::println("Failed path: {}", c.values);
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
