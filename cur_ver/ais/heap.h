// Antibody specific heap sort
// Melissa Danforth
// UCD ECS271 W02

#ifndef __A_HEAP_H__
#define __A_HEAP_H__

#include <iostream>
#include "antibody.h"
using namespace std;

class AntibodyHeap {
        private:
                Antibody **data;
                int cls;  // Classification we're building a heap for
                int len;
                void buildHeap();
                void formHeap(int, int);
        public:
                AntibodyHeap(Antibody **, int, int c = -1);
                ~AntibodyHeap() { killHeap(); }
                Antibody *deleteMax();
                void killHeap();
                friend ostream &operator<<(ostream &, AntibodyHeap &);
};

#endif
