/* share.c
 *
 * Ernest Richards
 * 
 */

#include <share.h>

inline void dShmids(void)
{
        int i = 0;
        while (i < 6)
        {
                shmdt(t5shm[i]);
                shmctl(t5shmid[i], IPC_RMID, 0);
                i++;
        }
        i = 0;
        while (i < 6)
        {
                shmdt(shm[i]);
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
}

inline void fShms(void)
{
        if (shm)
        {
                free(shm);
        }
}

inline void fShmids(void)
{
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
        t5s = (char **)malloc(sizeof(char *) * 5);
        if (hdr_data == NULL || t5s == NULL)
        {
                strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                write(2, buf, strlen(buf));
                _exit(-1);
        }
        int i = 0;
        while (i < 5)
        {
                t5s[i++] = (char *)malloc(sizeof(char) * 44);
        }
        i = 0;
        sigs = (int **)malloc(sizeof(int *) * 6);
        while (i < 6)
        {
                sigs[i++] = (int *)malloc(sizeof(int) * 14);
        }
}

inline void iShms(void)
{
        shm = (int **)malloc(sizeof(int *) * 6);
        t5shm = (char **)malloc(sizeof(char *) * 6);
        if (shm == NULL || t5shm == NULL)
        {
                strncpy(buf, "Unable to allocate sufficient memory\r\n", 38);
                write(2, buf, strlen(buf));
                _exit(-1);
        }
}

inline void iShmids(void)
{
        int i = 0;
        srand(time(NULL));
        shmid = (int *)malloc(sizeof(int) * 6);
        t5shmid = (int *)malloc(sizeof(int) * 6);
        while (i < 6)
        {
                shmid[i] = shmget(shmkey[i], sizeof(int) * 14, IPC_CREAT | IPC_EXCL | 0600);
                if (shmid[i] < 0)
                {
                        strncpy(buf, "unable to get shm with key ", 27);
                        strncat(buf, itoa(shmkey[i]), strlen(itoa(shmkey[i])));
                        strncat(buf, "\r\n", 2);
                        write(2, buf, strlen(buf));
                        _exit(-1);
                }
                t5shmid[i] = shmget(t5shmkey[i], sizeof(char) * 44, IPC_CREAT | IPC_EXCL | 0600);
                if (t5shmid[i] < 0)
                {
                        strncpy(buf, "unable to get t5shm with key ", 29);
                        strncat(buf, itoa(t5shmkey[i]), strlen(itoa(t5shmkey[i])));
                        strncat(buf, "\r\n", 2);
                        write(2, buf, strlen(buf));
                        _exit(-1);
                }
                i++;
        }
}

inline void aShmids(void)
{
        int i = 0;
        while (i < 6)
        {
                shm[i] = shmat(shmid[i], (void *) 0, 0);
                t5shm[i] = shmat(t5shmid[i], (void *) 0, 0);
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

