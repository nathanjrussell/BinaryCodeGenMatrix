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
    std::cout<<G<<std::endl;
    std::cout<<S<<std::endl;
    BinaryCodeGenMat H = G.augment(S);
    std::cout<<H<<std::endl;
    std::cout << "H has k=" << H.numRows() << ", n=" << H.length() << "\n";

    BinaryCodeGenMat I  = BinaryCodeGenMat::identity(4);
    BinaryCodeGenMat J = I.augment(I);
    std::cout<<J<<std::endl<<std::endl  ;

    BinaryCodeGenMat all1(4,4);
    for (int row =0; row<4; row++) {
        for (int col=0; col<4; col++)
        {
            all1.setEntry(row,col,row==col ? 0 : 1);
        }
    }
    BinaryCodeGenMat EH = BinaryCodeGenMat::identity(4).augment(all1);
    std::cout<<EH<<std::endl;
    BinaryCodeWord vv(4);
    vv.setBit(0,1); vv.setBit(1,1); vv.setBit(3,1);
    BinaryCodeWord result = vv * EH;
    std::cout<<result<<" has weight "<<result.weight()<<std::endl;


    return 0;
}
