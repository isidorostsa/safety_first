#include "proof_checker_defs.h"
#include "equal.hpp"

struct foo {
    IMPLEMENTATION(r1, r2, r3, r4)
        return c.make_r();
    }

    INTERFACE(r1, r2, r3, r4)
        CALL_INTERFACE(bool, b1, _equals, r1, r3);
        CLAIM(b1);

        CALL_INTERFACE(bool, b2, _equals, r2, r4);
        CLAIM(b2);

        CALL_IMPLEMENTATION;

        /*
         * r1 == r3 && r2 == r4
         * ->
         * f(r1, r2) == f(r3, r4)
         */

        CALL_INTERFACE(bool const, b3, _equals, r1, r2);
        CALL_INTERFACE(bool const, b4, _equals, r3, r4);

        IF(b3) {
            CLAIM(b4);
        }

        IF(b4) {
            CLAIM(b3);
        }

        RETURN_RESULT;
    }

    CHECK(r1, r2, r3, r4)
};

int main() {
    verify_interface<foo>();
    return 0;
}
