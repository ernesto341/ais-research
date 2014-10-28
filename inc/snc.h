#ifndef _SNC_H_
#define _SNC_H_

#pragma once

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

#endif
