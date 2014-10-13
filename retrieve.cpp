#include <iostream>
#include <stdtlib>

using namespace std;

#define SIGQTY 5

#ifndef fngPntLen
#define fngPntLen 14
#endif

inline static void inCtr(int ** s)
{
        (*s[0])++;
        (*s[0]) = (((*s[0]) % SIGQTY) > 0 ? ((*s[0]) % SIGQTY) : (*s[0]) % SIGQTY + 1); // 1 - 5
}

inline static void initShm(void)
{
        /* get sufficient memory to hold a large header */
        shmid = shmget(shmkey, shm_size + (sizeof(int) * fngPntLen), IPC_CREAT | IPC_EXCL | 0655);
        if (shmid < 0)
        {
                write(2, "unable to get shared memory\n", 28);
                _exit(-1);
        }
        shm = shmat(shmid, (void *) 0, 0);
}

inline static void initMem (void)
{
        sigs = (int **)malloc(sizeof(int *) * (SIGQTY + 1));
        int i = 0;
        while (i < (SIGQTY + 1))
        {
                sigs[i++] = (int *)malloc(sizeof(int) * fngPntLen);
        }
        *sigs[0] = 1; // count/position
        *(sigs[0] + 4) = 0; // pending
        hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size * 5);
        hdr_size *= 5;
}

inline static void freeMem (void)
{
        if (sigs)
        {
                int i = 0;
                while (i < (SIGQTY + 1))
                {
                        free(sigs[i++]);
                }
                free(sigs);
        }
        if (hdr_data)
        {
                free(hdr_data);
        }
}

void shandler ( int sign )
{
        shmdt(shm);
        shmctl(shmid, IPC_RMID, 0);
        freeMem();

        fprintf( stderr, "\n\n[+] Retrieve finished!\n" );
        exit( sign );
}

int main (void)
{
        shandler (0);
}
