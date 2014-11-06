#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "tools.hpp"

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

char buf[102];

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
                        shmdt((void *)t5shm[i]);
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
                retrieved_t5s[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                retrieved_sigs[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
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
        t5shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY));
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
                                strncpy(buf, "unable to get shm\n", 18);
                                write(2, buf, 22);
                        }
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, 0666);
                        if (t5shmid[i] < 0)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to get t5shm\n", 20);
                                        write(2, buf, 20);
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
