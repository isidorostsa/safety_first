#pragma once

#include <iostream>

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
        CALL_PRIMITIVE_INTERFACE_ON(result, r1, r2);
        RETURN_RESULT;
    }

    INTERFACE(r1, r2)
        DISCERN(r1);
        DISCERN(r2);

        CALL_PRIMITIVE_INTERFACE_ON(self_equality, r1, r1);
        GIVEN(self_equality);

        CALL_PRIMITIVE_INTERFACE;

        IF (result) {
            SUBSTITUTABLE(r1, r2);
        }
        RETURN_RESULT;
    }

    CHECK(r1, r2)
};