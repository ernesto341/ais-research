#include <iostream>

#include <globals.h>

using namespace std;

#define DEBUG 1

#define SIGBUF 50

char buf[102];

uint32_t shmkey[] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[] = {959, 653, 987, 627, 905};

int * shmid = NULL;
int * t5shmid = NULL;
volatile sig_atomic_t ** shm = NULL;
char ** t5shm = NULL;

volatile sig_atomic_t ** retrieved_sigs = NULL;
volatile sig_atomic_t ** retrieved_t5s = NULL;

sig_atomic_t ct = 0;
sig_atomic_t local_pos = 1;

inline static void inPos(void)
{
        ((local_pos))++;
        ((local_pos)) = ((((local_pos)) % SIGQTY) != 0 ? (((local_pos)) % SIGQTY) : SIGQTY); // 1 - 5
}

char * itoa (int i)
{
        char const digit[] = "0123456789";
        char * p = 0;
        p = (char *)malloc(sizeof(char) * 11);
        if (!p)
        {
                return (0);
        }
        if (i < 0)
        {
                *p++ = '-';
                i = -1;
        }
        int shifter = i;
        //Move to where representation ends
        do
        {
                ++p;
                shifter = shifter/10;
        }
        while (shifter);
        *p = '\0';
        //Move back, inserting digits as u go
        do
        {
                *--p = digit[i%10];
                i = i/10;
        }
        while (i);
        return (p);
}

unsigned int i = 0;

inline void fData(void)
{
        i = 0;
        if (retrieved_sigs && retrieved_t5s)
        {
                while (i < SIGBUF)
                {
                        free((sig_atomic_t *)retrieved_sigs[i]);
                        free((sig_atomic_t *)retrieved_t5s[i]);
                        i++;
                }
                free(retrieved_t5s);
                free(retrieved_sigs);
        }
        else if (retrieved_sigs)
        {
                while (i < SIGBUF)
                {
                        free((sig_atomic_t *)retrieved_sigs[i++]);
                }
                free(retrieved_sigs);
        }
        else if (retrieved_t5s)
        {
                while (i < SIGBUF)
                {
                        free((sig_atomic_t *)retrieved_t5s[i++]);
                }
                free(retrieved_t5s);
        }
}

inline void dShmids(void)
{
        i = 0;
        while (i < (SIGQTY + 1))
        {
                if (i < SIGQTY)
                {
                        shmdt(t5shm[i]);
                }
                shmdt((void *)shm[i]);
                i++;
        }
}

inline void iData(void)
{
        retrieved_t5s = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGBUF);
        retrieved_sigs = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGBUF));
        if (retrieved_t5s == NULL || retrieved_sigs == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        i = 0;
                        while (i < 38)
                        {
                                putc(buf[i], stderr);
                        }
                }
                _exit(-1);
        }
        i = 0;
        while (i < SIGBUF)
        {
                retrieved_t5s[i] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                retrieved_sigs[i] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                if (retrieved_t5s[i] == NULL || retrieved_sigs[i] == NULL)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                                i = 0;
                                while (i < 38)
                                {
                                        putc(buf[i], stderr);
                                }
                        }
                        _exit(-1);
                }
                i++;
        }
}

inline void fShms(void)
{
        if (t5shm)
        {
                free(t5shm);
        }
        if (shm)
        {
                free(shm);
        }
}

inline void fShmids(void)
{
        if (t5shmid)
        {
                free(t5shmid);
        }
        if (shmid)
        {
                free(shmid);
        }
}

inline void iShms(void)
{
        shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        t5shm = (char **)malloc(sizeof(char *) * (SIGQTY));
        if (shm == NULL || t5shm == NULL)
        {
                strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                write(2, buf, 38);
                _exit(-1);
        }
}

inline void iShmids(void)
{
        i = 0;
        srand(time(NULL));
        shmid = (int *)malloc(sizeof(int) * (SIGQTY + 1));
        t5shmid = (int *)malloc(sizeof(int) * (SIGQTY));
        if (shmid == NULL || t5shmid == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
        while (i < (SIGQTY + 1))
        {
                shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, 0666);
                if (shmid[i] < 0)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to get shm with key ", 27);
                                strncat(buf, itoa(shmkey[i]), strlen(itoa(shmkey[i])));
                                strncat(buf, "\r\n", 2);
                                write(2, buf, strlen(buf));
                        }
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        t5shmid[i] = shmget(t5shmkey[i], sizeof(char) * t5TplLen, 0666);
                        if (t5shmid[i] < 0)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to get t5shm with key ", 29);
                                        strncat(buf, itoa(t5shmkey[i]), strlen(itoa(t5shmkey[i])));
                                        strncat(buf, "\r\n", 2);
                                        write(2, buf, strlen(buf));
                                }
                                _exit(-1);
                        }
                }
                i++;
        }
}

inline void aShmids(void)
{
        i = 0;
        while (i < (SIGQTY + 1))
        {
                shm[i] = (int *)shmat(shmid[i], (void *) 0, 0);
                if ((void *)shm[i] == (void *)-1)
                {
                        strncpy(buf, "unable to attach shm", 14);
                        strncat(buf, "\r\n", 2);
                        write(2, buf, 16);
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        t5shm[i] = (char *)shmat(t5shmid[i], (void *) 0, 0);
                        if ((void *)t5shm[i] == (void *)-1)
                        {
                                strncpy(buf, "unable to attach t5shm", 16);
                                strncat(buf, "\r\n", 2);
                                write(2, buf, 18);
                                _exit(-1);
                        }
                }
                i++;
        }

}

void shandler ( int sign )
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );

        if (DEBUG)
        {
                if (shm[CTL][FLAGS] == PDONE)
                {
                        fprintf(stderr, "\r\n\t\t[i] --- Signaled to quit by producer\r\n");
                        fprintf(stderr, "\r\n\t\t[i] --- Sign passed to retrieve shandler is %d\r\n", sign);
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
                                //memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS])], sizeof(sig_atomic_t) * t5TplLen);
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
                                        /*
                                        fprintf(stderr, "\t");
                                        i = 0;
                                        while (i < t5TplLen)
                                        {
                                                fprintf(stderr, "%c", retrieved_t5s[ct][i]);
                                                i++;
                                        }
                                        */
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
