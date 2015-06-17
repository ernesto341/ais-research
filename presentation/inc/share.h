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

#include <snc.h>
#include <itoa.h>
#include <globals.h>

extern uint32_t shmkey[];
extern uint32_t urishmkey[];
extern uint32_t t5shmkey[];

extern char * uri_str;

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
