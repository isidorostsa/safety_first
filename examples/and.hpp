#pragma once

#include <iostream>

#include "proof_checker_defs.h"
#include "primitives/bool.hpp"
#include <tuple>

struct _and {
    struct _primitive {
        IMPLEMENTATION(r1, r2)
            IF(r1) {
                IF(r2) {
                    CALL_INTERFACE(bool, tt, _bool_true);
                    return tt;
                }
            }
            CALL_INTERFACE(bool, ff, _bool_false);
            return ff;
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
        CALL_PRIMITIVE_INTERFACE_ON(result, r1, r2);
        RETURN_RESULT;
    }

    INTERFACE(r1, r2)
        DISCERN(r1);
        DISCERN(r2);

        CALL_INTERFACE(bool, tt, _bool_true);
        CALL_INTERFACE(bool, ff, _bool_false);

        // and(true, true) = true
        CALL_PRIMITIVE_INTERFACE_ON(and_tt, tt, tt);
        GIVEN(and_tt);

        // and(true, false) = false
        CALL_PRIMITIVE_INTERFACE_ON(and_tf, tt, ff);
        GIVEN_FALSE(and_tf);

        // and(false, true) = false
        CALL_PRIMITIVE_INTERFACE_ON(and_ft, ff, tt);
        GIVEN_FALSE(and_ft);

        // and(false, false) = false
        CALL_PRIMITIVE_INTERFACE_ON(and_ff, ff, ff);
        GIVEN_FALSE(and_ff);

        CALL_PRIMITIVE_INTERFACE;

        RETURN_RESULT;
    }

    CHECK(r1, r2)
};