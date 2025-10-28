#include "BinaryCodeWord.hpp"
#include "BinaryCodeGenMat.hpp"
#include <iostream>

int main() {
    BinaryCodeWord r0(4), r1(4);
    r0.setBit(1,1); r0.setBit(3,1);
    r1.setBit(1,1); r1.setBit(2,1);
    BinaryCodeGenMat G({r0, r1});
    std::cout << "G has k=" << G.numRows() << ", n=" << G.length() << "\n";
    BinaryCodeWord u(2);
    u.setBit(0,1); u.setBit(1,1);
    BinaryCodeWord v = u * G;
    std::cout << "u * G = " << v << "\n";
    BinaryCodeWord s0(2), s1(2);
    s0.setBit(0,1);
    s1.setBit(1,1);
    BinaryCodeGenMat S({s0, s1});
    BinaryCodeGenMat H = G.augment(S);
    std::cout << "H has k=" << H.numRows() << ", n=" << H.length() << "\n";
    return 0;
}
