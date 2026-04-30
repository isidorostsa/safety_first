#pragma once

#include "proof_checker_defs.h"


struct _bool_true {
    IMPLEMENTATION0()
        // Should not be called
        std::println("Was called");
        assert(false);
        return c.make_r_copy(Case::_true);
    }

    INTERFACE0()
        static_assert(not is_being_checked);
        CALL_IMPLEMENTATION;
        CLAIM(result);
        RETURN_RESULT;
    }
};

struct _bool_false {
    IMPLEMENTATION0()
        std::println("Was called");
        assert(false);
        return c.make_r_copy(Case::_false);
    }

    INTERFACE0()
        static_assert(not is_being_checked);
        CALL_IMPLEMENTATION;
        CLAIM_FALSE(result);
        RETURN_RESULT;
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
