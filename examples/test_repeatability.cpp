#include "proof_checker_defs.h"

struct _equals_primitive {
    constexpr static std::source_location loc = std::source_location::current();

    static propagate_errors_if_t<true> implementation(Case &c, std::tuple<r, r> const &args) {
        IMPLEMENTATION_START;
        return c.make_r();
    }

    template<bool care_about_this = true, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case &c, std::tuple<r, r> args) {
        INTERFACE_START;
        auto &[a, b] = args;
        DISCERN(a);
        DISCERN(b);
        IMPLEMENTATION;
        DISCERN(result);
        RETURN_RESULT;
    }
};

struct _equals {
    constexpr static std::source_location loc = std::source_location::current();

    static propagate_errors_if_t<true> implementation(Case &c, std::tuple<r, r> const &args) {
        constexpr bool care_about_this = true;
        constexpr bool is_being_checked = true;
        constexpr bool in_preconditions = false;
        return c.make_r();
    }

    template<bool care_about_this = true, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case &c, std::tuple<r, r> args) {
        INTERFACE_START;
        auto &[a, b] = args;

        DISCERN(a);
        DISCERN(b);

        IMPLEMENTATION;

        DISCERN(result);

        IF(result) {
            SUBSTITUTABLE(a, b);
        }

        RETURN_RESULT;
    }
};

struct foo {
    constexpr static std::source_location loc = std::source_location::current();

    static propagate_errors_if_t<true> implementation(Case &c, std::tuple<r, r, r, r> const &args) {
        IMPLEMENTATION_START;
        return c.make_r();
    }

    template<bool care_about_this, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case &c, std::tuple<r, r, r, r> args) {
        INTERFACE_START;

        auto &&[r1, r2, r3, r4] = args;

        CALL_INTERFACE(bool, b1, _equals, r1, r3);
        CLAIM(b1);

        CALL_INTERFACE(bool, b2, _equals, r2, r4);
        CLAIM(b2);

        IMPLEMENTATION;

        /*
         * r1 == r3 && r2 == r4
         * ->
         * f(r1, r2) == f(r3, r4)
         */

        CALL_INTERFACE(bool const, b3, _equals, r1, r2);
        CALL_INTERFACE(bool const, b4, _equals, r3, r4);

        IF(b3) {
            CLAIM(b4);
        }

        IF(b4) {
            CLAIM(b3);
        }

        RETURN_RESULT;
    }

    static auto check(Case &c) {
        return interface<true, true>(c, {c.make_r(), c.make_r(), c.make_r(), c.make_r()});
    }
};

int main() {
    verify_interface<foo>();
    return 0;
}
