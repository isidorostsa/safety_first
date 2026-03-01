#include "proof_checker_defs.h"
#include "equal.hpp"
#include "not.hpp"


struct _test_equal {
    IMPLEMENTATION(r1, r2)
        RETURN_VOID;
    }

    INTERFACE(r1, r2)
        SUBSTITUTABLE(r1, r2);
        CALL_IMPLEMENTATION;
        CALL_INTERFACE(bool, same, _equals, r1, r2);
        CLAIM(same);
        RETURN_RESULT;
    }

    static auto check(Case &c) {
        std::tuple<r, r> args = {c.make_r(), c.make_r()};
        std::tuple<r&, r&> args_ref = {std::get<0>(args), std::get<1>(args)};

        return interface<true, true>(c, args_ref);
    }
};

struct _test_not {
    IMPLEMENTATION(r1)
        RETURN_VOID;
    }

    INTERFACE( r1)
        CLAIM(r1);
        CALL_IMPLEMENTATION;
        CALL_INTERFACE(bool, not_r1, _not, r1);
        CLAIM_FALSE(not_r1);
        RETURN_RESULT;
    }

    static auto check(Case &c) {
        std::tuple<r> args = {c.make_r()};
        std::tuple<r&> args_ref = {std::get<0>(args)};

        return interface<true, true>(c, args_ref);
    }
};
int main () {
    verify_interface<_test_equal>();
    verify_interface<_test_not>();
}