#ifndef BINARY_CODE_GEN_MAT_HPP
#define BINARY_CODE_GEN_MAT_HPP

#include <vector>
#include <stdexcept>
#include "BinaryCodeWord.hpp"

class BinaryCodeGenMat {
public:
    explicit BinaryCodeGenMat(const std::vector<BinaryCodeWord>& rows);

    int numRows() const { return k_; }
    int length()  const { return n_; }

    BinaryCodeWord& operator[](int r);
    const BinaryCodeWord& operator[](int r) const;

    BinaryCodeGenMat augment(const BinaryCodeGenMat& rhs) const;

    friend BinaryCodeWord operator*(const BinaryCodeWord& u, const BinaryCodeGenMat& G);

private:
    std::vector<BinaryCodeWord> rows_;
    int k_ = 0;
    int n_ = 0;
};

#endif // BINARY_CODE_GEN_MAT_HPP
