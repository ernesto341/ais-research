#include <pull.hpp>

using namespace std;

/*

   Antibody ** champs;

*/
typedef struct _test_param
{
        //unknownweb * uw_obj; // pass one of the precreated/initialized objects by reference
        uint32_t sig[fngPntLen]; // signature to be tested
        char t5tuple[t5TplLen]; // t5 tuple
        volatile sig_atomic_t go; // signal to thread to start testing
} test_param, *ptest_param;

typedef struct _test_result
{
        uint32_t sig[fngPntLen]; // tested signature
        int8_t attack; // whether or not the tested signature was determined to be an attack
        char **resultv; // all results will be stored locally in each uw_obj, but this struct is a set of *new results that can be logged if necessary
        uint32_t resultc; // count of results
        char t5tuple[t5TplLen]; // t5 tuple
} test_result, *ptest_result;

/*
uint32_t count(const char ** r)
{
        uint32_t ct = 0;
        while (*r != "\0")
        {
                (&r)++;
                ct++;
        }
        return (ct);
}
*/

void * testThread(void * v)
{
        test_param p = *(ptest_param)v;
        //p.uw_obj->test(&champs, p.sig);
        ptest_result r = new test_result;
        memcpy(r->sig, p.sig, sizeof(uint32_t) * fngPntLen);
        memcpy(r->t5tuple, p.t5tuple, sizeof(char) * t5TplLen);
        //r->attack = (uint8_t)p.uw_obj.queryTests(); // HERE - use this? to determine if this was an attack
        //r->resultv = ;
        //r->resultc = count(resultv);

        if(r->attack - LOGLEVEL >= 0)
        {
                // HERE - need to enforce mutual exclusion?
                //log_queue.enqueue(*r);
        }

        return ((void *)r);
}


queue<test_result> log_queue;
uint32_t logfd;

/* HERE */
/* child function that watches the queue for entries that need to be logged and logs new entries */
void Stats (void)
{
        /* open log file */
        /* start while shm[flag] != done loop */
        /* start while item in queue loop */
        /* log new item to queue */
        /* dump overall statistics to log file */
        /* close log file */
}

inline static void Convert (char dst[t5TplLen], sig_atomic_t src[t5TplLen])
{
        unsigned int i = 0;
        while (i < t5TplLen)
        {
                dst[i] = (char)src[i];
                i++;
        }
}

#ifndef THREAD_MAX
#define THREAD_MAX 8
#endif

volatile sig_atomic_t testing;

void pull()
{
        unsigned int i = 0;
        /* HERE */
        /* fork log process */
        /*
           test_param params[THREAD_MAX];
           unknownweb uw_objs[THREAD_MAX];
           for (i = 0; i < THREAD_MAX; i++)
           {
           params[i].uw_obj = &(uw_objs[i]);
           params[i].go = 0;
           pthread_create();
           }
           */
        testing = 0;
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
                                /*
                                   memcpy((void *)&(params[testing].sig), (void *)retrieved_sigs[ct], sizeof(sig_atomic_t) * fngPntLen);
                                   */
                                /* convert sig_atomic_t to char */
                                /*
                                   Convert(params[testing].tuple, retrieved_t5s[ct]);
                                   params[testing].go = 1;
                                   */
                                if (testing < THREAD_MAX)
                                {
                                        testing++;
                                }
                                else
                                {
                                        fprintf(stderr, "Insufficient threads to process incoming signatures\n");
                                        fflush(stderr);
                                }
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
        /* HERE */
        /* signal dhs that retrieve is done */
        /* join threads */
        /* join logging process */
        return;
}
