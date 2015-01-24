#ifndef _SNC_H_
#define _SNC_H_

#pragma once

#include <globals.h>

typedef struct
{
        int32_t * shmid;
        int32_t * t5shmid;

        volatile sig_atomic_t ** shm;
        volatile sig_atomic_t ** t5shm;
} smem_t, *psmem_t;

typedef struct
{
        sig_atomic_t ** sigs;
        sig_atomic_t ** t5s;
} mem_t, *pmem_t;

/* storage and control */
typedef struct
{
        smem_t smem;
        mem_t mem;

} snc_t, *psnc_t;

#endif
