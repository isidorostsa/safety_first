#include "proof_checker_defs.h"
#include "or.hpp"
#include "not.hpp"

TEST(test_or_true_true, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _or, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_or_true_false, r1, r2)
    GIVEN(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _or, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_or_false_true, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _or, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_or_false_false, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _or, r1, r2);
    CLAIM_FALSE(res);
END_TEST(r1, r2);

TEST(test_or_commutative, r1, r2)
    GIVEN(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, or_12, _or, r1, r2);
    CALL_INTERFACE(bool, or_21, _or, r2, r1);
    CLAIM_EQUAL_BOOL(or_12, or_21);
END_TEST(r1, r2);

TEST(test_or_idempotent_true, r1)
    GIVEN(r1);
    CALL_INTERFACE(bool, res, _or, r1, r1);
    CLAIM(res);
END_TEST(r1);

TEST(test_or_idempotent_false, r1)
    GIVEN_FALSE(r1);
    CALL_INTERFACE(bool, res, _or, r1, r1);
    CLAIM_FALSE(res);
END_TEST(r1);

int main() {
    verify_interface<test_or_true_true>();
    verify_interface<test_or_true_false>();
    verify_interface<test_or_false_true>();
    verify_interface<test_or_false_false>();
    verify_interface<test_or_commutative>();
    verify_interface<test_or_idempotent_true>();
    verify_interface<test_or_idempotent_false>();
}
