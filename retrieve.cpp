#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <globals.h>

using namespace std;

#ifndef DEBUG
#define DEBUG 1
#endif

char buf[1024];

#define SIGQTY 5

#ifndef _bufSize
#define _bufSize
static const uint32_t bufSize = 5;
#endif

#ifndef fngPntLen
#define fngPntLen 14
#endif

#ifndef _t5TplLen
#define _t5TplLen
static const uint32_t t5TplLen = 44;
#endif

uint32_t shmkey[6] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[5] = {959, 653, 987, 627, 905};

int * shmid = NULL;
int * t5shmid = NULL;
int ** shm = NULL;
char ** t5shm = NULL;

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
	 do
	 { //Move to where representation ends
		  ++p;
		  shifter = shifter/10;
	 }
	 while (shifter);
	 *p = '\0';
	 do
	 { //Move back, inserting digits as u go
		  *--p = digit[i%10];
		  i = i/10;
	 }
	 while (i);
	 return p;
}

inline static void inCtr(int ** s)
{
        (*s[0])++;
        (*s[0]) = (((*s[0]) % SIGQTY) > 0 ? ((*s[0]) % SIGQTY) : (*s[0]) % SIGQTY + 1); // 1 - 5
}

unsigned int i = 0;

inline void dShmids(void)
{
        i = 0;
        while (i < (bufSize + 1))
        {
                shmdt(t5shm[i]);
                i++;
        }
        i = 0;
        while (i < (bufSize + 1))
        {
                shmdt(shm[i]);
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
        shm = (int **)malloc(sizeof(int *) * (bufSize + 1));
        t5shm = (char **)malloc(sizeof(char *) * (bufSize + 1));
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
        shmid = (int *)malloc(sizeof(int) * (bufSize + 1));
        t5shmid = (int *)malloc(sizeof(int) * (bufSize + 1));
        if (shmid == NULL || t5shmid == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
        while (i < (bufSize + 1))
        {
                shmid[i] = shmget(shmkey[i], sizeof(int) * fngPntLen, 0666);
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
                i++;
        }
}

inline void aShmids(void)
{
        i = 0;
        unsigned int j = 0;
        while (i < (bufSize + 1))
        {
                shm[i] = (int *)shmat(shmid[i], (void *) 0, 0);
                t5shm[i] = (char *)shmat(t5shmid[i], (void *) 0, 0);
                if ((void *)shm[i] == (void *)-1)
                {
                        strncpy(buf, "unable to attach shm", 14);
                        strncat(buf, "\r\n", 2);
                        write(2, buf, 16);
                        _exit(-1);
                }
                if ((void *)t5shm[i] == (void *)-1)
                {
                        strncpy(buf, "unable to attach t5shm", 16);
                        strncat(buf, "\r\n", 2);
                        write(2, buf, 18);
                        _exit(-1);
                }
                j = 0;
                while (j < fngPntLen)
                {
                        shm[i][j++] = 0;
                }
                j = 0;
                while (j < t5TplLen)
                {
                        shm[i][j++] = '0';
                }
                i++;
        }

}

void shandler ( int sign )
{
        dShmids();

        fShmids();
        fShms();

        fprintf( stderr, "\n\n[+] Retrieve finished!\n" );
        exit( sign );
}

int main (void)
{

        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        if (DEBUG)
        {
                fprintf(stderr, "retrieve\n\tcalling ishms()\r\n");
        }
        iShms();
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

        // Retrieve

        if (DEBUG)
        {
                fprintf(stderr, "\tabout to enter\r\n");
        }
        while (1)
        {
                while (shm[0][FLAGS] != PWTEN)
                {
                        fprintf(stderr, "waiting for flag to be PWTEN\r\n");
                        fflush(stderr);
                }
                shm[0][FLAGS] = CRING;
                write(2, "data written, retrieving...\r\n", 26);
                write(2, "SIMULATED retrieved, releasing\r\n", 22);
                shm[0][FLAGS] = CREAD;
        }

        shandler (0);
}
