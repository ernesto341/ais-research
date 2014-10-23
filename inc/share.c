/* share.c
 *
 * Ernest Richards
 * 
 */

#include <share.h>

#ifndef SIGQTY
#define SIGQTY 5
#endif

#ifndef t5TplLen
#define t5TplLen 44
#endif

#ifndef fngPntLen
#define fngPntLen 14
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

unsigned int i = 0;

inline void dShmids(void)
{
        i = 0;
        while (i < (SIGQTY))
        {
                shmdt(t5shm[i]);
                shmctl(t5shmid[i], IPC_RMID, 0);
                i++;
        }
        i = 0;
        while (i < (SIGQTY + 1))
        {
                shmdt((sig_atomic_t *)shm[i]);
                shmctl(shmid[i], IPC_RMID, 0);
                i++;
        }
}

inline void fData(void)
{
        if (hdr_data)
        {
                free(hdr_data);
        }
        unsigned int i = 0;
        if (sigs)
        {
                while (i < (SIGQTY + 1))
                {
                        free(sigs[i++]);
                }
                free(sigs);
        }
        i = 0;
        if (t5s)
        {
                while (i < SIGQTY)
                {
                        free(t5s[i++]);
                }
                free(t5s);
        }
        i = 0;
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

void freeMem (void)
{
        dShmids();

        fShmids();
        fShms();
        fData();
}

inline void iData(void)
{
        hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * (hdr_size * 5));
        hdr_size *= 5;
        t5s = (char **)malloc(sizeof(char *) * SIGQTY);
        if (hdr_data == NULL || t5s == NULL)
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
                t5s[i++] = (char *)malloc(sizeof(char) * t5TplLen);
        }
        i = 0;
        sigs = (int **)malloc(sizeof(int *) * (SIGQTY + 1));
        while (i < (SIGQTY + 1))
        {
                sigs[i++] = (int *)malloc(sizeof(int) * fngPntLen);
        }
}

inline void iShms(void)
{
        shm = (volatile sig_atomic_t **)malloc(sizeof(sig_atomic_t *) * (SIGQTY + 1));
        t5shm = (char **)malloc(sizeof(char *) * (SIGQTY));
        if (shm == NULL || t5shm == NULL)
        {
                if (DEBUG)
                {
                        strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                        write(2, buf, 38);
                }
                _exit(-1);
        }
}

inline void iShmids(void)
{
        i = 0;
        srand(time(NULL));
        shmid = (int *)malloc(sizeof(int) * (SIGQTY + 1));
        t5shmid = (int *)malloc(sizeof(int) * (SIGQTY + 1));
        if (shmid == NULL || t5shmid == NULL)
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
                shmid[i] = shmget(shmkey[i], sizeof(sig_atomic_t) * fngPntLen, IPC_CREAT | 0666);
                if (shmid[i] < 0)
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
                t5shmid[i] = shmget(t5shmkey[i], sizeof(char) * t5TplLen, IPC_CREAT | 0666);
                if (t5shmid[i] < 0)
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

inline void aShmids(void)
{
        i = 0;
        unsigned int j = 0;
        while (i < (SIGQTY + 1))
        {
                shm[i] = shmat(shmid[i], (void *) 0, 0);
                if ((void *)shm[i] == (void *)-1)
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
                        shm[i][j++] = 0;
                }
                i++;
        }
        i = 0;
        while (i < SIGQTY)
        {
                t5shm[i] = shmat(t5shmid[i], (void *) 0, 0);
                if ((void *)t5shm[i] == (void *)-1)
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
                        t5shm[i][j++] = '0';
                }
                i++;
        }

}

void initMem(void)
{
        iData();
        iShms();
        iShmids();

        aShmids();
}

/* share.c */

