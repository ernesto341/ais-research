// Calculate a random integer in a given range or a random coin flip

#include <iostream>
#include <cstdlib>
#include "random.h"

using namespace std;

int randomInt(int bound) {
        double r;

        r = random() / ((double)RAND_MAX + 1);
        return (int)(r * bound);
}

float randomCoin() {
        float r;

        r = random() / ((float)RAND_MAX + 1);
        return r;
}

