//
// Created by ros on 10/7/25.
//

#include "proof_checker_defs.h"
#include "equal.hpp"


struct _and {
    IMPLEMENTATION(r1, r2)
        IF(r1) {
            IF(r2) {
                return c.make_r_copy(Case::_true);
            }
        }
        return c.make_r_copy(Case::_false);
    }

    INTERFACE(r1, r2)
        DISCERN(r1);
        DISCERN(r2);

        CALL_IMPLEMENTATION;

        DISCERN(result);

        IF(result) {
            CLAIM(r1);
            CLAIM(r2);
        }

        RETURN_RESULT;
    }

    static auto check(Case &c) {
        std::tuple<r, r> args = {c.make_r(), c.make_r()};
        std::tuple<r&, r&> args_ref = {std::get<0>(args), std::get<1>(args)};

        return interface<true, true>(c, args_ref);
    }
};

int main() {
    verify_interface<_and>();
    return 0;
}