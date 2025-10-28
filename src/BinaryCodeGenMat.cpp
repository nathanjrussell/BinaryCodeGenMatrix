#include "BinaryCodeGenMat.hpp"

BinaryCodeGenMat::BinaryCodeGenMat(const std::vector<BinaryCodeWord>& rows)
    : rows_(rows), k_(static_cast<int>(rows.size())), n_(0)
{
    if (k_ == 0) throw std::invalid_argument("BinaryCodeGenMat: must have at least one row");
    n_ = rows_[0].size();
    for (int i = 1; i < k_; ++i)
        if (rows_[i].size() != n_)
            throw std::invalid_argument("BinaryCodeGenMat: all rows must have the same length");
}

BinaryCodeWord& BinaryCodeGenMat::operator[](int r) {
    if (r < 0 || r >= k_) throw std::out_of_range("row index out of range");
    return rows_[r];
}

const BinaryCodeWord& BinaryCodeGenMat::operator[](int r) const {
    if (r < 0 || r >= k_) throw std::out_of_range("row index out of range");
    return rows_[r];
}

BinaryCodeGenMat BinaryCodeGenMat::augment(const BinaryCodeGenMat& rhs) const {
    if (rhs.k_ != k_) throw std::invalid_argument("augment: row counts must match");
    int newN = n_ + rhs.n_;
    std::vector<BinaryCodeWord> newRows;
    newRows.reserve(k_);

    for (int i = 0; i < k_; ++i) {
        BinaryCodeWord row(newN);
        for (int c = 0; c < n_; ++c) row.setBit(c, rows_[i].getBit(c));
        for (int c = 0; c < rhs.n_; ++c) row.setBit(n_ + c, rhs.rows_[i].getBit(c));
        newRows.push_back(row);
    }
    return BinaryCodeGenMat(newRows);
}

BinaryCodeWord operator*(const BinaryCodeWord& u, const BinaryCodeGenMat& G) {
    if (u.size() != G.k_) throw std::invalid_argument("dimension mismatch in multiplication");
    BinaryCodeWord result(G.n_);
    for (int i = 0; i < G.k_; ++i)
        if (u.getBit(i)) result += G.rows_[i];
    return result;
}
