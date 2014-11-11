#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <retglobals.h>
#include <signal.h>

#include <mem.hpp>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

extern sig_atomic_t local_pos;
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


inline static void inPos(void)
{
        ((local_pos))++;
        ((local_pos)) = ((((local_pos)) % SIGQTY) != 0 ? (((local_pos)) % SIGQTY) : SIGQTY); // 1 - 5
}
