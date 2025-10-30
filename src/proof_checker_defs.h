//
// Created by ros on 9/26/25.
//

#pragma once

#include <exception>
#include <expected>

#include "uuid.h"

#include <variant>
#include "case.h"


#define _JOIN(x, y) x##y
#define JOIN(x, y) _JOIN(x, y)

#define NUM_ARGS_IMPL(_1, _2, _3, _4, _5, COUNT, ...) COUNT
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

#define APPLY1(M, x) M(x)
#define APPLY2(M, x, ...) M(x), APPLY1(M, __VA_ARGS__)
#define APPLY3(M, x, ...) M(x), APPLY2(M, __VA_ARGS__)
#define APPLY4(M, x, ...) M(x), APPLY3(M, __VA_ARGS__)
#define APPLY5(M, x, ...) M(x), APPLY4(M, __VA_ARGS__)

#define APPLY(M, ...) JOIN(APPLY, NUM_ARGS(__VA_ARGS__))(M, __VA_ARGS__)

#define MAKE_R_COPY(x) c.make_r_copy(x)

#define IF(x) if(c.next(x))

#define CARE \
    (in_preconditions && (care_about_this && !is_being_checked))    \
    ||                                                              \
    (!in_preconditions && (care_about_this && is_being_checked))

#define _CALL_INTERFACE(type, var_name, foo, temp_var_name, ...)    \
    auto temp_var_name = foo::interface<CARE, false                      \
        >(c, {APPLY(MAKE_R_COPY, __VA_ARGS__)});                    \
    if (not temp_var_name) {                                        \
        return temp_var_name;                                       \
    }                                                               \
    r var_name = std::move(temp_var_name.value());                  \
    (void)0

#define CALL_INTERFACE(type, var_name, foo, ...)            \
    _CALL_INTERFACE(type, var_name, foo,                    \
    JOIN(__call_interface_temp_, __COUNTER__), __VA_ARGS__)

#define IMPLEMENTATION                                      \
    constexpr bool in_preconditions = false;                \
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
    c.template discern<in_preconditions>(x,                 \
        func_name, func_call)

#define SUBSTITUTABLE(x, y)                                 \
    if(not c.substitutable(x, y)) {                         \
        if constexpr (CARE) {                               \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0

#define CLAIM(x)                                            \
    if(not c.claim(x)) {                                    \
        if constexpr (CARE) {                               \
            return std::unexpected(erroneous_branch_t{});   \
        } else {                                            \
            return std::unexpected(impossible_branch_t{});  \
        }                                                   \
    }                                                       \
    (void)0

#define INTERFACE_START                                         \
    static_assert(in_preconditions);                            \
    static_assert(!is_being_checked || care_about_this);        \
    auto [func_name, func_call] = get_call_uuid()

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

        auto res = Foo::check(c);
        if (res) {
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
        std::println("Function table: {}", c.repeatability_table);
    } while (c.prepare_next_iteration());

    if (!found_successful_case) {
        std::println("No successful case found");
    }

    std::println("------");
    std::println("Finished");
    return true;
}