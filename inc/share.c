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
                shmdt((sig_atomic_t *)snc->t5shm[i]);
                shmctl(snc->t5shmid[i], IPC_RMID, 0);
                i++;
        }
        i = 0;
        while (i < (SIGQTY + 1))
        {
                shmdt((sig_atomic_t *)snc->shm[i]);
                shmctl(snc->shmid[i], IPC_RMID, 0);
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
        if (snc->sigs)
        {
                while (i < (SIGQTY + 1))
                {
                        free(snc->sigs[i++]);
                }
                free(snc->sigs);
        }
        i = 0;
        if (snc->t5s)
        {
                while (i < SIGQTY)
                {
                        free(snc->t5s[i++]);
                }
                free(snc->t5s);
        }
        i = 0;
}

inline void fShms(psnc_t snc)
{
        if (snc->t5shm)
        {
                free(snc->t5shm);
        }
        if (snc->shm)
        {
                free(snc->shm);
        }
}

inline void fShmids(psnc_t snc)
{
        if (snc->t5shmid)
        {
                free(snc->t5shmid);
        }
        if (snc->shmid)
        {
                free(snc->shmid);
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
        snc->t5s = (sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * SIGQTY);
        if (hdr_data == 0 || snc->t5s == 0)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
        i = 0;
        while (i < SIGQTY)
        {
                snc->t5s[i++] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen);
        }
        i = 0;
        snc->sigs = (sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        while (i < (SIGQTY + 1))
        {
                snc->sigs[i++] = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * fngPntLen);
        }
}

inline void iShms(psnc_t snc)
{
        snc->shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        snc->t5shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY));
        if (snc->shm == 0 || snc->t5shm == 0)
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
        snc->shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY + 1));
        snc->t5shmid = (int32_t *)malloc(sizeof(int32_t) * (SIGQTY));
        if (snc->shmid == 0 || snc->t5shmid == 0)
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
                snc->shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, IPC_CREAT | 0666);
                if (snc->shmid[i] < 0)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to get shm with key ", 27);
                                strncat(buf, itoa(shmkey[i]), strlen(itoa(shmkey[i])));
                                strncat(buf, "\r\n", 2);
                                write(2, buf, strlen(buf));
                        }
                        _exit(-1);
                }
                i++;
        }
        i = 0;
        while (i < (SIGQTY))
        {
                snc->t5shmid[i] = shmget(t5shmkey[i], sizeof(sig_atomic_t) * t5TplLen, IPC_CREAT | 0666);
                if (snc->t5shmid[i] < 0)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to get t5shm with key ", 29);
                                strncat(buf, itoa(t5shmkey[i]), strlen(itoa(t5shmkey[i])));
                                strncat(buf, "\r\n", 2);
                                write(2, buf, strlen(buf));
                        }
                        _exit(-1);
                }
                i++;
        }
}

inline void aShmids(psnc_t snc)
{
        i = 0;
        unsigned int j = 0;
        while (i < (SIGQTY + 1))
        {
                snc->shm[i] = shmat(snc->shmid[i], (void *) 0, 0);
                if ((void *)snc->shm[i] == (void *)-1)
                {
                        if (DEBUG)
                        {
                                strncpy(buf, "unable to attach shm", 14);
                                strncat(buf, "\r\n", 2);
                                write(2, buf, 16);
                        }
                        _exit(-1);
                }
                j = -1;
                while (j < fngPntLen)
                {
                        snc->shm[i][j++] = -1;
                }
                i++;
        }
        i = 0;
        while (i < SIGQTY)
        {
                snc->t5shm[i] = shmat(snc->t5shmid[i], (void *) 0, 0);
                if ((void *)snc->t5shm[i] == (void *)-1)
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
                        snc->t5shm[i][j++] = '0';
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
                snc->sigs[CTL][POS] = 1;
                snc->sigs[CTL][PEND] = 0;
                snc->sigs[CTL][FLAGS] = 0;
                snc->shm[CTL][POS] = 1;
                snc->shm[CTL][PEND] = 0;
                snc->shm[CTL][FLAGS] = 0;
        }
}

/* share.c */

