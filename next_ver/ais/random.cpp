// Calculate a random integer in a given range or a random coin flip

#include <iostream>
#include <cstdlib>
#include "random.h"

static bool SEEDED = false;

using namespace std;

int randomInt(int bound) {
        double r;
        if (!SEEDED)
        {
                srandom(time(NULL));
                SEEDED = true;
        }

        r = random() / ((double)RAND_MAX + 1);
        return (int)(r * bound);
}

float randomCoin() {
        float r;
        if (!SEEDED)
        {
                srandom(time(NULL));
                SEEDED = true;
        }

        r = random() / ((float)RAND_MAX + 1);
        return r;
}

