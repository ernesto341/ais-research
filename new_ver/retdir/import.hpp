#ifndef _IMPORT_HPP_
#define _IMPORT_HPP_

#pragma once

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include <retglobals.h>
#include <itoa.h>
#include <antibody.h>

using namespace std;

#ifndef DEBUG
#define DEBUG 1
#endif

extern Antibody ** champs;
extern pthread_mutex_t champs_mutex;

//extern volatile sig_atomic_t alen;
//extern volatile sig_atomic_t class_count;
//extern volatile sig_atomic_t ab_count;

volatile static sig_atomic_t alen = 0;
volatile static sig_atomic_t class_count = 0;
volatile static sig_atomic_t ab_count = 0;

extern volatile sig_atomic_t do_import;

extern bool quit;

void * importManager (void *);
Antibody ** importChamps (char * fin = 0);
void onDemandImport (int);

#endif

