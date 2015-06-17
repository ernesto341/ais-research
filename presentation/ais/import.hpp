#ifndef __IMPORT_HPP__
#define __IMPORT_HPP__

#pragma once

#include <iostream>
#include <fstream>
#include <string.h> /* strlen, memcpy */
#include <signal.h> /* sig_atomic_t */
#include <stdint.h> /* uint*_t */

#include <sys/stat.h> /* struct stat */

#include "itoa.h"
#include "antibody.h"

using namespace std;

#ifndef DEBUG
#define DEBUG 1
#endif

extern Antibody *pop[CLASS_COUNT][MAX_ANTIBODIES];

Antibody ** importChamps (char * fin = 0);

#endif

