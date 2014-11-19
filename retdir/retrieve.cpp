#include <iostream>
#include <queue>

#include <retglobals.h>

#include <tools.hpp>
#include <mem.hpp>
#include <pull.hpp>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

uint32_t shmkey[] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[] = {959, 653, 987, 627, 905};

int * shmid = NULL;
int * t5shmid = NULL;
volatile sig_atomic_t ** shm = NULL;
volatile sig_atomic_t ** t5shm = NULL;

volatile sig_atomic_t ** retrieved_sigs = NULL;
volatile sig_atomic_t ** retrieved_t5s = NULL;

sig_atomic_t ct = 0;
sig_atomic_t local_pos = 1;

void shandler ( int sign )
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );

        if (shm)
        {
                shm[CTL][FLAGS] = CDONE;
        }

        dShmids();

        fShmids();
        fShms();
        fData();

        fprintf( stderr, "\n\n[+] Retrieve finished!\n" );
        exit( sign );
}

inline static void setup (void)
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );
        iShms();
        iData();
        iShmids();
        aShmids();
}

int main (void)
{
        setup();

        pull();

        shandler (0);
}
