#pragma once
#include <globals.h>
#include <queue>

#include <tools.hpp>
#include <mem.hpp>
#include <itoa.h>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

#ifndef LOGLEVEL
#define LOGLEVEL 5
#endif

extern uint32_t shmkey[];
extern uint32_t t5shmkey[];

extern int * shmid;
extern int * t5shmid;
extern volatile sig_atomic_t ** shm;
extern volatile sig_atomic_t ** t5shm;

extern volatile sig_atomic_t ** retrieved_sigs;
extern volatile sig_atomic_t ** retrieved_t5s;

extern sig_atomic_t ct;
extern sig_atomic_t local_pos;

void pull();

