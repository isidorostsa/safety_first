#pragma once

#include "proof_checker_defs.h"
#include <cstdint>


template<uint8_t N>
struct _uint8_literal {
    IMPLEMENTATION0()
        assert(false);
        return c.make_r();
    }

    INTERFACE0()
        static_assert(not is_being_checked);
        CALL_IMPLEMENTATION;
        RETURN_RESULT;
    }
};

struct _uint8_copy {
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
