/* share.c
 *
 * Ernest Richards
 * 
 */

#include <share.h>

#ifndef DEBUG
#define DEBUG 1
#endif

inline void dShmids(psnc_t snc)
{
        uint32_t i = 0;
        while (i < (SIGQTY + 1))
        {
                shmdt((sig_atomic_t *)snc->smem.shm[i]);
                shmctl(snc->smem.shmid[i], IPC_RMID, 0);
                if (i < (SIGQTY))
                {
                        shmdt((sig_atomic_t *)snc->smem.t5shm[i]);
                        shmctl(snc->smem.t5shmid[i], IPC_RMID, 0);
                }
                i++;
        }
}

inline void fData(psnc_t snc)
{
        if (hdr_data != 000)
        {
                memset(hdr_data,'0', hdr_size);
                memset(hdr_data,'1', hdr_size);
                memset(hdr_data,'\0', hdr_size);
                free(hdr_data);
                hdr_data = 000;
        }
        uint32_t i = 0;
        if (snc->mem.sigs != 000)
        {
                while (i < (SIGQTY + 1))
                {
                        memset(snc->mem.sigs[i], '0', fngPntLen);
                        memset(snc->mem.sigs[i], '1', fngPntLen);
                        memset(snc->mem.sigs[i], '\0', fngPntLen);
                        free(snc->mem.sigs[i]);
                        snc->mem.sigs[i] = 000;
                        i++;
                }
                free(snc->mem.sigs);
                snc->mem.sigs = 000;
        }
        i = 0;
        if (snc->mem.t5s != 000)
        {
                while (i < SIGQTY)
                {
                        memset(snc->mem.t5s[i], '0', t5TplLen);
                        memset(snc->mem.t5s[i], '1', t5TplLen);
                        memset(snc->mem.t5s[i], '\0', t5TplLen);
                        free(snc->mem.t5s[i]);
                        snc->mem.t5s[i] = 000;
                        i++;
                }
                free(snc->mem.t5s);
                snc->mem.t5s = 000;
        }
}

inline void fShms(psnc_t snc)
{
        if (snc->smem.t5shm != 000)
        {
                free(snc->smem.t5shm);
                snc->smem.t5shm = 000;
        }
        if (snc->smem.shm != 000)
        {
                free(snc->smem.shm);
                snc->smem.shm = 000;
        }
}

inline void fShmids(psnc_t snc)
{
        if (snc->smem.t5shmid != 000)
        {
                *(snc->smem.t5shmid) = 0;
                free(snc->smem.t5shmid);
                snc->smem.t5shmid = 000;
        }
        if (snc->smem.shmid != 000)
        {
                *(snc->smem.shmid) = 0;
                free(snc->smem.shmid);
                snc->smem.shmid = 000;
        }
}

void freeMem (psnc_t snc)
{
        if (snc != 000)
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
                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                        write(2, buf, 37);
                }
                _exit(-1);
        }
        uint32_t i = 0;
        while (i < (SIGQTY + 1))
        {
                snc->mem.sigs[i] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
                if (snc->mem.sigs[i] == 0)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                                write(2, buf, 37);
                        }
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        snc->mem.t5s[i] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
                        if (snc->mem.t5s[i] == 0)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                                        write(2, buf, 37);
                                }
                                _exit(-1);
                        }
                }
                i++;
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
                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                        write(2, buf, 37);
                }
                _exit(-1);
        }
}

inline void iShmids(psnc_t snc)
{
        uint32_t i = 0;
        srand(time(NULL));
        snc->smem.shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY + 1));
        snc->smem.t5shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY));
        if (snc->smem.shmid == 0 || snc->smem.t5shmid == 0)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\n", 37);
                        write(2, buf, 37);
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
}

inline void aShmids(psnc_t snc)
{
        uint32_t i = 0;
        while (i < (SIGQTY + 1))
        {
                snc->smem.shm[i] = shmat(snc->smem.shmid[i], (void *) 0, 0);
                if ((void *)snc->smem.shm[i] == (void *)-1)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to attach shm\n", 15);
                                write(2, buf, 15);
                        }
                        _exit(-1);
                }
                if (i < SIGQTY)
                {
                        snc->smem.t5shm[i] = shmat(snc->smem.t5shmid[i], (void *) 0, 0);
                        if ((void *)snc->smem.t5shm[i] == (void *)-1)
                        {
                                if (DEBUG)
                                {
                                        strncpy(buf, "unable to attach t5shm\n", 17);
                                        write(2, buf, 17);
                                }
                                _exit(-1);
                        }
                }
                i++;
        }
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
                uint32_t i = 0;
                uint32_t j = 0;
                while (i < SIGQTY + 1)
                {
                        j = 0;
                        while (j < fngPntLen)
                        {
                                snc->smem.shm[i][j] = -1;
                                j++;
                        }
                        if (i < SIGQTY)
                        {
                                j = 0;
                                while (j < t5TplLen)
                                {
                                        snc->smem.t5shm[i][j] = -1;
                                        j++;
                                }
                        }
                        i++;
                }
                snc->mem.sigs[CTL][POS] = 1;
                snc->mem.sigs[CTL][PEND] = 0;
                snc->mem.sigs[CTL][FLAGS] = 0;
                snc->smem.shm[CTL][POS] = 1;
                snc->smem.shm[CTL][PEND] = 0;
                snc->smem.shm[CTL][FLAGS] = 0;
        }
}

/* share.c */

