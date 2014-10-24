/* share.h 
 *
 * Ernest Richards
 * 
 */

#ifndef _SHARE_H_
#define _SHARE_H_

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <sys/shm.h>

#include <itoa.h>
#include <globals.h>

/* storage and control */
typedef struct
{
        int32_t * shmid;
        int32_t * t5shmid;

        volatile sig_atomic_t ** shm;
        volatile sig_atomic_t ** t5shm;

        sig_atomic_t ** sigs;
        sig_atomic_t ** t5s;
} snc_t, *psnc_t;

/* 0, 1 - 5 */
/*
extern volatile sig_atomic_t ** shm;
extern int ** sigs;
extern char ** t5s;
extern char ** t5shm;
extern int * shmid;
extern int * t5shmid;
*/
extern unsigned char * hdr_data;
extern uint32_t hdr_size;
extern uint32_t shmkey[];
extern uint32_t t5shmkey[];

char buf[1024];

inline void dShmids(psnc_t);
inline void fData(psnc_t);
inline void fShms(psnc_t);
inline void fShmids(psnc_t);
void freeMem (psnc_t);
inline void iData(psnc_t);
inline void iShms(psnc_t);
inline void iShmids(psnc_t);
inline void aShmids(psnc_t);
void initMem(psnc_t);

#endif

/* share.h */
