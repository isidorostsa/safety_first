//
// Created by ros on 9/20/25.
//
#include "proof_checker_defs.h"
#include <tuple>

struct _equals {
    struct _primitive {
        IMPLEMENTATION(r1, r2)
            return c.make_r();
        }

        INTERFACE(r1, r2)
            DISCERN(r1);
            DISCERN(r2);
            CALL_IMPLEMENTATION;
            DISCERN(result);
            RETURN_RESULT;
        }
    };

    IMPLEMENTATION(r1, r2)
        CALL_INTERFACE(bool, result, _primitive, r1, r2);
        RETURN_RESULT;
    }

    INTERFACE(r1, r2)
        DISCERN(r1);
        DISCERN(r2);

        CALL_INTERFACE(bool, known_prim_result, _primitive, r1, r1);
        CLAIM(known_prim_result);

        CALL_IMPLEMENTATION;
        DISCERN(result);

        CALL_INTERFACE(bool, prim_result, _primitive, r1, r2);
        CLAIM_EQUAL_BOOL(prim_result, result);

        IF (result) {
            SUBSTITUTABLE(r1, r2);
        }

        RETURN_RESULT;
    }

    static auto check(Case &c) {
        std::tuple<r, r> args = {c.make_r(), c.make_r()};
        std::tuple<r&, r&> args_ref = {std::get<0>(args), std::get<1>(args)};

        return interface<true, true>(c, args_ref);
    }
};

int main () {
    verify_interface<_equals>();
}