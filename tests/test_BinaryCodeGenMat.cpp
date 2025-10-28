#include <gtest/gtest.h>
#include "BinaryCodeWord.hpp"
#include "BinaryCodeGenMat.hpp"

TEST(BinaryCodeGenMat, ConstructAndGetters) {
    BinaryCodeWord r0(5), r1(5);
    r0.setBit(0,1); r0.setBit(2,1);
    r1.setBit(1,1); r1.setBit(4,1);
    BinaryCodeGenMat G({r0, r1});
    EXPECT_EQ(G.numRows(), 2);
    EXPECT_EQ(G.length(), 5);
}

TEST(BinaryCodeGenMat, Augment) {
    BinaryCodeWord a0(3), a1(3);
    a0.setBit(0,1); a0.setBit(2,1);
    a1.setBit(1,1);
    BinaryCodeGenMat A({a0, a1});

    BinaryCodeWord b0(2), b1(2);
    b0.setBit(1,1);
    b1.setBit(0,1); b1.setBit(1,1);
    BinaryCodeGenMat B({b0, b1});

    BinaryCodeGenMat C = A.augment(B);
    EXPECT_EQ(C.numRows(), 2);
    EXPECT_EQ(C.length(), 5);
}

TEST(BinaryCodeGenMat, Multiply) {
    BinaryCodeWord r0(5), r1(5), r2(5);
    r0.setBit(0,1); r0.setBit(4,1);
    r1.setBit(1,1); r1.setBit(2,1);
    r2.setBit(0,1); r2.setBit(1,1);
    BinaryCodeGenMat G({r0, r1, r2});
    BinaryCodeWord u(3);
    u.setBit(0,1);
    u.setBit(2,1);
    BinaryCodeWord v = u * G;
    EXPECT_EQ(v.size(), 5);
}
