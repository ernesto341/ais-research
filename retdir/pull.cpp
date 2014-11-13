#include <pull.hpp>

using namespace std;

int32_t child_pid = -1;
int32_t logfd = -1;
int32_t snum = 0;

pthread_t test_mgr_tid;

Antibody ** champs;

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
                if (DEBUG)
                {
                        fprintf(stderr, "\n\t\t[r] --- testThread begin\n");
                        fflush(stderr);
                }
                /* HERE - FIX THIS */
                if (((int *)(((ptest_param)(v))->sig))[0] != -1)
                {
                        for (unsigned int i = 0; i < fngPntLen; i++)
                        {
                                if (champs[i]->fitness() > MIN_FITNESS)
                                {
                                        ((ptest_param)v)->attack = (uint8_t)(champs[i]->match((int *)(((ptest_param)v)->sig)));
                                }
                        }

                        if((((ptest_param)v)->attack) > 0)
                        {
                                // HERE - need to enforce mutual exclusion?
                                log_queue.push(*((ptest_param)(v)));
                        }
                }
                ((ptest_param)v)->flag = DONE;
                if (DEBUG)
                {
                        fprintf(stderr, "\n\t\t[r] --- testThread end\n");
                        fflush(stderr);
                }
        }

        return ((void *)0);
}

/* child function that watches the queue for entries that need to be logged and logs new entries */
void Stats (const char * n = "traffic.log\0")
{
        if (DEBUG)
        {
                fprintf(stderr, "\n\t\t[r] --- Stats: func begin\n");
        }
        /* open log file */
        ofstream fout (n);
        if (fout.fail())
        {
                if (DEBUG)
                {
                        cout << "Unable to open file for logging\n" << flush;
                }
                return;
        }
        test_param tmp;
        /* start while shm[flag] != done loop */
        while (shm[FLAGS][CTL] != PDONE && shm[FLAGS][CTL] != CDONE)
        {
                /* start while item in queue loop */
                while (log_queue.size() != 0)
                {
                        if (DEBUG)
                        {
                                cout << "Something to log is in the queue!\n" << flush;
                        }
                        /* log new item to queue */
                        tmp = log_queue.front();
                        log_queue.pop();
                        if (tmp.attack == 1)
                        {
                                if (DEBUG)
                                {
                                        cout << "An attack\n" << flush;
                                }
                                fout << "[A] --- Attack Identified:" << endl
                                        << "\t" << "Unique Tuple:    " << tmp.tuple << endl
                                        << "\t" << "HTTP Signature:  " << tmp.sig << endl;
                        }
                        else
                        {
                                if (DEBUG)
                                {
                                        cout << "Normal traffic\n" << flush;
                                }
                                fout << "[N] --- Normal Traffic:" << endl
                                        << "\t" << "Unique Tuple:    " << tmp.tuple << endl
                                        << "\t" << "HTTP Signature:  " << tmp.sig << endl;
                        }
                }
        }
        /* dump overall statistics to log file */
        /* close log file */
        fout.close();
        return;
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
                int i = 0;
                /* while not done */
                while (shm[CTL][FLAGS] != PDONE && shm[CTL][FLAGS] != CDONE)
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
                                                        fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_create succeeded\n");
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
                                                        fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_join succeeded\n");
                                                }
                                        }
                                        testing--;
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
        }
        return ((void *)ct);
}

int alen = 0;
int class_count = 0;
int ab_count = 0;

bool importChamps (Antibody ** & pop, const string fin = "./ais/champions.abs")
{
        ifstream in(fin.c_str());
        if (in.fail())
        {
                cout << "Unable to open champs file\n" << flush;
                return (false);
        }
        char c = '\0';
        alen = 0;
        class_count = 0;
        /* maybe check read/calculated values below against what they should be? */
        while (in.get(c))
        {
                if (ab_count == 0)
                {
                        if (c == ',')
                        {
                                alen++;
                        }
                        else if (c == ';')
                        {
                                class_count++;
                        }
                }
                if (c == '\n')
                {
                        ab_count++;
                }
        }
        in.close();
        if (DEBUG)
        {
                cout << "In import function\n";
                cout << "Got alen: " << alen << endl;
                cout << "Got class_count: " << class_count << endl;
                cout << "Got ab_count: " << ab_count << endl;
        }

        pop = new Antibody *[class_count];
        for (int i = 0; i < class_count; i++)
        {
                pop[i] = new Antibody [ab_count];
        }

        in.open(fin.c_str());
        if (in.fail())
        {
                cout << "Unable to open champs file\n" << flush;
                return (false);
        }
        int i = 0, j = 0, k = 0, tmp = 0;
        while (in.peek() != EOF)
        {
                i = 0;
                while (i < class_count)
                {
                        j = 0;
                        while (j < ab_count)
                        {
                                k = 0;
                                while (k < alen)
                                {
                                        in >> tmp;
                                        pop[i][j].setFlag(k, tmp);
                                        k++;
                                }
                                in >> tmp;
                                pop[i][j].setTests(tmp);
                                in >> tmp;
                                pop[i][j].setPos(tmp);
                                in >> tmp;
                                pop[i][j].setFalsePos(tmp);
                                in >> tmp;
                                pop[i][j].setNeg(tmp);
                                in >> tmp;
                                pop[i][j].setFalseNeg(tmp);
                                k = 0;
                                while (k < class_count)
                                {
                                        in >> tmp;
                                        pop[i][j].setCat(k, tmp);
                                        k++;
                                }
                                j++;
                        }
                        i++;
                }
        }
        in.close();

        /* Excessive output
           if (DEBUG)
           {
           cout << "Done importing. Got the following:\n";
           for (int i = 0; i < class_count; i++)
           {
           cout << "Class " << i+1 << ":\n";
           for (int j = 0; j < ab_count; j++)
           {
           cout << "\t" << pop[i][j] << "\n";
           }
           cout << endl;
           }
           cout << endl << flush;
           }
           */

        return (true);
}

void pull(Antibody ** pop, const int32_t pipefd)
{
        if (pop == NULL)
        {
                /* attempt to pull a population from file */
                /* maybe fork and exec breed and train module regularly */
                if (!importChamps(champs))
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

        /* pipe to recieve new antibody populations from breed and train module ??? */
        if (pipefd < 0)
        {
        }

        uint32_t i = 0;

        /* fork log process */
        if ((child_pid = fork()) < 0)
        {
                perror("fork()");
        }
        /* child - logging process */
        if (child_pid == 0)
        {
                if (DEBUG)
                {
                        fprintf(stderr, "\n\t\t[r] --- fork succeeded - child\n");
                }
                Stats();
                exit(0);
        }
        /* parent - retrieve loop */
        else
        {
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
                if (DEBUG)
                {
                        // Retrieve
                        fprintf(stderr, "\tentering loop\r\n");
                }

                /* accept signal from producer to quit */
                while (shm[CTL][FLAGS] != PDONE)
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
                                                fprintf(stderr, "\tshm[CTL][FLAGS] == PWTEN\r\n");
                                        }
                                        shm[CTL][FLAGS] = CRING;
                                        memcpy((void *)retrieved_sigs[ct], (void *)shm[(shm[CTL][POS])], sizeof(sig_atomic_t) * fngPntLen);
                                        memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS])-1], sizeof(sig_atomic_t) * t5TplLen);
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
                                                fprintf(stderr, "pos = %d, pend = %d\r\n", shm[CTL][POS], shm[CTL][PEND]);
                                                fprintf(stderr, "\r\nsig:\r\n");
                                                i = 0;
                                                while (i < fngPntLen)
                                                {
                                                        fprintf(stderr, "\t%d - %d", i, retrieved_sigs[ct][i]);
                                                        fprintf(stderr, "\r\n");
                                                        i++;
                                                }
                                                fprintf(stderr, "\t");
                                                i = 0;
                                                while (i < t5TplLen)
                                                {
                                                        fprintf(stderr, "%c", retrieved_t5s[ct][i]);
                                                        i++;
                                                }
                                                fprintf(stderr, "\r\n");
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
                if (DEBUG)
                {
                        if (shm[CTL][FLAGS] == PDONE)
                        {
                                fprintf(stderr, "\n\t\t[i] --- Signaled to quit by producer\n");
                                fflush(stderr);
                        }
                }
                /* signal dhs that retrieve is done */
                shm[CTL][FLAGS] = CDONE;
                /* join testMgr. since fork/logging is not a priority, make sure there is actually a child process before waiting for child process */
                if ((pthread_join(test_mgr_tid, NULL)) < 0)
                {
                        perror("pthread_join()");
                        /* HERE - cleanup and exit */
                }
                if (child_pid > 0)
                {
                        wait(&snum);
                }

                exit(EXIT_SUCCESS);
        }
        return;
}
