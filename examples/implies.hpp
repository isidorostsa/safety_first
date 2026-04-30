#pragma once

#include <iostream>

#include "proof_checker_defs.h"
#include "primitives/bool.hpp"
#include <tuple>

struct _implies {
    struct _primitive {
        IMPLEMENTATION(r1, r2)
            IF(r1) {
                IF(r2) {
                    CALL_INTERFACE(bool, tt, _bool_true);
                    return tt;
                }
                CALL_INTERFACE(bool, ff, _bool_false);
                return ff;
            }
            CALL_INTERFACE(bool, tt2, _bool_true);
            return tt2;
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

        // implies(true, true) = true
        CALL_PRIMITIVE_INTERFACE_ON(imp_tt, tt, tt);
        GIVEN(imp_tt);

        // implies(true, false) = false
        CALL_PRIMITIVE_INTERFACE_ON(imp_tf, tt, ff);
        GIVEN_FALSE(imp_tf);

        // implies(false, true) = true
        CALL_PRIMITIVE_INTERFACE_ON(imp_ft, ff, tt);
        GIVEN(imp_ft);

        // implies(false, false) = true
        CALL_PRIMITIVE_INTERFACE_ON(imp_ff, ff, ff);
        GIVEN(imp_ff);

        CALL_PRIMITIVE_INTERFACE;

        DISCERN(result);

        RETURN_RESULT;
    }

    CHECK(r1, r2)
};