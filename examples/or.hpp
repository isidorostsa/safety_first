#pragma once

#include <iostream>

#include "proof_checker_defs.h"
#include "primitives/bool.hpp"
#include <tuple>

struct _or {
    struct _primitive {
        IMPLEMENTATION(r1, r2)
            IF(r1) {
                CALL_INTERFACE(bool, tt, _bool_true);
                return tt;
            }
            IF(r2) {
                CALL_INTERFACE(bool, tt2, _bool_true);
                return tt2;
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

        // or(true, true) = true
        CALL_PRIMITIVE_INTERFACE_ON(or_tt, tt, tt);
        GIVEN(or_tt);

        // or(true, false) = true
        CALL_PRIMITIVE_INTERFACE_ON(or_tf, tt, ff);
        GIVEN(or_tf);

        // or(false, true) = true
        CALL_PRIMITIVE_INTERFACE_ON(or_ft, ff, tt);
        GIVEN(or_ft);

        // or(false, false) = false
        CALL_PRIMITIVE_INTERFACE_ON(or_ff, ff, ff);
        GIVEN_FALSE(or_ff);

        CALL_PRIMITIVE_INTERFACE;

        RETURN_RESULT;
    }

    CHECK(r1, r2)
};