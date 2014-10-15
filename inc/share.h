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

#ifndef SIGQTY
#define SIGQTY 5
#endif

/* 0, 1 - 5 */
extern int ** shm;
extern int ** sigs;
extern char ** t5s;
extern char ** t5shm;
extern int * shmid;
extern int * t5shmid;
extern unsigned char * hdr_data;
extern uint32_t hdr_size;
extern const uint32_t t5_size;
extern const uint32_t shm_size;
extern uint32_t shmkey[];
extern uint32_t t5shmkey[];

char buf[1024];

inline void dShmids(void);
inline void fData(void);
inline void fShms(void);
inline void fShmids(void);
void freeMem (void);
inline void iData(void);
inline void iShms(void);
inline void iShmids(void);
inline void aShmids(void);
void initMem(void);

#endif

/* share.h */
