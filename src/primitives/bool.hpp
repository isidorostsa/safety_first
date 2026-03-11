#pragma once

#include "proof_checker_defs.h"


struct _bool_true {
    IMPLEMENTATION0()
        // Should not be called
        assert(false);
        return c.make_r_copy(Case::_true);
    }

    INTERFACE0()
        static_assert(not is_being_checked);
        CALL_IMPLEMENTATION;
        IF(result) {
            RETURN_RESULT;
        } else {
            KILL_BRANCH;
        }
    }
};

struct _bool_false {
    IMPLEMENTATION0()
        assert(false);
        return c.make_r_copy(Case::_false);
    }

    INTERFACE0()
        static_assert(not is_being_checked);
        CALL_IMPLEMENTATION;
        IF(result) {
            KILL_BRANCH;
        } else {
            RETURN_RESULT;
        }
    }
};

struct _bool_copy {
    IMPLEMENTATION(r1)
        return c.make_r_copy(r1);
    }

    INTERFACE(r1)
        DISCERN(r1);
        CALL_IMPLEMENTATION;
        DISCERN(result);
        SUBSTITUTABLE(result, r1);
        RETURN_RESULT;
    }

    CHECK(r1)
};
