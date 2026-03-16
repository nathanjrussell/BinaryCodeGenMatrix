#include <gtest/gtest.h>

#include <BinaryCodeGenMat/BinaryCodeGenMat.hpp>

#include <random>
#include <set>
#include <stdexcept>
#include <vector>

// -----------------------------
// Helpers
// -----------------------------

static BinaryCodeWord makeWordFromMask(int length, std::uint64_t mask) {
    BinaryCodeWord w(length);
    for (int i = 0; i < length; ++i) {
        w.setBit(i, (mask >> i) & 1ULL);
    }
    return w;
}

static std::uint64_t toMask(const BinaryCodeWord& w) {
    const int L = w.length();
    if (L > 64) throw std::invalid_argument("toMask only supports length <= 64 in tests");
    std::uint64_t mask = 0;
    for (int i = 0; i < L; ++i) {
        if (w.getBit(i) == 1) mask |= (1ULL << i);
    }
    return mask;
}

// Brute-force rank of row set over GF(2) by enumerating all linear combinations.
// Works for small numRows (<= 20-ish). We'll keep tests small.
static int bruteRankGF2(const std::vector<std::uint64_t>& rows) {
    const int m = static_cast<int>(rows.size());
    std::set<std::uint64_t> span;
    span.insert(0ULL);
    for (int i = 0; i < m; ++i) {
        std::set<std::uint64_t> next = span;
        for (auto v : span) next.insert(v ^ rows[i]);
        span.swap(next);
    }
    // span size = 2^rank
    int rank = 0;
    std::size_t sz = span.size();
    while (sz > 1) { sz >>= 1; ++rank; }
    return rank;
}

// Check whether matrix rows are in systematic form on the left:
// For the first k = numRows() rows, columns [0..k-1] should look like identity.
// (This matches your elimination routine which forces pivots into pivotRow column via swaps.)
static void expectLeftIdentity(const BinaryCodeGenMat& S) {
    const int k = S.numRows();
    const int n = S.length();
    ASSERT_LE(k, n) << "Systematic form expects k <= n";
    for (int r = 0; r < k; ++r) {
        for (int c = 0; c < k; ++c) {
            const int expected = (r == c) ? 1 : 0;
            EXPECT_EQ(S[r].getBit(c), expected) << "at row " << r << " col " << c;
        }
    }
}

// Assert all rows are non-zero
static void expectNoZeroRows(const BinaryCodeGenMat& S) {
    for (int i = 0; i < S.numRows(); ++i) {
        EXPECT_FALSE(S[i].isZero()) << "row " << i << " should not be zero";
    }
}

// Compare two matrices for exact row equality
static void expectMatricesEqual(const BinaryCodeGenMat& A, const BinaryCodeGenMat& B) {
    ASSERT_EQ(A.length(), B.length());
    ASSERT_EQ(A.numRows(), B.numRows());
    for (int i = 0; i < A.numRows(); ++i) {
        EXPECT_TRUE(A[i] == B[i]) << "row " << i << " differs";
    }
}

// -----------------------------
// Tests: State machine + validation
// -----------------------------

TEST(BinaryCodeGenMat_State, UninitializedOperationsThrow) {
    BinaryCodeGenMat G;
    EXPECT_THROW((void)G.length(), std::logic_error);
    EXPECT_THROW((void)G.numRows(), std::logic_error);
    EXPECT_THROW((void)G.getSystematic(), std::logic_error);
    EXPECT_THROW((void)G[0], std::logic_error);
}

TEST(BinaryCodeGenMat_State, InitializeWithNoRowsThrows) {
    BinaryCodeGenMat G;
    EXPECT_THROW(G.initialize(), std::logic_error);
}

TEST(BinaryCodeGenMat_State, PushAfterInitializeThrows) {
    BinaryCodeWord r(5);
    r.setBit(0, 1);

    BinaryCodeGenMat G;
    G.pushRow(r);
    G.initialize();

    EXPECT_THROW(G.pushRow(r), std::logic_error);
}

TEST(BinaryCodeGenMat_State, DoubleInitializeThrows) {
    BinaryCodeWord r(5);
    r.setBit(0, 1);

    BinaryCodeGenMat G;
    G.pushRow(r);
    G.initialize();

    EXPECT_THROW(G.initialize(), std::logic_error);
}

TEST(BinaryCodeGenMat_Validation, LengthMismatchThrows) {
    BinaryCodeWord a(5);
    BinaryCodeWord b(6);
    BinaryCodeGenMat G;
    G.pushRow(a);
    G.pushRow(b);
    EXPECT_THROW(G.initialize(), std::invalid_argument);
}

TEST(BinaryCodeGenMat_Validation, IndexOutOfRangeThrows) {
    BinaryCodeWord r(5);
    BinaryCodeGenMat G;
    G.pushRow(r);
    G.initialize();

    EXPECT_THROW((void)G[-1], std::out_of_range);
    EXPECT_THROW((void)G[1], std::out_of_range);
}

// -----------------------------
// Tests: Copy semantics
// -----------------------------

TEST(BinaryCodeGenMat_Semantics, PushRowCopiesValue) {
    BinaryCodeWord r(6);
    r.setBit(0, 1);
    r.setBit(4, 1);

    BinaryCodeGenMat G;
    G.pushRow(r);

    // Mutate original AFTER pushing
    r.setBit(0, 0);
    r.setBit(1, 1);

    G.initialize();

    // Matrix row should reflect the original pushed value, not mutated r
    EXPECT_EQ(G.length(), 6);
    EXPECT_EQ(G.numRows(), 1);
    EXPECT_EQ(G[0].getBit(0), 1);
    EXPECT_EQ(G[0].getBit(1), 0);
    EXPECT_EQ(G[0].getBit(4), 1);
}

// -----------------------------
// Tests: Systematic form properties
// -----------------------------

TEST(BinaryCodeGenMat_Systematic, DoesNotMutateOriginal) {
    BinaryCodeWord r1(6);
    r1.setBit(0, 1);
    r1.setBit(3, 1);

    BinaryCodeWord r2(6);
    r2.setBit(1, 1);
    r2.setBit(4, 1);

    BinaryCodeGenMat G;
    G.pushRow(r1);
    G.pushRow(r2);
    G.initialize();

    // Snapshot original
    BinaryCodeWord orig0 = G[0];
    BinaryCodeWord orig1 = G[1];

    BinaryCodeGenMat S = G.getSystematic();

    // Original unchanged
    EXPECT_TRUE(G[0] == orig0);
    EXPECT_TRUE(G[1] == orig1);

    // Systematic is initialized and same length
    EXPECT_EQ(S.length(), G.length());
}

TEST(BinaryCodeGenMat_Systematic, DuplicateRowsCollapse) {
    BinaryCodeWord r(6);
    r.setBit(0, 1);
    r.setBit(3, 1);

    BinaryCodeGenMat G;
    G.pushRow(r);
    G.pushRow(r);
    G.initialize();

    EXPECT_EQ(G.numRows(), 2);

    BinaryCodeGenMat S = G.getSystematic();
    EXPECT_EQ(S.length(), 6);
    EXPECT_EQ(S.numRows(), 1); // rank should be 1 after reduction
    expectNoZeroRows(S);
    expectLeftIdentity(S);
}

TEST(BinaryCodeGenMat_Systematic, ZeroRowsRemoved) {
    BinaryCodeWord z(6); // all zeros
    BinaryCodeWord r(6);
    r.setBit(2, 1);

    BinaryCodeGenMat G;
    G.pushRow(z);
    G.pushRow(r);
    G.pushRow(z);
    G.initialize();

    BinaryCodeGenMat S = G.getSystematic();
    EXPECT_EQ(S.numRows(), 1);
    EXPECT_FALSE(S[0].isZero());
    expectLeftIdentity(S);
}

TEST(BinaryCodeGenMat_Systematic, Idempotent) {
    // Random-ish small example
    BinaryCodeWord r1(8);
    r1.setBit(0, 1);
    r1.setBit(3, 1);
    r1.setBit(7, 1);

    BinaryCodeWord r2(8);
    r2.setBit(1, 1);
    r2.setBit(3, 1);
    r2.setBit(5, 1);

    BinaryCodeWord r3(8);
    r3.setBit(0, 1);
    r3.setBit(1, 1);

    BinaryCodeGenMat G;
    G.pushRow(r1);
    G.pushRow(r2);
    G.pushRow(r3);
    G.initialize();

    BinaryCodeGenMat S1 = G.getSystematic();
    BinaryCodeGenMat S2 = S1.getSystematic();

    expectMatricesEqual(S1, S2);
    expectLeftIdentity(S1);
    expectNoZeroRows(S1);
}

// -----------------------------
// Strong correctness: rank agreement with brute force
// -----------------------------

TEST(BinaryCodeGenMat_Rank, RandomSmallMatchesBruteForceRank) {
    // We use length <= 16 so we can brute-force span safely.
    constexpr int TRIALS = 200;
    constexpr int MAX_ROWS = 10;
    constexpr int LENGTH = 16;

    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_int_distribution<int> rowsDist(1, MAX_ROWS);
    std::uniform_int_distribution<std::uint64_t> maskDist(0, (1ULL << LENGTH) - 1);

    for (int t = 0; t < TRIALS; ++t) {
        const int m = rowsDist(rng);

        BinaryCodeGenMat G;
        std::vector<std::uint64_t> rowMasks;
        rowMasks.reserve(m);

        for (int i = 0; i < m; ++i) {
            std::uint64_t mask = maskDist(rng);
            rowMasks.push_back(mask);
            G.pushRow(makeWordFromMask(LENGTH, mask));
        }
        G.initialize();

        const int brute = bruteRankGF2(rowMasks);

        BinaryCodeGenMat S = G.getSystematic();
        const int k = S.numRows();

        EXPECT_EQ(k, brute) << "trial " << t << " mismatch";
        EXPECT_EQ(S.length(), LENGTH);

        // Systematic structure should hold whenever k <= n (always true)
        expectLeftIdentity(S);
        expectNoZeroRows(S);
    }
}

// -----------------------------
// Specific corner cases
// -----------------------------

TEST(BinaryCodeGenMat_Corners, FullRankSquareBecomesIdentity) {
    // n=k=6 with identity rows -> should stay identity in systematic
    BinaryCodeGenMat G;
    for (int i = 0; i < 6; ++i) {
        BinaryCodeWord r(6);
        r.setBit(i, 1);
        G.pushRow(r);
    }
    G.initialize();

    BinaryCodeGenMat S = G.getSystematic();
    EXPECT_EQ(S.numRows(), 6);
    expectLeftIdentity(S);
}

TEST(BinaryCodeGenMat_Corners, MoreRowsThanColumnsRankAtMostN) {
    constexpr int n = 5;
    constexpr int m = 12;

    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<std::uint64_t> dist(0, (1ULL << n) - 1);

    BinaryCodeGenMat G;
    std::vector<std::uint64_t> masks;
    masks.reserve(m);

    for (int i = 0; i < m; ++i) {
        std::uint64_t mask = dist(rng);
        masks.push_back(mask);
        G.pushRow(makeWordFromMask(n, mask));
    }
    G.initialize();

    BinaryCodeGenMat S = G.getSystematic();
    EXPECT_LE(S.numRows(), n);

    const int brute = bruteRankGF2(masks);
    EXPECT_EQ(S.numRows(), brute);
    expectLeftIdentity(S);
    expectNoZeroRows(S);
}



TEST(BinaryCodeGenMat, SwapColumns) {
    BinaryCodeGenMat G;

    // Build rows using setBit only (no masks, no bit-order assumption beyond indices).
    BinaryCodeWord r0(10);
    r0.setBit(0, 1);
    r0.setBit(1, 1);
    BinaryCodeWord r1(10);
    r1.setBit(0, 1);
    BinaryCodeWord r2(10); /* all zeros */
    BinaryCodeWord r3(10);
    r3.setBit(9, 1);

    G.pushRow(r0);
    G.pushRow(r1);
    G.pushRow(r2);
    G.pushRow(r3);
    G.initialize();

    // Pre-swap sanity
    EXPECT_EQ(G[0].getBit(0), 1);
    EXPECT_EQ(G[0].getBit(9), 0);
    EXPECT_EQ(G[3].getBit(0), 0);
    EXPECT_EQ(G[3].getBit(9), 1);

    // Swap columns 0 and 9: bits in those positions should exchange for every row.
    G.swapColumns(0, 9);
    EXPECT_EQ(G[0].getBit(0), 0);
    EXPECT_EQ(G[0].getBit(9), 1);
    EXPECT_EQ(G[1].getBit(0), 0);
    EXPECT_EQ(G[1].getBit(9), 1);
    EXPECT_EQ(G[2].getBit(0), 0);
    EXPECT_EQ(G[2].getBit(9), 0);
    EXPECT_EQ(G[3].getBit(0), 1);
    EXPECT_EQ(G[3].getBit(9), 0);

    // Swap back should restore original.
    G.swapColumns(9, 0);
    EXPECT_EQ(G[0].getBit(0), 1);
    EXPECT_EQ(G[0].getBit(9), 0);
    EXPECT_EQ(G[1].getBit(0), 1);
    EXPECT_EQ(G[1].getBit(9), 0);
    EXPECT_EQ(G[2].getBit(0), 0);
    EXPECT_EQ(G[2].getBit(9), 0);
    EXPECT_EQ(G[3].getBit(0), 0);
    EXPECT_EQ(G[3].getBit(9), 1);

    // Also exercise a different pair.
    EXPECT_EQ(G[0].getBit(1), 1);
    EXPECT_EQ(G[0].getBit(8), 0);

    G.swapColumns(1, 8);
    EXPECT_EQ(G[0].getBit(1), 0);
    EXPECT_EQ(G[0].getBit(8), 1);

    G.swapColumns(8, 1);
    EXPECT_EQ(G[0].getBit(1), 1);
    EXPECT_EQ(G[0].getBit(8), 0);

}

TEST(BinaryCodeGenMat, CodewordMultiplication) {
    BinaryCodeGenMat G;

    BinaryCodeWord r0(5);
    r0.setBit(0, 1);
    r0.setBit(2, 1);

    BinaryCodeWord r1(5);
    r1.setBit(1, 1);
    r1.setBit(3, 1);

    G.pushRow(r0);
    G.pushRow(r1);
    G.initialize();

    BinaryCodeWord u(2); // length should match numRows
    u.setBit(0, 1); // select r0 only
    BinaryCodeWord c = u * G;
    EXPECT_EQ(c.length(), 5);
    EXPECT_EQ(c.getBit(0), 1);
    EXPECT_EQ(c.getBit(1), 0);
    EXPECT_EQ(c.getBit(2), 1);
    EXPECT_EQ(c.getBit(3), 0);
    EXPECT_EQ(c.getBit(4), 0);

    u.setBit(0, 0);
    u.setBit(1, 1); // select r1 only
    c = u * G;
    EXPECT_EQ(c.getBit(0), 0);
    EXPECT_EQ(c.getBit(1), 1);
    EXPECT_EQ(c.getBit(2), 0);
    EXPECT_EQ(c.getBit(3), 1);
    EXPECT_EQ(c.getBit(4), 0);

    u.setBit(0, 1);
    u.setBit(1, 1); // select both rows
    c = u * G;
    EXPECT_EQ(c.getBit(0), 1); // r0 and r1 add in bit 0
    EXPECT_EQ(c.getBit(1), 1); // r0 and r1 add in bit 1
    EXPECT_EQ(c.getBit(2), 1); // r0 only in bit 2
    EXPECT_EQ(c.getBit(3), 1); // r1 only in bit 3
    EXPECT_EQ(c.getBit(4), 0); // neither row has bit 4
}

// -----------------------------
// New tests: transpose + arithmetic
// -----------------------------

TEST(BinaryCodeGenMat_Arithmetic, TransposeRoundTrip) {
    BinaryCodeGenMat G;
    BinaryCodeWord r0(4);
    r0.setBit(1, 1);
    r0.setBit(3, 1);
    BinaryCodeWord r1(4);
    r1.setBit(0, 1);
    BinaryCodeWord r2(4);
    r2.setBit(2, 1);
    r2.setBit(3, 1);
    G.pushRow(r0);
    G.pushRow(r1);
    G.pushRow(r2);
    G.initialize();

    BinaryCodeGenMat T = G.transpose();
    EXPECT_EQ(T.numRows(), G.length());
    EXPECT_EQ(T.length(), G.numRows());

    BinaryCodeGenMat TT = T.transpose();
    EXPECT_TRUE(TT == G);
}

TEST(BinaryCodeGenMat_Arithmetic, AddAndEquals) {
    BinaryCodeGenMat A;
    BinaryCodeWord a0(5);
    a0.setBit(0, 1);
    a0.setBit(2, 1);
    BinaryCodeWord a1(5);
    a1.setBit(1, 1);
    A.pushRow(a0);
    A.pushRow(a1);
    A.initialize();

    BinaryCodeGenMat B;
    BinaryCodeWord b0(5);
    b0.setBit(2, 1);
    b0.setBit(4, 1);
    BinaryCodeWord b1(5);
    b1.setBit(1, 1);
    b1.setBit(3, 1);
    B.pushRow(b0);
    B.pushRow(b1);
    B.initialize();

    BinaryCodeGenMat C = A + B;
    EXPECT_EQ(C.length(), 5);
    EXPECT_EQ(C.numRows(), 2);

    // Expected rows are XOR
    BinaryCodeWord e0(5);
    e0.setBit(0, 1);
    e0.setBit(4, 1);
    BinaryCodeWord e1(5);
    e1.setBit(3, 1);

    EXPECT_TRUE(C[0] == e0);
    EXPECT_TRUE(C[1] == e1);
    EXPECT_TRUE((C != A));
    EXPECT_TRUE((A == A));

    A += B;
    EXPECT_TRUE(A == C);
}

TEST(BinaryCodeGenMat_Arithmetic, MultiplySmall) {
    // A is 2x3
    BinaryCodeGenMat A;
    BinaryCodeWord a0(3);
    a0.setBit(0, 1);
    a0.setBit(2, 1);
    BinaryCodeWord a1(3);
    a1.setBit(1, 1);
    A.pushRow(a0);
    A.pushRow(a1);
    A.initialize();

    // B is 3x4
    BinaryCodeGenMat B;
    BinaryCodeWord b0(4);
    b0.setBit(0, 1);
    b0.setBit(1, 1);
    BinaryCodeWord b1(4);
    b1.setBit(1, 1);
    b1.setBit(2, 1);
    BinaryCodeWord b2(4);
    b2.setBit(3, 1);
    B.pushRow(b0);
    B.pushRow(b1);
    B.pushRow(b2);
    B.initialize();

    BinaryCodeGenMat C = A * B; // 2x4
    EXPECT_EQ(C.numRows(), 2);
    EXPECT_EQ(C.length(), 4);

    // Row0 = b0 + b2
    BinaryCodeWord e0 = b0;
    e0 += b2;
    // Row1 = b1
    BinaryCodeWord e1 = b1;

    EXPECT_TRUE(C[0] == e0);
    EXPECT_TRUE(C[1] == e1);

    BinaryCodeGenMat C2 = A;
    C2 *= B;
    EXPECT_TRUE(C2 == C);
}

TEST(BinarySquareMatrix, PowerAndIdentity) {
    // Build a simple 3x3 matrix
    BinarySquareMatrix A;
    BinaryCodeWord r0(3);
    r0.setBit(0, 1);
    r0.setBit(1, 1);
    BinaryCodeWord r1(3);
    r1.setBit(1, 1);
    r1.setBit(2, 1);
    BinaryCodeWord r2(3);
    r2.setBit(0, 1);
    r2.setBit(2, 1);
    A.pushRow(r0);
    A.pushRow(r1);
    A.pushRow(r2);
    A.initialize();

    BinarySquareMatrix A0 = A.power(0);
    BinaryIdentityMatrix I(3);
    EXPECT_TRUE(A0 == I);

    BinarySquareMatrix A1 = A.power(1);
    EXPECT_TRUE(A1 == A);

    BinarySquareMatrix A2 = A.power(2);
    BinaryCodeGenMat brute = A * A;
    EXPECT_TRUE(A2 == brute);

    // Identity resizing
    BinaryIdentityMatrix I2;
    I2.setSize(4);
    EXPECT_EQ(I2.numRows(), 4);
    EXPECT_EQ(I2.length(), 4);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_EQ(I2[i].getBit(j), (i == j) ? 1 : 0);
        }
    }
}

TEST(BinaryCodeGenMat_Dimension, GetDimMatchesBruteForceAndDoesNotMutate) {
    BinaryCodeGenMat G;
    // 4 columns, 5 rows with dependencies and a zero row
    BinaryCodeWord r0(4); // 1100
    r0.setBit(0, 1);
    r0.setBit(1, 1);
    BinaryCodeWord r1(4); // 0110
    r1.setBit(1, 1);
    r1.setBit(2, 1);
    BinaryCodeWord r2(4); // r0 + r1 = 1010 (dependent)
    r2 = r0;
    r2 += r1;
    BinaryCodeWord r3(4); // zero
    BinaryCodeWord r4(4); // 0001 independent from r0,r1
    r4.setBit(3, 1);

    G.pushRow(r0);
    G.pushRow(r1);
    G.pushRow(r2);
    G.pushRow(r3);
    G.pushRow(r4);
    G.initialize();

    // Snapshot original rows
    std::vector<BinaryCodeWord> orig;
    for (int i = 0; i < G.numRows(); ++i) orig.push_back(G[i]);

    // Brute-force rank on masks (bit i corresponds to column i)
    std::vector<std::uint64_t> masks;
    masks.reserve(static_cast<std::size_t>(G.numRows()));
    for (int i = 0; i < G.numRows(); ++i) masks.push_back(toMask(G[i]));
    const int brute = bruteRankGF2(masks);

    EXPECT_EQ(G.getDim(), brute);

    // Verify not mutated
    for (int i = 0; i < G.numRows(); ++i) {
        EXPECT_TRUE(G[i] == orig[i]);
    }
}

