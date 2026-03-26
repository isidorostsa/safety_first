#include "proof_checker_defs.h"
#include "implies.hpp"
#include "not.hpp"
#include "or.hpp"

TEST(test_implies_true_true, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _implies, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_implies_true_false, r1, r2)
    GIVEN(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _implies, r1, r2);
    CLAIM_FALSE(res);
END_TEST(r1, r2);

TEST(test_implies_false_true, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, res, _implies, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_implies_false_false, r1, r2)
    GIVEN_FALSE(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, res, _implies, r1, r2);
    CLAIM(res);
END_TEST(r1, r2);

TEST(test_implies_reflexive, r1)
    GIVEN(r1);
    CALL_INTERFACE(bool, res, _implies, r1, r1);
    CLAIM(res);
END_TEST(r1);

// implies(r1, r2) = or(not(r1), r2)
TEST(test_implies_is_or_not, r1, r2)
    GIVEN(r1);
    GIVEN(r2);
    CALL_INTERFACE(bool, imp, _implies, r1, r2);
    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, or_not_r1_r2, _or, not_r1, r2);
    CLAIM_EQUAL_BOOL(imp, or_not_r1_r2);
END_TEST(r1, r2);

// contrapositive: implies(r1, r2) = implies(not(r2), not(r1))
TEST(test_implies_contrapositive, r1, r2)
    GIVEN(r1);
    GIVEN_FALSE(r2);
    CALL_INTERFACE(bool, imp, _implies, r1, r2);
    CALL_INTERFACE(bool, not_r1, _not, r1);
    CALL_INTERFACE(bool, not_r2, _not, r2);
    CALL_INTERFACE(bool, contra, _implies, not_r2, not_r1);
    CLAIM_EQUAL_BOOL(imp, contra);
END_TEST(r1, r2);

int main() {
    verify_interface<test_implies_true_true>();
    verify_interface<test_implies_true_false>();
    verify_interface<test_implies_false_true>();
    verify_interface<test_implies_false_false>();
    verify_interface<test_implies_reflexive>();
    verify_interface<test_implies_is_or_not>();
    verify_interface<test_implies_contrapositive>();
}
