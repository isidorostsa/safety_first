#pragma once

#include <iostream>

#include "proof_checker_defs.h"
#include "implies.hpp"
#include "equal.hpp"
#include <tuple>

struct _not {
    struct _primitive {
        IMPLEMENTATION(r1)
            c.ledger.dump_graph();
            IF(r1) {
                CALL_INTERFACE(bool, ff, _bool_false);
                return ff;
            }
            CALL_INTERFACE(bool, tt, _bool_true);
            return tt;
        }

        INTERFACE(r1)
            DISCERN(r1);
            CALL_IMPLEMENTATION;
            DISCERN(result);
            RETURN_RESULT;
        }
    };

    IMPLEMENTATION(r1)
        DISCERN(r1);
        CALL_PRIMITIVE_INTERFACE_ON(result, r1);
        DISCERN(result);
        RETURN_RESULT;
    }

    INTERFACE(r1)
        DISCERN(r1);

        CALL_INTERFACE(bool, tt, _bool_true);
        CALL_INTERFACE(bool, ff, _bool_false);

        // not(true) = false
        CALL_PRIMITIVE_INTERFACE_ON(not_tt, tt);
        GIVEN_FALSE(not_tt);

        // not(false) = true
        CALL_PRIMITIVE_INTERFACE_ON(not_ff, ff);
        GIVEN(not_ff);

        CALL_PRIMITIVE_INTERFACE;

        CALL_INTERFACE(bool, notnot_r1, _primitive ,result);
        CALL_INTERFACE(bool, r1_is_notnot_r1, _equals, notnot_r1, r1);
        CLAIM(r1_is_notnot_r1);

        RETURN_RESULT;
    }

    CHECK(r1)
};
