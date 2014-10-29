/* Ernest Richards
 * snc.hpp
 * cpp implementation of snc structure in snc.h
 *
 * TO DO:
 *
 * turn buf into char * with resize checks
 *
 */



#ifndef _SNC_HPP_
#define _SNC_HPP_

#pragma once

#include <globals.h>
#include <time.h>
#include <queue.h>

#ifndef _buf
#define _buf
char buf[1024];
#endif

#ifndef _keys
#define _keys
uint32_t shmkey[] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[] = {959, 653, 987, 627, 905};
#endif

#define NO_MEM 7
#define I_SHM 8
#define A_SHM 6
#define LOG 9

using namespace std;

typedef struct
{
        int32_t * shmid;
        int32_t * t5shmid;

        volatile sig_atomic_t ** shm;
        volatile sig_atomic_t ** t5shm;
} smem_t, *psmem_t;

typedef struct
{
        volatile sig_atomic_t ** sigs;
        volatile sig_atomic_t ** t5s;
} mem_t, *pmem_t;

typedef struct
{
        sig_atomic_t * sigs;
        sig_atomic_t * t5s;
} data_t, *pdata_t;

/* storage and control */
class snc
{
        private:
                uint32_t i;

                /* if using an expanding buffer to *permanently store signatures and tuples, this is the last signature/tuple pair returned */
                sig_atomic_t top;

                /* local buffer size */
                /* default size is 50, probably more than enough
                 * need to test to determine actual need */
                uint32_t SIGBUF;

                /* sig_atomic_t = 2.1 million */
                volatile sig_atomic_t ct1;
                volatile sig_atomic_t ct2;

                /* current position within SIGQTY buffer ring */
                sig_atomic_t pos;

                smem_t smem;
                /* instead of creating and manging a mem type by myself, let smarter people take care of that crap
                 * leaving 'mem' in place for now in order to test use of queue and allow existing memory
                 * allocation/deallocation functions to continue working  */
                mem_t mem;
                queue<data_t> data;
                /* shmids for transfer between applications */
                /*
                   int32_t * shmid;
                   int32_t * t5shmid;
                   */

                /* shared memory segments */
                /*
                   volatile sig_atomic_t ** shm;
                   volatile sig_atomic_t ** t5shm;
                   */

                /* actual retrieved sigs and identifying tuples */
                /*
                   volatile sig_atomic_t ** sigs;
                   volatile sig_atomic_t ** t5s;
                   */

                /* logfile file descriptor */
                int32_t logfd;

                /* routines for logging */
                int32_t openLog(const char * n = NULL, int32_t * fd = NULL);
                int32_t closeLog(int32_t * fd = NULL);
                uint8_t Log(const char *);

                /* memory and shm attach */
                inline void fData(void)
                {
                        i = 0;
                        if (mem.sigs && mem.t5s)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)mem.sigs[i]);
                                        free((sig_atomic_t *)mem.t5s[i]);
                                        i++;
                                }
                                free(mem.t5s);
                                free(mem.sigs);
                        }
                        else if (mem.sigs)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)mem.sigs[i++]);
                                }
                                free(mem.sigs);
                        }
                        else if (mem.t5s)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)mem.t5s[i++]);
                                }
                                free(mem.t5s);
                        }
                }

                inline void dShmids(void)
                {
                        i = 0;
                        while (i < (SIGQTY + 1))
                        {
                                if (i < SIGQTY)
                                {
                                        shmdt((void *)smem.t5shm[i]);
                                }
                                shmdt((void *)smem.shm[i]);
                                i++;
                        }
                }

                inline void iData(void)
                {
                        mem.t5s = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGBUF);
                        mem.sigs = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGBUF));
                        if (mem.t5s == NULL || mem.sigs == NULL)
                        {
                                Log("Unable to allocate sufficient memory\n");
                                _exit(NO_MEM);
                        }
                        i = 0;
                        while (i < SIGBUF)
                        {
                                mem.t5s[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                                mem.sigs[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                                if (mem.t5s[i] == NULL || mem.sigs[i] == NULL)
                                {
                                        Log("Unable to allocate sufficient memory\n");
                                        _exit(NO_MEM);
                                }
                                i++;
                        }
                }

                inline void fShms(void)
                {
                        if (smem.t5shm)
                        {
                                free(smem.t5shm);
                        }
                        if (smem.shm)
                        {
                                free(smem.shm);
                        }
                }

                inline void fShmids(void)
                {
                        if (smem.t5shmid)
                        {
                                free(smem.t5shmid);
                        }
                        if (smem.shmid)
                        {
                                free(smem.shmid);
                        }
                }

                inline void iShms(void)
                {
                        smem.shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
                        smem.t5shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY));
                        if (smem.shm == NULL || smem.t5shm == NULL)
                        {
                                Log("Unable to allocate sufficient memory\n");
                                _exit(NO_MEM);
                        }
                }

                inline void iShmids(void)
                {
                        i = 0;
                        srand(time(NULL));
                        smem.shmid = (int *)malloc(sizeof(int) * (SIGQTY + 1));
                        smem.t5shmid = (int *)malloc(sizeof(int) * (SIGQTY));
                        if (smem.shmid == NULL || smem.t5shmid == NULL)
                        {
                                Log("Unable to allocate sufficient memory\n");
                                _exit(NO_MEM);
                        }
                        while (i < (SIGQTY + 1))
                        {
                                smem.shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, 0666);
                                if (smem.shmid[i] < 0)
                                {
                                        Log("unable to get shm\n");
                                        _exit(I_SHM);
                                }
                                if (i < SIGQTY)
                                {
                                        smem.t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, 0666);
                                        if (smem.t5shmid[i] < 0)
                                        {
                                                Log("unable to get t5shm\n");
                                                _exit(I_SHM);
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
                                smem.shm[i] = (volatile sig_atomic_t *)shmat(smem.shmid[i], (void *) 0, 0);
                                if ((void *)smem.shm[i] == (void *)-1)
                                {
                                        Log("unable to attach shm\n");
                                        _exit(A_SHM);
                                }
                                if (i < SIGQTY)
                                {
                                        smem.t5shm[i] = (volatile sig_atomic_t *)shmat(smem.t5shmid[i], (void *) 0, 0);
                                        if ((void *)smem.t5shm[i] == (void *)-1)
                                        {
                                                Log("unable to attach shm\n");
                                                _exit(A_SHM);
                                        }
                                }
                                i++;
                        }
                }

                /* dummy manger for memory allocation */
                inline void Alloc(void)
                {
                        this->iData();
                        this->iShms();
                        this->iShmids();
                        this->aShmids();
                }

                /* dummy manger for memory deallocation */
                inline void Free(void)
                {
                        this->dShmids();
                        this->fShmids();
                        this->fShms();
                        this->fData();
                }

        public:
                /* constructors */
                snc(void)
                {
                        logfd = -1;
                        logfd = openLog();
                        SIGBUF = 50;
                        Alloc();
                }
                snc(const snc &);

                ~snc(void)
                {
                }

                /* accessors */

                sig_atomic_t getFlag(void) const
                {
                        if (smem.shm)
                        {
                                return (smem.shm[CTL][FLAGS]);
                        }
                        return (-1);
                }

                inline sig_atomic_t * getT5(const int & i)
                {
                        if (i < 0 || (unsigned int)i > SIGBUF)
                        {
                                return (NULL);
                        }
                        return ((sig_atomic_t *)(mem.t5s[i]));
                }

                inline sig_atomic_t * getSig(const int & i)
                {
                        if (i < 0 || (unsigned int)i > SIGBUF)
                        {
                                return (NULL);
                        }
                        return ((sig_atomic_t *)(mem.sigs[i]));
                }

                inline pdata_t Dequeue(void)
                {
                        pdata_t t = (pdata_t)malloc(sizeof(data_t));
                        if (!t)
                        {
                                return (0);
                        }
                        t = &(data.top());
                        data.pop();
                        return (t);
                }

                inline pdata_t getPair(const int & i)
                {
                        pdata_t t = (pdata_t)malloc(sizeof(data_t));
                        if (!t)
                        {
                                return (0);
                        }
                        memcpy((void *)t->sigs, (const void *)mem.sigs[i], sizeof(sig_atomic_t));
                        memcpy((void *)t->t5s, (const void *)mem.t5s[i], sizeof(sig_atomic_t));
                        return(t);
                }

                inline int32_t getLogfd(void)
                {
                        return(logfd);
                }

                /* this is going to give some unexpected results */
#if __WORDSIZE == 64
                inline uint64_t getCt(void)
                {
                        return((uint64_t)(ct1 + (ct2*2147483647)));
                }
#else
                inline uint32_t getCt(void)
                {
                        return((uint32_t)(ct1 + (ct2*2147483647)));
                }
#endif

                inline int32_t getT5shmid(const int & i)
                {
                        return(smem.t5shmid[i]);
                }

                inline int32_t getShmid(const int & i)
                {
                        return(smem.shmid[i]);
                }

                inline int32_t * getT5shmids(void)
                {
                        return(smem.t5shmid);
                }

                inline int32_t * getShmids(void)
                {
                        return(smem.shmid);
                }

                inline uint32_t getSize(void)
                {
                        return(SIGBUF);
                }

                inline sig_atomic_t getPos(void)
                {
                        return(pos);
                }

                /* mutators */
                inline static void Enqueue(const data_t t)
                {
                        data.enqueue(t);
                }

                uint8_t setFlag(const void * val)
                {
                        if (!smem.shm || !val)
                        {
                                return (-1);
                        }
                        smem.shm[CTL][FLAGS] = (sig_atomic_t)*((sig_atomic_t *)(val));
                        return (0);
                }

                inline void incTop(void)
                {
                        top++;
                        top = top % SIGBUF;
                }
                inline void incPos(void)
                {
                        pos++;
                        pos = (((pos) % SIGQTY) != 0 ? ((pos) % SIGQTY) : SIGQTY); // 1 - 5
                }
                inline void incCt(void)
                {
                        ct1++;
                        /* check for and handle overflow SORT OF */
                        if (ct1 < 0)
                        {
                                ct1 = 0;
                                ct2++;
                                if (ct2 < 0)
                                {
                                        Log("Count of retrieved headers = 2.1 million squared, continuing\n");
                                        ct2 = 0;
                                }
                        }
                }

                /* operators */
                inline static void Copy(pdata_t src, pdata_t dst)
                {
                        memcpy(src->sigs, dst->sigs, sizeof(sig_atomic_t));
                        memcpy(src->t5s, dst->t5s, sizeof(sig_atomic_t));
                }
                inline static void Copy(data_t src, data_t dst)
                {
                        memcpy(src.sigs, dst.sigs, sizeof(sig_atomic_t));
                        memcpy(src.t5s, dst.t5s, sizeof(sig_atomic_t));
                }

                /* others */
#ifndef _itoa_h_
#define _itoa_H_
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
#endif

};

#endif

/* main file */
/*
   uint8_t Log(const char * c)
   {
   if (!c)
   {
   return (-1);
   }
   if (logfd != -1)
   {
   while (*c != '\0')
   {
   if ((putc(*c, logfd)) < 0)
   {
   putc("L", stdout);
   _exit(LOG);
   }
   }
   return (0);
   }
   else
   {
   while (*c != '\0')
   {
   if ((putc(*c, stdout)) < 0)
   {
   putc("L", stdout);
   _exit(LOG);
   }
   }
   return (LOG);
   }
   }

   uint32_t openLog(const char * n, uint32_t * fd)
   {
   if (!fd)
   {
   if ((fd = (uint32_t *)malloc(sizeof(uint32_t) * 1)) < 0)
   {
   Log("Insufficient memory\n\0");
   _exit(NO_MEM);
   }
   }
   */
/* HERE */
/*
   char * path = getcwd();
   if (path)
   {
   if (n)
   {
   strncat(path, n, strlen(n));
   }
   else
   {
   strncat(path, (const char *)"dhs_retrieve.log", 16);
   }
   }
   else
   {
   strncpy(path, (const char *)"/tmp/dhs_retrieve.log", 21);
   }
 *fd = open ((const char *)path , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , 0644);
 if (*fd < 0)
 {
 Log("Unable to open log file\n");
 _exit(LOG);
 }
 return (*fd);
 }

 int32_t closeLog(int32_t * fd)
 {
 if (!fd || *fd < 0)
 {
 return(0);
 }
 if (close(*fd) < 0)
 {
 Log("Unable to close logfile");
 return (-1);
 }
 return (0);
 }

*/
