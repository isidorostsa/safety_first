//
// Created by ros on 10/7/25.
//

#include <expected>

#include "proof_checker_defs.h"

struct _some {
    template<bool care_about_this = true, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<r, r> args) {
        return c.make_r();
    }
};

struct _and {
    constexpr static std::source_location loc = std::source_location::current();

    static propagate_errors_if_t<true> implementation(Case& c, std::tuple<r, r> const& args) {
        IMPLEMENTATION_START;

        auto&& [a, b] = args;

        IF(a) {
            IF(b) {
                return c.make_r_copy(Case::_true);
            }
        }
        return c.make_r_copy(Case::_false);
    }

    template<bool care_about_this, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<r, r> args) {
        INTERFACE_START;
        auto&& [a, b] = args;

        DISCERN(a);
        DISCERN(b);

        CALL_INTERFACE(int, my, _some, a, b);

        IMPLEMENTATION;

        DISCERN(result);

        IF(result) {
            CLAIM(a);
            CLAIM(b);
        }

        RETURN_RESULT;
    }

    static auto check(Case& c) {
        return interface<true, true>(c, {c.make_r(), c.make_r()});
    }

};

int main() {
    verify_interface<_and>();
    return 0;
}