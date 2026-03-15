#pragma once

#include "proof_checker_defs.h"
#include "primitives/bool.hpp"
#include <tuple>

struct _not {
    struct _primitive {
        IMPLEMENTATION(r1)
            return c.make_r();
        }

        INTERFACE(r1)
            DISCERN(r1);
            CALL_IMPLEMENTATION;
            DISCERN(result);
            RETURN_RESULT;
        }
    };

    IMPLEMENTATION(r1)
        CALL_PRIMITIVE_INTERFACE_ON(result, r1);
        RETURN_RESULT;
    }

    INTERFACE(r1)
        DISCERN(r1);

        CALL_INTERFACE(bool, tt, _bool_true);
        CALL_INTERFACE(bool, ff, _bool_false);

        CALL_PRIMITIVE_INTERFACE_ON(prim_not_t, tt);
        CALL_PRIMITIVE_INTERFACE_ON(prim_not_f, ff);

        GIVEN_FALSE(prim_not_t);
        GIVEN(prim_not_f);

        CALL_PRIMITIVE_INTERFACE;

        IF(r1) {
            CLAIM_FALSE(result);
        } else {
            CLAIM(result);
        }

        // result = _prim(r1) <-- V123
        // going through prim(r1):
        //      discerned input: r1. Never seen this. discerned output: V123, input was never seen b4, so skip.

        // r1 = true
        // what can we say about V123? How could we

        RETURN_RESULT;
    }

    CHECK(r1)
};