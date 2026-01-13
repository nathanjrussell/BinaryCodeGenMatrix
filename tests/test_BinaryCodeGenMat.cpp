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
    BinaryCodeGenMat H(2,5);
    EXPECT_NE(G, H);
    H.setEntry(0,0,1); H.setEntry(0,2,1);
    H.setEntry(1,1,1); H.setEntry(1,4,1);
    EXPECT_EQ(G, H);
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

TEST(BinaryCodeGenMat, MatrixMatrixMultiplyBasic) {
    // lhs: 2x3
    BinaryCodeWord l0(3), l1(3);
    l0.setBit(0,1); l0.setBit(2,1); // [1 0 1]
    l1.setBit(1,1); l1.setBit(2,1); // [0 1 1]
    BinaryCodeGenMat L({l0, l1});

    // rhs: 3x2 (3 rows, each length 2)
    BinaryCodeWord r0(2), r1(2), r2(2);
    r0.setBit(0,1);            // [1 0]
    r1.setBit(1,1);            // [0 1]
    r2.setBit(0,1); r2.setBit(1,1); // [1 1]
    BinaryCodeGenMat R({r0, r1, r2});

    BinaryCodeGenMat C = L * R; // result should be 2x2
    EXPECT_EQ(C.numRows(), 2);
    EXPECT_EQ(C.length(), 2);

    // expected rows: [0 1], [1 0]
    BinaryCodeWord e0(2), e1(2);
    e0.setBit(1,1);
    e1.setBit(0,1);
    BinaryCodeGenMat E({e0, e1});
    EXPECT_EQ(C, E);
}

TEST(BinaryCodeGenMat, MatrixMultiplyIdentity) {
    // square matrix 3x3
    BinaryCodeWord a0(3), a1(3), a2(3);
    a0.setBit(0,1); a0.setBit(2,1);
    a1.setBit(1,1);
    a2.setBit(0,1); a2.setBit(1,1);
    BinaryCodeGenMat A({a0, a1, a2});

    BinaryCodeGenMat I = BinaryCodeGenMat::identity(3);
    BinaryCodeGenMat P = A * I;
    EXPECT_EQ(P, A);
    P = I * A;
    EXPECT_EQ(P, A);
}

TEST(BinaryCodeGenMat, MatrixMultiplyDimensionMismatch) {
    BinaryCodeGenMat A(2,3); // 2x3
    BinaryCodeGenMat B(4,2); // 4x2 (k=4, n=2) -- incompatible since A.n_ (3) != B.k_ (4)
    EXPECT_THROW(A * B, std::invalid_argument);
}
