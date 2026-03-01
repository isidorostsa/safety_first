#include "proof_checker_defs.h"
#include "equal.hpp"
#include "not.hpp"


struct _test_equal {
    IMPLEMENTATION(r1, r2)
        RETURN_VOID;
    }

    INTERFACE(r1, r2)
        SUBSTITUTABLE(r1, r2);
        CALL_IMPLEMENTATION;
        CALL_INTERFACE(bool, same, _equals, r1, r2);
        CLAIM(same);
        RETURN_RESULT;
    }

    CHECK(r1, r2)
};

struct _test_not {
    IMPLEMENTATION(r1)
        RETURN_VOID;
    }

    INTERFACE( r1)
        CLAIM(r1);
        CALL_IMPLEMENTATION;
        CALL_INTERFACE(bool, not_r1, _not, r1);
        CLAIM_FALSE(not_r1);
        RETURN_RESULT;
    }

    CHECK(r1)
};
int main () {
    verify_interface<_test_equal>();
    verify_interface<_test_not>();
}