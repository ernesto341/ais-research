#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <globals.h>
#include <signal.h>

#include "mem.hpp"

using namespace std;

#define DEBUG 1

#ifndef SIGBUF
#define SIGBUF 50
#endif

extern sig_atomic_t local_pos;
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
