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
	try
	{
		data = new Antibody*[len];
	}
	catch (bad_alloc)
	{
		_exit(-1);
	}
	for(int i = 0; i < len; i++) {
		try
		{
			data[i] = new Antibody(*d[i]);
		}
		catch (bad_alloc)
		{
			_exit(-1);
		}
	}
	buildHeap();
}

Antibody *AntibodyHeap::deleteMax() {
	Antibody *ret = data[0];
	for(int i = 0; i < len - 1; i++) {
		data[i] = data[i+1];
	}
	len--;
	Antibody ** tmp = 000;
	try
	{
		tmp = new Antibody*[len];
	}
	catch (bad_alloc)
	{
		_exit(-1);
	}
	for(int i = 0; i < len; i++) {
		try
		{
			tmp[i] = new Antibody(*data[i]);
		}
		catch (bad_alloc)
		{
			_exit(-1);
		}
		delete data[i];
		data[i] = 000;
	}
	delete [] data;
	data = tmp;

	buildHeap();
	return ret;
}

void AntibodyHeap::killHeap() {
	for(int i = 0; i < len; i++)
	{
		delete data[i];
		data[i] = 000;
	}
	delete [] data;
	data = 000;
	len = 0;
}

ostream &operator<<(ostream &o, AntibodyHeap &h) {
	for(int i = 0; i < h.len; i++) {
		o << i << ": " << *h.data[i];
	}
	return o;
}
