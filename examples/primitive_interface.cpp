//
// Created by ros on 9/20/25.
//
#include "proof_checker_defs.h"

/*
class T {
    void T_construct_impl(int const& _v) implementation;
    void T_copy_construct_impl(const T& t) implementation;

public:
    int v;

    explicit T(int const& v) interface { // NOLINT(cppcoreguidelines-pro-type-member-init)
        discern(v);
        T_construct_impl(FWD(v));
        discern(*this);
        substitutable(this->v, v);
    };

    T(const T& t) interface { // NOLINT(cppcoreguidelines-pro-type-member-init)
        discern(t);
        T_copy_construct_impl(FWD(t));
        discern(*this);
        substitutable(t, *this);
    };
};

class F {
    bool operator_equals_impl(F const& other) implementation const;

public:
    int v;

    bool operator==(F const& other) interface const {
        discern(this->v, other.v);
        auto&& result = operator_equals_impl(FWD(other));
        discern(result);
        if (result) {
            substitutable(*this, result);
        }
        return result;
    }

};

const T t0{0};
const F f0{0};

struct _and {
    static propagate_errors_if_t<true> implementation(Case& c, std::tuple<r>& args) {
        constexpr bool care_about_this = true;
        constexpr bool is_being_checked = true;
        constexpr bool in_preconditions = false;


    }

    template<bool care_about_this, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<r> args) {
        static_assert(!is_being_checked || care_about_this);

        auto&& [a] = args;

        DISCERN(a);

        IMPLEMENTATION;

        DISCERN(result);

        RETURN_RESULT;
    }

    static auto check(Case& c) {
        return interface<true>(c, {c.make_r(), c.make_r()});
    }

};
struct foo_primitive {
    static propagate_errors_if_t<true> implementation(Case& c, std::tuple<r> const& args) {
        auto&& [t] = args;
        return c.make_r_copy(t);
    }

    template<bool care_about_this, bool is_being_checked = false>
    static propagate_errors_if_t<care_about_this> interface(Case& c, std::tuple<r> args) {
        INTERFACE_START;
        auto&& [t] = args;
        DISCERN(t);
        IMPLEMENTATION;
        DISCERN(result);
        RETURN_RESULT;
    };
};

// F foo_impl(T const& t) implementation { return __foo_impl(FWD(t)); }
F foo_impl(T const& t) implementation;
F foo(T const& t) interface {
    discern(t);
    auto&& result = foo_impl(FWD(t));
    discern(result);
    claim(foo_primitive(FWD(t)) == f0);
    claim(result == foo_primitive(t));
    return result;
}

int main () {
    claim(foo(t0) == f0);
}
*/

int main () {}