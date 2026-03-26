#include "proof_checker_defs.h"
#include "and.hpp"
#include "not.hpp"
#include "or.hpp"

TEST(test_and_true_true, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _and, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_and_true_false, r1, r2)
    GIVEN(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _and, r1, r2);
    CLAIM_FALSE(res);
END_TEST(r1, r2);

TEST(test_and_false_true, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _and, r1, r2);
    CLAIM_FALSE(res);
END_TEST(r1, r2);

TEST(test_and_false_false, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _and, r1, r2);
    CLAIM_FALSE(res);
END_TEST(r1, r2);

TEST(test_and_commutative, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, and_12, _and, r1, r2);
    CALL_INTERFACE(bool, and_21, _and, r2, r1);
    CLAIM_EQUAL_BOOL(and_12, and_21);
END_TEST(r1, r2);

TEST(test_and_idempotent_true, r1)
    GIVEN(r1);
    CALL_INTERFACE(bool, res, _and, r1, r1);
    CLAIM(res);
END_TEST(r1);

TEST(test_and_idempotent_false, r1)
    GIVEN_FALSE(r1);
    CALL_INTERFACE(bool, res, _and, r1, r1);
    CLAIM_FALSE(res);
END_TEST(r1);

// De Morgan: not(and(r1, r2)) = or(not(r1), not(r2))
TEST(test_de_morgan_and, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, and_12, _and, r1, r2);
    CALL_INTERFACE(bool, not_and, _not, and_12);
    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, not_r2, _not, r2);
    CALL_INTERFACE(bool, or_nots, _or, not_r1, not_r2);
    CLAIM_EQUAL_BOOL(not_and, or_nots);
END_TEST(r1, r2);

int main() {
    verify_interface<test_and_true_true>();
    verify_interface<test_and_true_false>();
    verify_interface<test_and_false_true>();
    verify_interface<test_and_false_false>();
    verify_interface<test_and_commutative>();
    verify_interface<test_and_idempotent_true>();
    verify_interface<test_and_idempotent_false>();
    verify_interface<test_de_morgan_and>();
}
