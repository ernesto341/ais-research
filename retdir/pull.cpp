#include <pull.hpp>

using namespace std;

int status = 0;
int32_t import_mgr_pid = -1;
int pipefd[2];

volatile sig_atomic_t do_import = 0;

ofstream fout;

bool quit = false;

void cleanup(void);
int32_t child_pid = -1;
int32_t logfd = -1;
int32_t snum = 0;

pthread_t test_mgr_tid;
pthread_t log_tid;
pthread_t import_mgr_tid;

/* ptr to double array of Antibodies */
Antibody ** champs;
pthread_mutex_t champs_mutex;
pthread_mutex_t log_mutex;

volatile sig_atomic_t testing = 0;

typedef struct _test_param
{
        uint8_t tnum; // thread number
        pthread_t tid; // thread id
        uint32_t sig[fngPntLen]; // signature to be tested
        char tuple[t5TplLen]; // t5 tuple
        volatile sig_atomic_t flag; // signal to thread to start testing
        int8_t attack; // whether or not the tested signature was determined to be an attack
} test_param, *ptest_param;

queue<test_param> log_queue;

/* v is a test_param *, ie ptest_param */
void * testThread(void * v)
{
        if (v)
        {
                fprintf(stderr, "\n\t\t[T] --- testThread: Begin\n");
                fflush(stderr);
                /* HERE - FIX THIS */
                if (((int *)(((ptest_param)(v))->sig))[0] != -1)
                {
                        fprintf(stderr, "\n\t\t[T] --- testThread: Good Signature\n");
                        fflush(stderr);
                        pthread_mutex_lock(&champs_mutex);
                        /*
                           for (unsigned int i = 0; i < fngPntLen; i++)
                           {
                           if ((champs)[i]->fitness() > MIN_FITNESS)
                           {
                           ((ptest_param)v)->attack = (uint8_t)((champs)[i]->match((int *)(((ptest_param)v)->sig)));
                           }
                           }
                           */
                        for (int i = 0; i < class_count; i++)
                        {
                                for (int j = 0; j < ab_count; j++)
                                {
                                        if ((champs)[i][j].fitness() > MIN_FITNESS)
                                        {
                                                ((ptest_param)v)->attack = (uint8_t)((champs)[i][j].match((int *)(((ptest_param)v)->sig)));
                                        }
                                }
                        }
                        pthread_mutex_unlock(&champs_mutex);

                        //if((((ptest_param)v)->attack) > 0)
                        {
                                fprintf(stderr, "\n\t\t[T] --- testThread: Something to log\n");
                                fflush(stderr);
                                pthread_mutex_lock(&log_mutex);
                                log_queue.push(*(ptest_param)(v));
                                pthread_mutex_unlock(&log_mutex);
                        }
                }
                else
                {
                        fprintf(stderr, "\n\t\t[T] --- testThread: Bad Signature\n");
                        fflush(stderr);
                }
                ((ptest_param)v)->flag = DONE;
        }

        return ((void *)0);
}

inline static void Copy(test_param & d, test_param s)
{
        d.attack = s.attack;
        unsigned int x = 0;
        for (x = 0; x < t5TplLen; x++)
        {
                d.tuple[x] = s.tuple[x];
        }
        for (x = 0; x < fngPntLen; x++)
        {
                d.sig[x] = s.sig[x];
        }
}

/*
   typedef struct _test_param
   {
   uint8_t tnum; // thread number
   pthread_t tid; // thread id
   uint32_t sig[fngPntLen]; // signature to be tested
   char tuple[t5TplLen]; // t5 tuple
   volatile sig_atomic_t flag; // signal to thread to start testing
   int8_t attack; // whether or not the tested signature was determined to be an attack
   } test_param, *ptest_param;
   */

/* child function that watches the queue for entries that need to be logged and logs new entries */
void * Stats (void * v)
{
        if (DEBUG)
        {
                fprintf(stderr, "\n\t\t[r] --- Stats: func begin\n");
        }
        test_param tmp;
        /* start while shm[flag] != done loop */
        while (!quit)
        {
                pthread_mutex_lock(&log_mutex);
                while (!log_queue.empty())
                {
                        fprintf(stderr, "\n\t\t[S] --- Stats: Something to log\n");
                        fflush(stderr);
                        /* log new item to queue */
                        Copy(tmp, log_queue.front());
                        log_queue.pop();
                        /*
                           if (tmp.attack == 1)
                           {
                           fout << "[A] --- Attack Identified:" << endl
                           << "\t" << "Unique Tuple:    " << tmp.tuple << endl
                           << "\t" << "HTTP Signature:  \t";
                           for (unsigned int i = 0; i < fngPntLen; i++)
                           {
                           fout << tmp.sig[i] << " ";
                           }
                           fout << endl;
                           }
                           else
                           {
                           */
                        fout << "[N] --- Normal Traffic ---" << endl
                                << "\t" << "Unique Tuple:    " << tmp.tuple << endl;
                        fout << "\t" << "HTTP Signature:  \t";
                        for (unsigned int i = 0; i < fngPntLen; i++)
                        {
                                fout << tmp.sig[i] << " ";
                        }
                        fout << endl;
                        /*
                           }
                           */
                }
                /* unlock never blocks, so shouldn't matter */
                pthread_mutex_unlock(&log_mutex);
        }
        /* dump overall statistics to log file */
        return ((void *)0);
}

inline static void Convert (char dst[t5TplLen], volatile sig_atomic_t src[t5TplLen])
{
        unsigned int i = 0;
        while (i < t5TplLen)
        {
                dst[i] = (char)src[i];
                i++;
        }
}

/* v is a test_param [MAX_THREADS] */
void * testMgr (void * v)
{
        if (v)
        {
                /* spawn log process */
                if ((pthread_create(&(log_tid), NULL, Stats, (void *)0)) < 0)
                {
                        perror("pthread_create()");
                }
                int i = 0;
                /* while not done */
                while (!quit)
                {
                        i = 0;
                        /* check all necessary testing params */
                        while (i < testing)
                        {
                                if (((ptest_param)v)[i].flag == START)
                                {
                                        ((ptest_param)v)[i].flag = WORKING;
                                        /* begin a test, pass (ptest_param)&((test_param *)v[i]), i.e., a ptest_param */
                                        /* HERE - exit? if unable to spawn testing threads */
                                        if ((pthread_create(&(((ptest_param)v)[i].tid), NULL, testThread, (void *)(&(((ptest_param)v)[i])))) < 0)
                                        {
                                                perror("pthread_create()");
                                        }
                                        else
                                        {
                                                if (DEBUG)
                                                {
                                                        fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_create succeeded, starting testing on a test_param struct\n");
                                                        fflush(stderr);
                                                }
                                        }
                                }
                                if (((ptest_param)v)[i].flag == DONE)
                                {
                                        /* HERE - exit? if unable to return from testing threads */
                                        if ((pthread_join((((ptest_param)v)[i].tid), NULL)) < 0)
                                        {
                                                perror("pthread_join()");
                                        }
                                        else
                                        {
                                                if (DEBUG)
                                                {
                                                        fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_join succeeded, done testing on a test_param\n");
                                                        fflush(stderr);
                                                }
                                        }
                                        testing--;
                                        ((ptest_param)v)[i].flag = COMPLETE;
                                }
                                i++;
                        }
                }
                /* for debugging/optimization purposes, figure out how many threads we actually used */
                i = 0;
                uint8_t ct = MAX_THREADS;
                while (i < MAX_THREADS)
                {
                        if (((ptest_param)v)[i].flag == UNUSED)
                        {
                                ct--;
                        }
                }
                if ((pthread_join(log_tid, NULL)) < 0)
                {
                        perror("pthread_join()");
                }
        }
        return ((void *)0);
}

void pull(Antibody ** pop, const int32_t pipefd)
{
        if (pop == NULL)
        {
                /* attempt to pull a population from file */
                /* maybe fork and exec breed and train module regularly */
                if ((champs = importChamps()) == 0)
                {
                        cout << "Unable to get antibodies to test with, quitting\n" << flush;
                        shm[CTL][FLAGS] = CDONE;
                        return;
                }

                /* excessive output
                   if (DEBUG)
                   {
                   cout << "In main. Got the following:\n";
                   for (int i = 0; i < class_count; i++)
                   {
                   cout << "Class " << i+1 << ":\n";
                   for (int j = 0; j < ab_count; j++)
                   {
                   cout << "\t" << champs[i][j] << "\n";
                   }
                   cout << endl;
                   }
                   cout << endl << flush;
                   }
                   */
        }
        else
        {
                champs = pop;
        }

        /* setup champs update signal */
        signal(SIGUSR1, &onDemandImport);

        /*
           if (DEBUG)
           {
           cout << setw(20) << right << "Champs = \t" << champs << endl;
           for (unsigned int i = 0; i < class_count; i++)
           {
           cout << setw(20) << right << "champs[" << i << "] = \t" << champs[i] << endl;
           for (unsigned int j = 0; j < ab_count; j++)
           {
           cout << setw(20) << right << "champs[" << i << "][" << j << "] = \t" << champs[i][j] << endl;
           }
           }
           cout << endl;
           shm[CTL][FLAGS] = CDONE;
           return;
           }
           */

        uint32_t i = 0;
        pthread_mutex_init(&champs_mutex, NULL);
        pthread_mutex_init(&log_mutex, NULL);
        char * n = (char *)"./traffic.log\0";

        /* open log file */
        fout.open(n);
        if (fout.fail())
        {
                if (DEBUG)
                {
                        cout << "Unable to open file for logging\n" << flush;
                }
        }

        /* parent - retrieve loop */
        if (DEBUG)
        {
                fprintf(stderr, "\n\t\t[r] --- fork succeeded - parent\n");
        }
        test_param params[MAX_THREADS];
        for (i = 0; i < MAX_THREADS; i++)
        {
                params[i].flag = UNUSED;
                params[i].tnum = i;
        }
        /* start testMgr with &params[] */
        /* HERE - cleanup and exit */
        if ((pthread_create(&(test_mgr_tid), NULL, testMgr, (void *)(&(params)))) < 0)
        {
                perror("pthread_create()");
        }
        else
        {
                if (DEBUG)
                {
                        // Retrieve
                        fprintf(stderr, "\n\t\t[r] --- pthread_create succeeded\n");
                }
        }
        testing = 0;
        if ((pthread_create(&(import_mgr_tid), NULL, importManager, (void *)(0))) < 0)
        {
                perror("pthread_create()");
        }
        else
        {
                if (DEBUG)
                {
                        // Retrieve
                        fprintf(stderr, "\n\t\t[r] --- pthread_create succeeded\n");
                }
        }
        if (DEBUG)
        {
                // Retrieve
                fprintf(stderr, "\tentering loop\n");
        }

        /* accept signal from producer to quit */
        while (shm[CTL][FLAGS] != PDONE && shm[CTL][FLAGS] != CDONE)
        {
                /* only copy when dhs isn't writing and while there are pending headers to get - add mutex functionality?
                */
                while (shm[CTL][PEND] > 0)
                {
                        if (shm[CTL][FLAGS] == PWTEN || shm[CTL][FLAGS] == CREAD)
                        {
                                if (DEBUG)
                                {
                                        // Retrieve
                                        fprintf(stderr, "\tshm[CTL][FLAGS] == PWTEN\n");
                                }
                                shm[CTL][FLAGS] = CRING;
                                memcpy((void *)retrieved_sigs[ct], (void *)shm[(shm[CTL][POS])], sizeof(sig_atomic_t) * fngPntLen);
                                memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS])-1], sizeof(sig_atomic_t) * t5TplLen);
                                /* HERE - lock individual param structs before modification? */
                                if (testing < MAX_THREADS)
                                {
                                        memcpy((void *)&(params[testing].sig), (void *)retrieved_sigs[ct], sizeof(sig_atomic_t) * fngPntLen);
                                        /* convert sig_atomic_t to char */
                                        Convert(params[testing].tuple, retrieved_t5s[ct]);
                                        /* these two lines signal the testMgr to spawn a testing thread */
                                        testing++;
                                        params[testing].flag = START;
                                }
                                else
                                {
                                        fprintf(stderr, "Insufficient threads to process incoming signatures\n");
                                        fflush(stderr);
                                }
                                if (DEBUG)
                                {
                                        fprintf(stderr, "pos = %d, pend = %d\n", shm[CTL][POS], shm[CTL][PEND]);
                                        fprintf(stderr, "\nsig:\n");
                                        i = 0;
                                        while (i < fngPntLen)
                                        {
                                                fprintf(stderr, "\t%d - %d", i, retrieved_sigs[ct][i]);
                                                fprintf(stderr, "\n");
                                                i++;
                                        }
                                        fprintf(stderr, "\t");
                                        i = 0;
                                        while (i < t5TplLen)
                                        {
                                                fprintf(stderr, "%c", retrieved_t5s[ct][i]);
                                                i++;
                                        }
                                        fprintf(stderr, "\n");
                                        fflush(stderr);
                                }
                                /* keep an accurate record of our buffer position and dhs' buffer position */
                                ct = (ct+1)%SIGBUF;
                                inPos();

                                /* decrement pending counter
                                 * should never go below 0,
                                 * so this test is probably unnecessary
                                 */
                                if (shm[CTL][PEND] > 0)
                                {
                                        shm[CTL][PEND]--;
                                }
                                /* unlock shared resource */
                                shm[CTL][FLAGS] = CREAD;
                        }
                }
        }
        quit = true;
        /* signal dhs that retrieve is done */
        if (shm)
        {
                shm[CTL][FLAGS] = CDONE;
        }
        /* join testMgr. since fork/logging is not a priority, make sure there is actually a child process before waiting for child process */
        if ((pthread_join(import_mgr_tid, NULL)) < 0)
        {
                perror("pthread_join()");
                /* HERE - cleanup and exit */
        }
        if ((pthread_join(test_mgr_tid, NULL)) < 0)
        {
                perror("pthread_join()");
                /* HERE - cleanup and exit */
        }
        /*
           delete champs;
           pthread_mutex_destroy(&champs_mutex);
           */
        cleanup();

        exit(EXIT_SUCCESS);
        return;
}

void cleanup(void)
{
        delete champs;
        pthread_mutex_destroy(&champs_mutex);
        pthread_mutex_destroy(&log_mutex);
        /* close log file */
        fout.close();
}
