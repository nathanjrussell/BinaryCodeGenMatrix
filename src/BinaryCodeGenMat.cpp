#include "BinaryCodeGenMat/BinaryCodeGenMat.hpp"

#include <algorithm>

namespace {
BinaryCodeGenMat fromRows(const std::vector<BinaryCodeWord>& rows) {
    BinaryCodeGenMat M;
    for (const auto& r : rows) {
        M.pushRow(r);
    }
    M.initialize();
    return M;
}
}

BinaryCodeGenMat::BinaryCodeGenMat(const BinaryCodeGenMat& other)
    : m_initialized(other.m_initialized),
      m_length(other.m_length),
      m_numRows(other.m_numRows),
      m_staging(other.m_staging) {
    if (other.m_rows && other.m_numRows > 0) {
        m_rows = std::make_unique<BinaryCodeWord[]>(static_cast<std::size_t>(other.m_numRows));
        for (int i = 0; i < other.m_numRows; ++i) {
            m_rows[i] = other.m_rows[i];
        }
    }
}

BinaryCodeGenMat& BinaryCodeGenMat::operator=(const BinaryCodeGenMat& other) {
    if (this == &other) return *this;
    m_initialized = other.m_initialized;
    m_length = other.m_length;
    m_numRows = other.m_numRows;
    m_staging = other.m_staging;

    if (other.m_rows && other.m_numRows > 0) {
        m_rows = std::make_unique<BinaryCodeWord[]>(static_cast<std::size_t>(other.m_numRows));
        for (int i = 0; i < other.m_numRows; ++i) {
            m_rows[i] = other.m_rows[i];
        }
    } else {
        m_rows.reset();
    }
    return *this;
}

void BinaryCodeGenMat::pushRow(const BinaryCodeWord& row) {
    if (m_initialized) {
        throw std::logic_error("BinaryCodeGenMat::pushRow: cannot push after initialize()");
    }
    if (!row.initialized()) {
        throw std::logic_error("BinaryCodeGenMat::pushRow: codeword is uninitialized");
    }
    m_staging.push_back(row);
}

void BinaryCodeGenMat::initialize() {
    if (m_initialized) {
        throw std::logic_error("BinaryCodeGenMat::initialize: already initialized");
    }
    if (m_staging.empty()) {
        throw std::logic_error("BinaryCodeGenMat::initialize: no rows supplied");
    }

    m_length = m_staging[0].length();
    for (const auto& r : m_staging) {
        if (r.length() != m_length) {
            throw std::invalid_argument("BinaryCodeGenMat::initialize: row length mismatch");
        }
    }

    m_numRows = static_cast<int>(m_staging.size());
    m_rows = std::make_unique<BinaryCodeWord[]>(static_cast<std::size_t>(m_numRows));
    for (int i = 0; i < m_numRows; ++i) {
        m_rows[i] = m_staging[i];
    }

    m_initialized = true;
}

int BinaryCodeGenMat::length() const {
    requireInitialized();
    return m_length;
}

int BinaryCodeGenMat::numRows() const {
    requireInitialized();
    return m_numRows;
}

const BinaryCodeWord& BinaryCodeGenMat::operator[](int i) const {
    requireInitialized();
    requireValidIndex(i);
    return m_rows[i];
}

BinaryCodeWord& BinaryCodeGenMat::operator[](int i) {
    requireInitialized();
    requireValidIndex(i);
    return m_rows[i];
}

BinaryCodeWord operator*(const BinaryCodeWord& lhs, const BinaryCodeGenMat& rhs){
    if (!lhs.initialized()){
        throw std::logic_error("Left-hand BinaryCodeWord is uninitialized");
    }
    rhs.requireInitialized();
    if (lhs.length() != rhs.numRows()) {
        throw std::invalid_argument("Length of BinaryCodeWord does not match number of rows in BinaryCodeGenMat");
    }
    BinaryCodeWord result(rhs.length());
    for (int row = 0; row < rhs.numRows(); ++row){
        if (lhs.getBit(row) == 1) {
            result += rhs[row];
        }
    }
    return result;
}

void BinaryCodeGenMat::swapColumns(int c1, int c2){
    requireInitialized();
    if (c1 < 0 || c1 >= m_length || c2 < 0 || c2 >= m_length) {
        throw std::out_of_range("Column index out of range");
    }
    if (c1 == c2) return;
    for (int i = 0; i < m_numRows; ++i) {
        m_rows[i].swap(c1, c2); // uses BinaryCodeWord::swap
    }
}

void BinaryCodeGenMat::swapColumns(std::vector<BinaryCodeWord>& rows, int c1, int c2) {
    if (c1 == c2) return;
    for (auto& r : rows) {
        r.swap(c1, c2); // uses BinaryCodeWord::swap
    }
}

BinaryCodeGenMat BinaryCodeGenMat::getSystematic() const {
    requireInitialized();

    std::vector<BinaryCodeWord> work;
    work.reserve(static_cast<std::size_t>(m_numRows));
    for (int i = 0; i < m_numRows; ++i) {
        work.push_back(m_rows[i]);
    }

    int pivotRow = 0;
    int pivotCol = 0;

    while (pivotRow < static_cast<int>(work.size()) && pivotCol < m_length) {
        int sel = -1;
        for (int r = pivotRow; r < static_cast<int>(work.size()); ++r) {
            if (work[r].getBit(pivotCol) == 1) {
                sel = r;
                break;
            }
        }

        if (sel == -1) {
            ++pivotCol;
            continue;
        }

        if (sel != pivotRow) {
            std::swap(work[sel], work[pivotRow]);
        }

        if (pivotCol != pivotRow) {
            swapColumns(work, pivotCol, pivotRow);
            pivotCol = pivotRow;
        }

        for (int r = 0; r < static_cast<int>(work.size()); ++r) {
            if (r == pivotRow) continue;
            if (work[r].getBit(pivotRow) == 1) {
                work[r] += work[pivotRow];
            }
        }

        ++pivotRow;
        ++pivotCol;
    }

    std::vector<BinaryCodeWord> reduced;
    for (auto& r : work) {
        if (!r.isZero()) {
            reduced.push_back(r);
        }
    }

    BinaryCodeGenMat sys;
    for (auto& r : reduced) {
        sys.pushRow(r);
    }
    sys.initialize();
    return sys;
}

int BinaryCodeGenMat::getDim() const {
    requireInitialized();

    // Rank of the row-space over GF(2) using row operations only.
    // Copy rows so we don't mutate *this.
    std::vector<BinaryCodeWord> work;
    work.reserve(static_cast<std::size_t>(m_numRows));
    for (int i = 0; i < m_numRows; ++i) {
        work.push_back(m_rows[i]);
    }

    int rank = 0;
    int pivotCol = 0;

    while (rank < static_cast<int>(work.size()) && pivotCol < m_length) {
        // Find a row with a 1 in pivotCol at or below current rank.
        int sel = -1;
        for (int r = rank; r < static_cast<int>(work.size()); ++r) {
            if (work[r].getBit(pivotCol) == 1) {
                sel = r;
                break;
            }
        }

        if (sel == -1) {
            ++pivotCol;
            continue;
        }

        if (sel != rank) {
            std::swap(work[sel], work[rank]);
        }

        // Eliminate this pivot column from all other rows.
        for (int r = 0; r < static_cast<int>(work.size()); ++r) {
            if (r == rank) continue;
            if (work[r].getBit(pivotCol) == 1) {
                work[r] += work[rank];
            }
        }

        ++rank;
        ++pivotCol;
    }

    return rank;
}

BinaryCodeGenMat BinaryCodeGenMat::transpose() const {
    requireInitialized();

    // If A is k x n, A^T is n x k.
    const int k = m_numRows;
    const int n = m_length;

    std::vector<BinaryCodeWord> rowsT;
    rowsT.reserve(static_cast<std::size_t>(n));
    for (int c = 0; c < n; ++c) {
        BinaryCodeWord rowT(k);
        for (int r = 0; r < k; ++r) {
            rowT.setBit(r, m_rows[r].getBit(c));
        }
        rowsT.push_back(rowT);
    }
    return fromRows(rowsT);
}

BinaryCodeGenMat& BinaryCodeGenMat::operator+=(const BinaryCodeGenMat& rhs) {
    requireInitialized();
    rhs.requireInitialized();
    if (m_length != rhs.m_length || m_numRows != rhs.m_numRows) {
        throw std::invalid_argument("BinaryCodeGenMat::operator+=: dimension mismatch");
    }
    for (int i = 0; i < m_numRows; ++i) {
        m_rows[i] += rhs.m_rows[i];
    }
    return *this;
}

BinaryCodeGenMat& BinaryCodeGenMat::operator*=(const BinaryCodeGenMat& rhs) {
    requireInitialized();
    rhs.requireInitialized();

    // (m_numRows x m_length) * (rhs.m_numRows x rhs.m_length)
    // requires m_length == rhs.m_numRows, result is (m_numRows x rhs.m_length)
    if (m_length != rhs.m_numRows) {
        throw std::invalid_argument("BinaryCodeGenMat::operator*=: dimension mismatch");
    }

    const BinaryCodeGenMat rhsT = rhs.transpose(); // (rhs.m_length x rhs.m_numRows)

    std::vector<BinaryCodeWord> newRows;
    newRows.reserve(static_cast<std::size_t>(m_numRows));
    for (int r = 0; r < m_numRows; ++r) {
        // (1 x m_length) * (m_length x rhs.m_length) -> (1 x rhs.m_length)
        // We compute using transpose so we can reuse BinaryCodeWord * BinaryCodeGenMat:
        // row * rhsT => vector of length rhs.m_numRows?? Wait: rhsT has numRows==rhs.m_length.
        // Requirements: lhs.length == rhsT.numRows(); lhs is this row (length=m_length) and rhsT.numRows()==rhs.m_length.
        // Since m_length == rhs.numRows, rhsT.numRows()==rhs.m_length may differ. So we cannot multiply row by rhsT.
        // Instead compute each output bit as dot(row, column_j(rhs)). Column_j(rhs) is row j of rhsT and has length rhs.numRows()==m_length.
        BinaryCodeWord outRow(rhs.m_length);
        for (int c = 0; c < rhs.m_length; ++c) {
            const BinaryCodeWord& colAsRow = rhsT[c]; // length == rhs.numRows == m_length
            int bit = 0;
            for (int i = 0; i < m_length; ++i) {
                bit ^= (m_rows[r].getBit(i) & colAsRow.getBit(i));
            }
            outRow.setBit(c, bit);
        }
        newRows.push_back(outRow);
    }

    // Replace internal storage
    m_length = rhs.m_length;
    m_rows = std::make_unique<BinaryCodeWord[]>(static_cast<std::size_t>(m_numRows));
    for (int i = 0; i < m_numRows; ++i) {
        m_rows[i] = newRows[i];
    }
    return *this;
}

bool BinaryCodeGenMat::operator==(const BinaryCodeGenMat& rhs) const {
    requireInitialized();
    rhs.requireInitialized();
    if (m_length != rhs.m_length || m_numRows != rhs.m_numRows) return false;
    for (int i = 0; i < m_numRows; ++i) {
        if (!(m_rows[i] == rhs.m_rows[i])) return false;
    }
    return true;
}

BinarySquareMatrix BinarySquareMatrix::power(int exp) const {
    if (exp < 0) {
        throw std::invalid_argument("BinarySquareMatrix::power: exp must be >= 0");
    }
    // Ensure square
    if (numRows() != length()) {
        throw std::logic_error("BinarySquareMatrix::power: matrix is not square");
    }

    const int n = length();
    BinaryIdentityMatrix result(n);

    BinaryCodeGenMat base = *this;
    int e = exp;
    while (e > 0) {
        if (e & 1) {
            result *= base;
        }
        e >>= 1;
        if (e) {
            base *= base;
        }
    }
    // Convert result (a BinaryIdentityMatrix) into a BinarySquareMatrix return type
    BinarySquareMatrix out;
    for (int i = 0; i < result.numRows(); ++i) {
        out.pushRow(result[i]);
    }
    out.initialize();
    return out;
}

BinaryIdentityMatrix::BinaryIdentityMatrix(int size) {
    setSize(size);
}

void BinaryIdentityMatrix::setSize(int size) {
    if (size <= 0) {
        throw std::invalid_argument("BinaryIdentityMatrix::setSize: size must be > 0");
    }
    // Rebuild from scratch (avoid relying on implicit assignment of internal state)
    *this = BinaryIdentityMatrix();
    for (int i = 0; i < size; ++i) {
        BinaryCodeWord r(size);
        r.setBit(i, 1);
        pushRow(r);
    }
    initialize();
}

void BinaryCodeGenMat::requireInitialized() const {
    if (!m_initialized) {
        throw std::logic_error("BinaryCodeGenMat: not initialized");
    }
}

void BinaryCodeGenMat::requireValidIndex(int i) const {
    if (i < 0 || i >= m_numRows) {
        throw std::out_of_range("BinaryCodeGenMat: row index out of range");
    }
}
