// Antibody masses... collections of the best performing antibodies
// Uses antibody heap to build

// Melissa Danforth
// 02 July 2002

#include <iostream>
#include <cstdlib>
#include "heap.h"
#include "antibody.h"
#include "mass.h"
#include "random.h"
using namespace std;

AntibodyMass::AntibodyMass(Antibody **d, int l, int cl) {
  AntibodyHeap h(d, l, cl);
  len = l;
  data = new Antibody*[len];
  for(int i = 0; i < len; i++) {
    Antibody *tmp = h.deleteMax();
    data[i] = new Antibody(*tmp);
    delete tmp;
  }
  h.killHeap();
}

AntibodyMass::AntibodyMass(AntibodyMass &c) {
  len = c.len;
  data = new Antibody*[len];
  for(int i = 0; i < len; i++) {
    data[i] = new Antibody(*c.data[i]);
  }
}

Antibody *AntibodyMass::deleteRandom(int max) {
  int m = (max > len) ? len : max;
  int pos = randomInt(m);
  Antibody *tmp = data[pos];
  for(int i = pos; i < len - 1; i++) {
    data[i] = data[i+1];
  }
  len--;
  return tmp;
}

void AntibodyMass::killHeap() {
  for(int i = 0; i < len; i++) delete data[i];
  len = 0;
}

ostream &operator<<(ostream &o, AntibodyMass &h) {
  for(int i = 0; i < h.len; i++) {
    o << i << ": " << *h.data[i];
  }
  return o;
}
