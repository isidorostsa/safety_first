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

TEST(test_not_preserves_equals, r1, r2)
    CALL_INTERFACE(bool, r1_eq_r2, _equals, r1, r2);
    CLAIM(r1_eq_r2);

    CALL_IMPLEMENTATION;

    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, not_r2, _not, r2);

    CLAIM_EQUAL_BOOL(not_r1, not_r2);
END_TEST(r1, r2);

TEST(test_not_not_is_original, r1)
    CLAIM(r1);

    CALL_IMPLEMENTATION;

    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, not_not_r1, _not, not_r1);

    CLAIM(not_not_r1);
END_TEST(r1);

TEST(test_not_not_is_original_generic, r1)
    CALL_IMPLEMENTATION;

    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, not_not_r1, _not, not_r1);

    CLAIM_EQUAL_BOOL(r1, not_not_r1);
END_TEST(r1);



int main () {
    verify_interface<test_equals_substitutable>();
    verify_interface<test_not_true_is_false>();
    verify_interface<test_not_preserves_equals>();
    verify_interface<test_not_not_is_original>();
    verify_interface<test_not_not_is_original_generic>();
}
