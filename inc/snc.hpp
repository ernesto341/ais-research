#ifndef _SNC_HPP_
#define _SNC_HPP_

#pragma once

#include <globals.h>

#define NO_MEM 7

using namespace std;

#ifndef _shandler
#define _shandler
void shandler(const int * s)
{
}
#endif

/* storage and control */
class snc
{
        private:
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
                uint32_t logfd;

                /* routines for logging */
                uint32_t openLog(const char * n = NULL, uint32_t * fd = NULL);
                uint8_t Log(const char *);

        public:
                /* constructors */
                snc(void)
                {
                        SIGBUF = 50;
                        if (Alloc() < 0)
                        {
                                Log("Insufficient memory\n\0");
                                shandler(NO_MEM);
                        }
                }
                snc(const snc &);

                ~snc(void);

                /* accessors */

                /* mutators */
                inline static void incCt(void)
                {
                        ((pos))++;
                        ((pos)) = ((((pos)) % SIGQTY) != 0 ? (((pos)) % SIGQTY) : SIGQTY); // 1 - 5
                }
                inline static void incPos(void)
                {
                        ct1++;
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
};

#endif

                uint8_t Log(const char * c)
                {
                        if (!c || logfd < 0)
                        {
                                return (-1);
                        }
                        while (*c != '\0')
                        {
                                if ((putc(*c, logfd)) < 0)
                                {
                                        putc("L", stdout);
                                        shandler(9);
                                }
                        }
                }

                uint32_t openLog(const char * n, uint32_t * fd)
                {
                        uint32_t *ret = fd;
                        if (!ret)
                        {
                                if ((ret = (uint32_t *)malloc(sizeof(uint32_t) * 1)) < 0)
                                {
                                        Log("Insufficient memory\n\0");
                                        shandler(NO_MEM);
                                }
                        }
                        char *
                }

