#include <iostream>
#include <queue>

#include <retglobals.h>
#include <mem.hpp>
#include <pull.hpp>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

uint32_t shmkey[] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t urishmkey[] = {123, 33, 4663, 9043, 4478};
uint32_t t5shmkey[] = {959, 653, 987, 627, 905};

int * shmid = NULL;
int * t5shmid = NULL;
int * urishmid = NULL;
volatile sig_atomic_t ** shm = NULL;
volatile sig_atomic_t ** t5shm = NULL;
char ** urishm = NULL;

volatile sig_atomic_t ** retrieved_sigs = NULL;
char ** retrieved_uris = NULL;
volatile sig_atomic_t ** retrieved_t5s = NULL;

sig_atomic_t ct = 0;
sig_atomic_t local_pos = 1;

/**
 * @brief signal handler. used for exit.
 */
void shandler ( int sign )
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );
        signal( SIGABRT, &shandler );

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

/**
 * @brief Sets up signal handler and calls initialization functions
 */
inline static void setup (void)
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );
        signal( SIGABRT, &shandler );
        iShms();
        iData();
        iShmids();
        aShmids();
}

/**
 * @brief Main function. starts pull.
 */
int main (void)
{
        setup();

        pull();

        shandler (0);
}
