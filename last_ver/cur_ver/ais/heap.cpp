// Antibody specific heap sort
// Melissa Danforth
// UCD ECS271 W02

#include <iostream>
#include "antibody.h"
#include "heap.h"
using namespace std;

void AntibodyHeap::buildHeap() {
  for(int i = len/2; i >= 0; i--) {
    formHeap(i, len);
  }
}

void AntibodyHeap::formHeap(int l, int h) {
  Antibody *tmp;
  int d = 0;

  if(2*(l+1) >= h) return; // Index out of bounds
  if(2*(l+1) < h && data[2*(l+1)]->fitness(cls) > data[2*(l+1)-1]->fitness(cls)) 
    d = 2*(l+1);       // Right child
  else d = 2*(l+1)-1;  // Left child
  if(data[l]->fitness(cls) < data[d]->fitness(cls)) {
    tmp = data[l];
    data[l] = data[d];
    data[d] = tmp;
    formHeap(d, h);
  }
}

AntibodyHeap::AntibodyHeap(Antibody **d, int l, int c) {
  Antibody *tmp;

  cls = c;
  len = l;
  data = new Antibody*[len];
  for(int i = 0; i < len; i++) {
    data[i] = new Antibody(*d[i]);
  }
  buildHeap();
}

Antibody *AntibodyHeap::deleteMax() {
  Antibody *tmp = data[0];
  for(int i = 0; i < len-1; i++) {
    data[i] = data[i+1];
  }
  data[len-1] = NULL;
  len--;
  buildHeap();
  return tmp;
}

void AntibodyHeap::killHeap() {
  for(int i = 0; i < len; i++) delete data[i];
  len = 0;
}

ostream &operator<<(ostream &o, AntibodyHeap &h) {
  for(int i = 0; i < h.len; i++) {
    o << i << ": " << *h.data[i];
  }
  return o;
}
