#include "gtest/gtest.h"
#include <sample.hh>

TEST (AdditionTest, PositiveAdd) {
    // Given
    int a = 42;
    int b = 69;
    // When
    int res = example::add_int(a, b);
    // Then
    EXPECT_EQ (a + b, res);
}

