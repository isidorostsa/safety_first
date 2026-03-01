#pragma once

#include "proof_checker_defs.h"
#include <tuple>

struct _not {
    struct _primitive {
        IMPLEMENTATION(r1)
            IF(r1) {
                return c.make_r_copy(Case::_false);
            } else {
                return c.make_r_copy(Case::_true);
            }
        }

        INTERFACE(r1)
            DISCERN(r1);
            CALL_IMPLEMENTATION;
            DISCERN(result);
            RETURN_RESULT;
        }
    };

    IMPLEMENTATION(r1)
        CALL_INTERFACE(bool, result, _primitive, r1);
        RETURN_RESULT;
    }

    INTERFACE(r1)
        DISCERN(r1);

        // temp workaround, need to figure out how to handle r constness
        auto f = c.make_r_copy(Case::_false);
        auto t = c.make_r_copy(Case::_true);

        CALL_INTERFACE(bool, known_prim_false, _primitive, f);
        CLAIM(known_prim_false);

        CALL_INTERFACE(bool, known_prim_true, _primitive, t);
        CLAIM_FALSE(known_prim_true);

        CALL_IMPLEMENTATION;
        DISCERN(result);

        CALL_INTERFACE(bool, prim_result, _primitive, r1);

        CLAIM_EQUAL_BOOL(prim_result, result);

        RETURN_RESULT;
    }

    CHECK(r1)
};