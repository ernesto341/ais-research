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

#ifndef _buf
#define _buf
char buf[1024];
#endif

#define NO_MEM 7
#define I_SHM 8
#define A_SHM 6
#define LOG 9

using namespace std;

/* storage and control */
class snc
{
        private:
                unsigned int i = 0;

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

                /* shmids for transfer between applications */
                int32_t * shmid;
                int32_t * t5shmid;

                /* shared memory segments */
                volatile sig_atomic_t ** shm;
                volatile sig_atomic_t ** t5shm;

                /* actual retrieved sigs and identifying tuples */
                sig_atomic_t ** sigs;
                sig_atomic_t ** t5s;

                /* logfile file descriptor */
                int32_t logfd;

                /* signal handler - basically an exit function*/
                void shandler ( int sign )
                {
                        signal( SIGINT, &shandler );
                        signal( SIGTERM, &shandler );
                        signal( SIGSEGV, &shandler );

                        if (DEBUG)
                        {
                                if (snc.shm[CTL][FLAGS] == PDONE)
                                {
                                        Log("\n\t\t[i] --- Signaled to quit by producer\n");
                                }
                                strncpy(buf, "\n\t\t[i] --- Signal: ", 19);
                                tmp = itoa(sign);
                                strncat(buf, tmp, strlen(tmp));
                                strncat(buf, "\n", 1);
                                Log(buf);
                        }
                        snc.shm[CTL][FLAGS] = CDONE;

                        Free();

                        pcap_close( handle );

                        ntoh_exit();

                        Log("\n\t\tX      -----   Inactive   -----      X\n\n");
                        _exit( sign );
                }

                /* routines for logging */
                int32_t openLog(const char * n = NULL, int32_t * fd = NULL);
                int32_t closeLog(int32_t * fd = NULL);
                uint8_t Log(const char *);

                /* memory and shm attach */
                inline void fData(void)
                {
                        i = 0;
                        if (sigs && t5s)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)sigs[i]);
                                        free((sig_atomic_t *)t5s[i]);
                                        i++;
                                }
                                free(t5s);
                                free(sigs);
                        }
                        else if (sigs)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)sigs[i++]);
                                }
                                free(sigs);
                        }
                        else if (t5s)
                        {
                                while (i < SIGBUF)
                                {
                                        free((sig_atomic_t *)t5s[i++]);
                                }
                                free(t5s);
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
                        t5s = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGBUF);
                        sigs = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGBUF));
                        if (t5s == NULL || sigs == NULL)
                        {
                                Log("Unable to allocate sufficient memory\n");
                                shandler(NO_MEM);
                        }
                        i = 0;
                        while (i < SIGBUF)
                        {
                                t5s[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                                sigs[i] = (volatile sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                                if (t5s[i] == NULL || sigs[i] == NULL)
                                {
                                        Log("Unable to allocate sufficient memory\n");
                                        shandler(NO_MEM);
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
                                Log("Unable to allocate sufficient memory\n");
                                shandler(NO_MEM);
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
                                Log("Unable to allocate sufficient memory\n");
                                shandler(NO_MEM);
                        }
                        while (i < (SIGQTY + 1))
                        {
                                shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, 0666);
                                if (shmid[i] < 0)
                                {
                                        Log("unable to get shm\n");
                                        shandler(I_SHM);
                                }
                                if (i < SIGQTY)
                                {
                                        t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, 0666);
                                        if (t5shmid[i] < 0)
                                        {
                                                Log("unable to get t5shm\n");
                                                shandler(I_SHM);
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
                                        Log("unable to attach shm\n");
                                        shandler(A_SHM);
                                }
                                if (i < SIGQTY)
                                {
                                        t5shm[i] = (volatile sig_atomic_t *)shmat(t5shmid[i], (void *) 0, 0);
                                        if ((void *)t5shm[i] == (void *)-1)
                                        {
                                                Log("unable to attach shm\n");
                                                shandler(A_SHM);
                                        }
                                }
                                i++;
                        }
                }

                /* dummy manger for memory allocation */
                inline static void Alloc(void)
                {
                        iData();
                        iShms();
                        iShmids();
                        aShmids();
                }

                /* dummy manger for memory deallocation */
                inline static void Free(void)
                {
                        dShmids();
                        fShmids();
                        fShms();
                        fData();
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
                inline sig_atomic_t * getT5(const int & i) const
                {
                        if (i < 0 || i > SIGBUF)
                        {
                                return (NULL);
                        }
                        return (t5s[i]);
                }

                inline sig_atomic_t * getSig(const int & i) const
                {
                        if (i < 0 || i > SIGBUF)
                        {
                                return (NULL);
                        }
                        return (sigs[i]);
                }

                /* HERE - this entire program should be incorporated into the ais, so in the ais you should call and handle independently getSig(top) and getT5(top) */
                inline sig_atomic_t * getPair(void) const
                {
                        sig_atomic_t * t = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen + sizeof(sig_atomic_t) * t5TplLen);
                        memset(t, -1, fngPntLen + t5TplLen);
                        t = sigs[top];
                        t+fngPntLen = t5s[top];
                        top++;
                        return(t);
                }

                inline static const int32_t getLogfd(void) const
                {
                        return(logfd);
                }

                /* this is going to give some unexpected results */
#if __WORDSIZE == 64
                inline uint64_t getCt(void)
                {
                        return((uint64_t)(ct1 + (ct2*2147483647));
                }
#else
                inline uint32_t getCt(void)
                {
                        return((uint32_t)(ct1 + (ct2*2147483647));
                }
#endif

                inline static const int32_t getT5shmid(const int & i) const
                {
                        return(t5shmid[i]);
                }

                inline static const int32_t getShmid(const int & i) const
                {
                        return(shmid[i]);
                }

                inline static const int32_t * getT5shmids(void) const
                {
                        return(t5shmid);
                }

                inline static const int32_t * getShmids(void) const
                {
                        return(shmid);
                }

                inline uint32_t getSize(void) const
                {
                        return(SIGBUF);
                }

                inline sig_atomic_t getPos(void) const
                {
                        return(pos);
                }

                /* mutators */
                inline static void incTop(void)
                {
                        top++;
                        top = top % SIGBUF;
                }
                inline static void incPos(void)
                {
                        pos++;
                        pos = (((pos) % SIGQTY) != 0 ? ((pos) % SIGQTY) : SIGQTY); // 1 - 5
                }
                inline static void incCt(void)
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
                                shandler(LOG);
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
                                shandler(LOG);
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
                        shandler(NO_MEM);
                }
        }
        /* HERE */
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
                shandler(LOG);
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

