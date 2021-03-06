#pragma once
#include <sys/wait.h>
#include <sys/sem.h>
#include <queue>
#include <list>
#include <cmath>
#include <iomanip>
#include <fstream>

#include <retglobals.h>
#include <mem.hpp>
#include <itoa.h>
#include <antibody.h>
#include <import.hpp>

using namespace std;

#define DEBUG 1

#define AGREE 3

#ifndef SIGBUF
#define SIGBUF 50
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 5
#endif

#ifndef MIN_FITNESS
#define MIN_FITNESS .30
#endif

#ifndef MAX_THREADS
#define MAX_THREADS 6
#endif

#define UNUSED 0
#define START 1
#define WORKING 2
#define DONE 3
#define LOG 4
#define COMPLETE 5

extern uint32_t shmkey[];
extern uint32_t t5shmkey[];

extern int * shmid;
extern int * t5shmid;
extern int * urishmid;
extern volatile sig_atomic_t ** shm;
extern volatile sig_atomic_t ** t5shm;
extern char ** urishm;

extern volatile sig_atomic_t ** retrieved_sigs;
extern volatile sig_atomic_t ** retrieved_t5s;
extern char ** retrieved_uris;

extern sig_atomic_t ct;
extern sig_atomic_t local_pos;

void pull(void);

