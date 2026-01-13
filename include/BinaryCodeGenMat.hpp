#ifndef BINARY_CODE_GEN_MAT_HPP
#define BINARY_CODE_GEN_MAT_HPP

#include <vector>
#include <stdexcept>
#include <iostream>

#include "BinaryCodeWord.hpp"

class BinaryCodeGenMat {
public:
    BinaryCodeGenMat(int k, int n);
    explicit BinaryCodeGenMat(const std::vector<BinaryCodeWord>& rows);

    int numRows() const { return k_; }
    int length()  const { return n_; }

    BinaryCodeWord& operator[](int r);
    const BinaryCodeWord& operator[](int r) const;
    friend std::ostream& operator<<(std::ostream& os, const BinaryCodeGenMat& m);
    BinaryCodeGenMat augment(const BinaryCodeGenMat& rhs) const;
    static BinaryCodeGenMat identity(int size);
    void setEntry(int r, int c, int value);

    // Compare two matrices for equality
    bool operator==(const BinaryCodeGenMat& rhs) const;
    bool operator!=(const BinaryCodeGenMat& rhs) const;

    friend BinaryCodeWord operator*(const BinaryCodeWord& u, const BinaryCodeGenMat& G);
    friend BinaryCodeGenMat operator*(const BinaryCodeGenMat& lhs, const BinaryCodeGenMat& rhs);

private:
    std::vector<BinaryCodeWord> rows_;
    int k_ = 0;
    int n_ = 0;
};

#endif // BINARY_CODE_GEN_MAT_HPP
