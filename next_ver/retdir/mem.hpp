#pragma once
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

static const uint32_t MAXURI = 2048;

extern uint32_t shmkey[];
extern uint32_t t5shmkey[];
extern uint32_t urishmkey[];

extern int * shmid;
extern int * t5shmid;
extern int * urishmid;
extern volatile sig_atomic_t ** shm;
extern char ** urishm;
extern volatile sig_atomic_t ** t5shm;

extern volatile sig_atomic_t ** retrieved_sigs;
extern volatile sig_atomic_t ** retrieved_t5s;
/* HERE */
extern char ** retrieved_uris;

extern sig_atomic_t ct;
extern sig_atomic_t local_pos;

extern char buf[];
extern unsigned int i;

/**
 * @brief Frees allocated memory used to store local data for retrieve operation.
 */
inline void fData(void)
{
        unsigned int i = 0;
        if (retrieved_sigs && retrieved_t5s && retrieved_uris)
        {
                while (i < SIGBUF)
                {
                        free((sig_atomic_t *)retrieved_sigs[i]);
                        free((sig_atomic_t *)retrieved_t5s[i]);
                        /* HERE */
                        i++;
                }
                free(retrieved_t5s);
                free(retrieved_sigs);
                //free(retrieved_uris);
        }
        else if (retrieved_sigs)
        {
                while (i < SIGBUF)
                {
                        free((sig_atomic_t *)retrieved_sigs[i++]);
                }
                free(retrieved_sigs);
        }
                        /* HERE */
        else if (retrieved_uris)
        {
                //free(retrieved_uris);
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

/**
 * @brief Detaches shared memory segments.
 */
inline void dShmids(void)
{
        unsigned int i = 0;
        /* HERE - invalid next ptr. why?*/
        //while (i < (SIGQTY + 1))
        while (i < (SIGQTY))
        {
                if (i < SIGQTY)
                {
                        shmdt((void *)t5shm[i]);
                        shmdt((void *)urishm[i]);
                }
                shmdt((void *)shm[i]);
                i++;
        }
}

/**
 * @brief Allocates memory for local data storage.
 */
inline void iData(void)
{
        unsigned int i = 0;
        char buf [102];
        retrieved_t5s = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGBUF);
        retrieved_sigs = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGBUF);
        retrieved_uris = (char **)malloc(sizeof(char *) * SIGBUF);
        if (retrieved_t5s == NULL || retrieved_sigs == NULL || retrieved_uris == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                        i = 0;
                        while (i < 37)
                        {
                                putc(buf[i], stderr);
                        }
                }
                _exit(-1);
        }
        i = 0;
        while (i < SIGBUF)
        {
                /* HERE */
                retrieved_t5s[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                retrieved_uris[i] = (char *)malloc(sizeof(char) * MAXURI);
                retrieved_sigs[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                if (retrieved_t5s[i] == NULL || retrieved_sigs[i] == NULL || retrieved_uris[i] == NULL)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                                i = 0;
                                while (i < 37)
                                {
                                        putc(buf[i], stderr);
                                }
                        }
                        _exit(-1);
                }
                i++;
        }
}

/**
 * @brief Frees shared memory.
 */
inline void fShms(void)
{
        if (t5shm)
        {
                free(t5shm);
        }
        if (urishm)
        {
                //free(urishm);
        }
        if (shm)
        {
                free(shm);
        }
}

/**
 * @brief Frees memory allocated to ids for shared memory.
 */
inline void fShmids(void)
{
        if (t5shmid)
        {
                free(t5shmid);
        }
        if (urishmid)
        {
                //free(urishmid);
        }
        if (shmid)
        {
                free(shmid);
        }
}

/**
 * @brief Allocates memory for shared memory segment.
 */
inline void iShms(void)
{
        char buf [102];
        shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        t5shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY));
        urishm = (char **)malloc(sizeof(char *) * (SIGQTY));
        if (shm == NULL || t5shm == NULL || urishm == NULL)
        {
                strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                write(2, buf, 37);
                _exit(-1);
        }
}

/**
 * @brief Allocates memory for and assigns shared memory IDs for existing shared memory segments.
 */
inline void iShmids(void)
{
        unsigned int i = 0;
        char buf [102];
        srand(time(NULL));
        shmid = (int *)malloc(sizeof(int) * (SIGQTY + 1));
        t5shmid = (int *)malloc(sizeof(int) * (SIGQTY));
        urishmid = (int *)malloc(sizeof(int) * (SIGQTY));
        if (shmid == NULL || t5shmid == NULL || urishmid == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                        write(2, buf, 37);
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
                                strncpy(buf, "unable to get shm\n", 18);
                                write(2, buf, 22);
                        }
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, 0666);
                        /* HERE */
                        urishmid[i] = shmget(urishmkey[i], sizeof(char) * MAXURI, 0666);
                        if (t5shmid[i] < 0 || urishmid[i] < 0)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to get t5 or uri shm\n", 28);
                                        write(2, buf, 28);
                                }
                                _exit(-1);
                        }
                }
                i++;
        }
}

/**
 * @brief Attaches to existing shared memory.
 */
inline void aShmids(void)
{
        unsigned int i = 0;
        char buf [102];
        while (i < (SIGQTY + 1))
        {
                shm[i] = (volatile sig_atomic_t *)shmat(shmid[i], (void *) 0, 0);
                if ((void *)shm[i] == (void *)-1)
                {
                        strncpy(buf, "unable to attach shm", 14);
                        strncat(buf, "\r\n", 2);
                        write(2, buf, 16);
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        t5shm[i] = (volatile sig_atomic_t *)shmat(t5shmid[i], (void *) 0, 0);
                        urishm[i] = (char *)shmat(urishmid[i], (void *) 0, 0);
                        if ((void *)t5shm[i] == (void *)-1 || (void *)urishm[i] == (void *)-1)
                        {
                                strncpy(buf, "unable to attach t5 or uri shm\n", 25);
                                write(2, buf, 25);
                                _exit(-1);
                        }
                }
                i++;
        }

}
