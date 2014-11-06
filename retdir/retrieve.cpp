#include <iostream>
#include <queue>

#include <globals.h>

#include "tools.hpp"
#include "mem.hpp"

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

/*
unknownweb * uw_obj1;
unknownweb * uw_obj2;
unknownweb * uw_obj3;
unknownweb * uw_obj4;
unknownweb * uw_obj5;
unknownweb * uw_obj6;
unknownweb * uw_obj7;
unknownweb * uw_obj8;

Antibody ** champs;

class statistics
{
        protected:
        // same stuff as in test_result struct
                queue <test_result> result_queue;
#if __WORDSIZE == 64
                uint64_t attack_type_qty[12];
#else
                uint32_t attack_type_qty[12];
#endif
        public:
                operator += ();
                operator << ();
};

typedef struct _test_param
{
        unknownweb ** uw_obj; // pass one of the precreated/initialized objects by reference
        const uint32_t sig[fngPntLen]; // signature to be tested
        const char t5tuple[t5TplLen]; // t5 tuple
} test_param, *ptest_param;

typedef struct _test_result
{
// what goes here?
        const uint32_t sig[fngPntLen]; // tested signature
        const int8_t attack; // whether or not the tested signature was determined to be an attack
        char **resultv; // all results will be stored locally in each uw_obj, but this struct is a set of *new results that can be logged if necessary
        const uint32_t resultc; // count of results
        const char t5tuple[t5TplLen]; // t5 tuple
} test_result, *ptest_result;
*/

void shandler ( int sign )
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );

        if (DEBUG)
        {
                if (shm[CTL][FLAGS] == PDONE)
                {
                        fprintf(stderr, "\n\t\t[i] --- Signaled to quit by producer\n");
                        fprintf(stderr, "\n\t\t[i] --- Sign passed to retrieve shandler is %d\n", sign);
                        fflush(stderr);
                }
        }
        shm[CTL][FLAGS] = CDONE;

        dShmids();

        fShmids();
        fShms();
        fData();

        fprintf( stderr, "\n\n[+] Retrieve finished!\n" );
        exit( sign );
}

int main (void)
{

        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );
        if (DEBUG)
        {
                fprintf(stderr, "retrieve\n\tcalling ishms()\r\n");
        }
        iShms();
        if (DEBUG)
        {
                fprintf(stderr, "\tcalling iData()\r\n");
        }
        iData();
        if (DEBUG)
        {
                fprintf(stderr, "\tcalling ishmids()\r\n");
        }
        iShmids();
        if (DEBUG)
        {
                fprintf(stderr, "\tcalling ashmids()\r\n");
        }
        aShmids();

        if (DEBUG)
        {
                // Retrieve
                fprintf(stderr, "\tentering loop\r\n");
        }

        /* accept signal from producer to quit */
        while (shm[CTL][FLAGS] != PDONE)
        {
                /* only copy when dhs isn't writing and while there are pending headers to get 
                 */
                while (shm[CTL][PEND] > 0)
                {
                        if (shm[CTL][FLAGS] == PWTEN || shm[CTL][FLAGS] == CREAD)
                        {
                                if (DEBUG)
                                {
                                        // Retrieve
                                        fprintf(stderr, "\tshm[CTL][FLAGS] == PWTEN\r\n");
                                }
                                shm[CTL][FLAGS] = CRING;
                                memcpy((void *)retrieved_sigs[ct], (void *)shm[(shm[CTL][POS])], sizeof(sig_atomic_t) * fngPntLen);
                                memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS])-1], sizeof(sig_atomic_t) * t5TplLen);
                                if (DEBUG)
                                {
                                        fprintf(stderr, "pos = %d, pend = %d\r\n", shm[CTL][POS], shm[CTL][PEND]);
                                        fprintf(stderr, "\r\nsig:\r\n");
                                        i = 0;
                                        while (i < fngPntLen)
                                        {
                                                fprintf(stderr, "\t%d - %d", i, retrieved_sigs[ct][i]);
                                                fprintf(stderr, "\r\n");
                                                i++;
                                        }
                                        fprintf(stderr, "\t");
                                        i = 0;
                                        while (i < t5TplLen)
                                        {
                                                fprintf(stderr, "%c", retrieved_t5s[ct][i]);
                                                i++;
                                        }
                                        fprintf(stderr, "\r\n");
                                        fflush(stderr);
                                }
                                ct = (ct+1)%SIGBUF;
                                inPos();

                                /* decrement pending counter
                                 * should never go below 0,
                                 * so this test is probably unnecessary
                                 */
                                if (shm[CTL][PEND] > 0)
                                {
                                        shm[CTL][PEND]--;
                                }
                                shm[CTL][FLAGS] = CREAD;
                        }
                }
        }

        shandler (0);
}
