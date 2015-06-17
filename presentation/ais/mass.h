// Antibody masses... collections of the best performing antibodies
// Parameters:
//    len  =  total size of the mass
//
// Uses antibody heap to build

// Melissa Danforth
// 02 July 2002


#ifndef __A_MASS_H__
#define __A_MASS_H__

#include <iostream>
#include "heap.h"
#include "antibody.h"
using namespace std;

class AntibodyMass {
  private:
    Antibody **data;
    int len;
  public:
    AntibodyMass(Antibody **, int, int cl = -1);
    AntibodyMass(AntibodyMass &);
    ~AntibodyMass() { killHeap(); }
    Antibody *deleteRandom(int);
    void killHeap();
    friend ostream &operator<<(ostream &, AntibodyMass &);
};

#endif
