
#include "proof_checker_defs.h"
#include "equal.hpp"

struct _assign_true {
    IMPLEMENTATION(r1, r2)
        RETURN_VOID;
    }

    INTERFACE(r1, r2)
        TRANSFER(r1);
        TRANSFER(r2);
        CLAIM(r2);
        TRANSFER(r1);
        TRANSFER(r2);
        CALL_IMPLEMENTATION;
        TRANSFER(r1);
        TRANSFER(r2);
        CLAIM(r1);
        TRANSFER(r1);
        TRANSFER(r2);
        RETURN_VOID;
    }
};

struct _and {
    IMPLEMENTATION(r1, r2)
        CALL_INTERFACE(void, _, _assign_true, r1, r2);
        RETURN_VOID;
    }

    INTERFACE(r1, r2)
        TRANSFER(r1);
        TRANSFER(r2);

        DISCERN(r1);
        DISCERN(r2);

        CLAIM(r1);

        CALL_IMPLEMENTATION;
        TRANSFER(r1);
        TRANSFER(r2);

        // This should fail, because _alter takes away the right to a's value.
        CLAIM(r1);

        RETURN_VOID;
    }

    CHECK(r1, r2)
};

int main() {
    verify_interface<_and>();
    return 0;
}