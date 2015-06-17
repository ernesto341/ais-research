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
	try
	{
		data = new Antibody*[len];
	}
	catch (bad_alloc)
	{
		_exit(-1);
	}
	for(int i = 0; i < len; i++) {
		Antibody *tmp = h.deleteMax();
		try
		{
			data[i] = new Antibody(*tmp);
		}
		catch (bad_alloc)
		{
			_exit(-1);
		}
		delete tmp;
		tmp = 000;
	}
	h.killHeap();
}

AntibodyMass::AntibodyMass(AntibodyMass &c) {
	len = c.len;
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
			data[i] = new Antibody(*c.data[i]);
		}
		catch (bad_alloc)
		{
			_exit(-1);
		}
	}
}

Antibody *AntibodyMass::deleteRandom(int max) {
	int m = (max > len) ? len : max;
	int pos = randomInt(m);
	Antibody *ret = data[pos];
	for(int i = pos; i < len - 1; i++) {
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

	return ret;
}

void AntibodyMass::killHeap() {
	for(int i = 0; i < len; i++)
	{
		delete data[i];
		data[i] = 000;
	}
	delete []data;
	data = 000;
	len = 0;
}

ostream &operator<<(ostream &o, AntibodyMass &h) {
	for(int i = 0; i < h.len; i++) {
		o << i << ": " << *h.data[i];
	}
	return o;
}
