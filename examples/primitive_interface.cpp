#include "proof_checker_defs.h"
#include "equal.hpp"
#include "not.hpp"

TEST(test_equals_substitutable, r1, r2)
    SUBSTITUTABLE(r1, r2);
    CALL_IMPLEMENTATION;
    CALL_INTERFACE(bool, same, _equals, r1, r2);
    CLAIM(same);
END_TEST(r1, r2);

TEST(test_not_true_is_false, r1)
    CLAIM(r1);
    CALL_IMPLEMENTATION;
    CALL_INTERFACE(bool, not_r1, _not, r1);
    CLAIM_FALSE(not_r1);
END_TEST(r1);

int main () {
    verify_interface<test_equals_substitutable>();
    verify_interface<test_not_true_is_false>();
}
