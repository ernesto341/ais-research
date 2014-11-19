#pragma once
#include <sys/wait.h>
#include <queue>
#include <list>
#include <iomanip>
#include <fstream>

#include <retglobals.h>
#include <tools.hpp>
#include <mem.hpp>
#include <itoa.h>
#include <antibody.h>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 5
#endif

#ifndef MIN_FITNESS
#define MIN_FITNESS 70
#endif

#ifndef MAX_THREADS
#define MAX_THREADS 8
#endif

#define UNUSED 0
#define START 1
#define WORKING 2
#define DONE 3
#define LOG 4

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

void pull(Antibody ** pop = NULL, const int32_t pipefd = -1);

