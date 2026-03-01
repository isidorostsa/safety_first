#pragma once

#include "proof_checker_defs.h"
#include <tuple>

struct _equals {
    struct _primitive {
        PRIMITIVE_IMPLEMENTATION(r1, r2)

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

        CALL_IMPLEMENTATION;

        DISCERN(result);

        CALL_INTERFACE(bool, known_prim_result, _primitive, r1, r1);
        CLAIM(known_prim_result);

        CALL_INTERFACE(bool, prim_result, _primitive, r1, r2);
        CLAIM_EQUAL_BOOL(prim_result, result);

        IF (result) {
            SUBSTITUTABLE(r1, r2);
        }
        RETURN_RESULT;
    }

    CHECK(r1, r2)
};