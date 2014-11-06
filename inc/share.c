/* share.c
 *
 * Ernest Richards
 * 
 */

#include <share.h>

#ifndef DEBUG
#define DEBUG 0
#endif

unsigned int i = 0;

inline void dShmids(psnc_t snc)
{
        i = 0;
        while (i < (SIGQTY))
        {
                shmdt((sig_atomic_t *)snc->smem.t5shm[i]);
                shmctl(snc->smem.t5shmid[i], IPC_RMID, 0);
                i++;
        }
        i = 0;
        while (i < (SIGQTY + 1))
        {
                shmdt((sig_atomic_t *)snc->smem.shm[i]);
                shmctl(snc->smem.shmid[i], IPC_RMID, 0);
                i++;
        }
}

inline void fData(psnc_t snc)
{
        if (hdr_data)
        {
                free(hdr_data);
        }
        unsigned int i = 0;
        if (snc->mem.sigs)
        {
                while (i < (SIGQTY + 1))
                {
                        free(snc->mem.sigs[i++]);
                }
                free(snc->mem.sigs);
        }
        i = 0;
        if (snc->mem.t5s)
        {
                while (i < SIGQTY)
                {
                        free(snc->mem.t5s[i++]);
                }
                free(snc->mem.t5s);
        }
        i = 0;
}

inline void fShms(psnc_t snc)
{
        if (snc->smem.t5shm)
        {
                free(snc->smem.t5shm);
        }
        if (snc->smem.shm)
        {
                free(snc->smem.shm);
        }
}

inline void fShmids(psnc_t snc)
{
        if (snc->smem.t5shmid)
        {
                free(snc->smem.t5shmid);
        }
        if (snc->smem.shmid)
        {
                free(snc->smem.shmid);
        }
}

void freeMem (psnc_t snc)
{
        if (snc)
        {
                dShmids(snc);

                fShmids(snc);
                fShms(snc);
                fData(snc);
        }
}

inline void iData(psnc_t snc)
{
        hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * (hdr_size * 5));
        hdr_size *= 5;
        snc->mem.t5s = (sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGQTY);
        snc->mem.sigs = (sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        if (hdr_data == 0 || snc->mem.t5s == 0 || snc->mem.sigs == 0)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
        i = 0;
        while (i < (SIGQTY + 1))
        {
                snc->mem.sigs[i++] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                if (i < SIGQTY)
                {
                        snc->mem.t5s[i++] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                }
        }
}

inline void iShms(psnc_t snc)
{
        snc->smem.shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        snc->smem.t5shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY));
        if (snc->smem.shm == 0 || snc->smem.t5shm == 0)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
}

inline void iShmids(psnc_t snc)
{
        i = 0;
        srand(time(NULL));
        snc->smem.shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY + 1));
        snc->smem.t5shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY));
        if (snc->smem.shmid == 0 || snc->smem.t5shmid == 0)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
        while (i < (SIGQTY + 1))
        {
                snc->smem.shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, IPC_CREAT | 0666);
                if (snc->smem.shmid[i] < 0)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to get shm with key ", 27);
                                strncat(buf, itoa(shmkey[i]), strlen(itoa(shmkey[i])));
                                strncat(buf, "\n", 1);
                                write(2, buf, strlen(buf));
                        }
                        _exit(-1);
                }
                if (i < (SIGQTY))
                {
                        snc->smem.t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, IPC_CREAT | 0666);
                        if (snc->smem.t5shmid[i] < 0)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to get t5shm with key ", 29);
                                        strncat(buf, itoa(t5shmkey[i]), strlen(itoa(t5shmkey[i])));
                                        strncat(buf, "\n", 1);
                                        write(2, buf, strlen(buf));
                                }
                                _exit(-1);
                        }
                }
                i++;
        }
        i = 0;
}

inline void aShmids(psnc_t snc)
{
        i = 0;
        unsigned int j = 0;
        while (i < (SIGQTY + 1))
        {
                snc->smem.shm[i] = shmat(snc->smem.shmid[i], (void *) 0, 0);
                if ((void *)snc->smem.shm[i] == (void *)-1)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to attach shm", 14);
                                strncat(buf, "\r\n", 2);
                                write(2, buf, 16);
                        }
                        _exit(-1);
                }
                j = 0;
                while (j < fngPntLen)
                {
                        snc->smem.shm[i][j++] = -1;
                }
                if (i < SIGQTY)
                {
                        snc->smem.t5shm[i] = shmat(snc->smem.t5shmid[i], (void *) 0, 0);
                        if ((void *)snc->smem.t5shm[i] == (void *)-1)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to attach t5shm", 16);
                                        strncat(buf, "\r\n", 2);
                                        write(2, buf, 18);
                                }
                                _exit(-1);
                        }
                        j = 0;
                        while (j < t5TplLen)
                        {
                                snc->smem.t5shm[i][j++] = '0';
                        }
                        i++;
                }
                i++;
        }
        i = 0;
}

void initMem(psnc_t snc)
{
        if (snc)
        {
                iData(snc);
                iShms(snc);
                iShmids(snc);

                aShmids(snc);

                /* initialize some values */
                snc->mem.sigs[CTL][POS] = 1;
                snc->mem.sigs[CTL][PEND] = 0;
                snc->mem.sigs[CTL][FLAGS] = 0;
                snc->smem.shm[CTL][POS] = 1;
                snc->smem.shm[CTL][PEND] = 0;
                snc->smem.shm[CTL][FLAGS] = 0;
        }
}

/* share.c */

