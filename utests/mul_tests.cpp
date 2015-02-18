#include "mul.h"
#include <gtest/gtest.h>

namespace {

    class MulTest : public ::testing::Test {
        protected:
            virtual void SetUp() {
                //m1;
            }

            MyMath m1;
    };

    TEST_F(MulTest, Zero) {
            EXPECT_EQ(0, m1.mul(0, 0));
    }

    TEST_F(MulTest, Ones) {
            EXPECT_EQ(1, m1.mul(1, 1));
    }

    TEST_F(MulTest, Fives) {
            EXPECT_EQ(25, m1.mul(5, 5));
    }

} // namespace

int main (int argc, char **argv) {
        ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
}

