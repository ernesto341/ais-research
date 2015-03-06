#include <pull.hpp>

using namespace std;

int status = 0;
int32_t import_mgr_pid = -1;
//int pipefd[2];

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

volatile static sig_atomic_t testing = 0;
volatile sig_atomic_t alen = 0;
volatile sig_atomic_t class_count = 0;
volatile sig_atomic_t ab_count = 0;

typedef struct _test_param
{
        uint8_t tnum; // thread number
        pthread_t tid; // thread id
        uint32_t sig[fngPntLen]; // signature to be tested
        char tuple[t5TplLen]; // t5 tuple
        volatile sig_atomic_t flag; // signal to thread to start testing
        uint32_t attack; // whether or not the tested signature was determined to be an attack
        string debug_buf;
} test_param, *ptest_param;

queue<test_param> log_queue;

inline static void inPos(void)
{
        ((local_pos))++;
        ((local_pos)) = ((((local_pos)) % SIGQTY) != 0 ? (((local_pos)) % SIGQTY) : SIGQTY); // 1 - 5
}
// Check if expressed attributes match given input
// Returns 1 if matches, 0 if not matches
int doMatch(Antibody a, int *test)
{
        ofstream of;
        of.open((const char *)"./match.log\0", ios_base::app | ios_base::ate | ios_base::out);
        int do_log = 0;
        if (of.good())
                do_log = 1;
        a.incTests();
        if (do_log == 1)
                of << endl << __DATE__ << " " << __TIME__<< endl << endl << flush;
        if(a.getFlag(a.COMMAND) && !(a.getAttr(a.COMMAND) & test[a.COMMAND]))
        {
                if (do_log == 1)
                        of << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[COMMAND] && !(a[COMMAND] & test[COMMAND])\n" << endl << flush;
                if (do_log == 1)
                {
                        of.close();
                        do_log = 0;
                }
                return (0);
        }
        if(a.getFlag(a.PROTOCOL) && !(a.getAttr(a.PROTOCOL) & test[a.PROTOCOL]))
        {
                if (do_log == 1)
                        of << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[PROTOCOL] && !(a[PROTOCOL] & test[PROTOCOL])\n" << endl << flush;
                if (do_log == 1)
                {
                        of.close();
                        do_log = 0;
                }
                return (0);
        }
        for(int i = a.LENGTH; i < ALEN; i++)
        {
                if(!a.getFlag(i))
                {
                        continue;
                }
                int overf = (int)pow(2.0, (double)a.getMax(i)) + (int)pow(2.0, (double)a.getMax(i) - 1.0) - 2;
                if (test[i] > overf)
                {
                        if (do_log == 1)
                                of << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\ttest[i] > overf\n" << endl << flush;
                        if (do_log == 1)
                        {
                                of.close();
                                do_log = 0;
                        }
                        return (1);  // Over max antibody match, assume is attack
                }
                if (test[i] < (a.getAttr(i) - a.getOff(i)) || test[i] > (a.getAttr(i) + a.getOff(i)))
                {
                        if (do_log == 1)
                                of << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\t(test[i] < (a[i] - offset[i]) || test[i] > (a[i] + offset[i]))\n" << endl << flush;
                        if (do_log == 1)
                        {
                                of.close();
                                do_log = 0;
                        }
                        return (0);
                }
        }
        if (do_log == 1)
                of << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\tDefault Case\n" << endl << flush;
        if (do_log == 1)
        {
                of.close();
                do_log = 0;
        }
        return (1);
}

/* v is a test_param *, ie ptest_param */
void * testThread(void * v)
{
        if (v != 000)
        {
                /*
                   fprintf(stderr, "\n\t\t[T] --- testThread: Begin\n");
                   fflush(stderr);
                   */
                pthread_mutex_lock(&champs_mutex);
                (((ptest_param)v)->debug_buf) = "";
                /* DEBUG */
                ((ptest_param)v)->attack = 0;
                for (int i = 0; i < class_count; i++)
                {
                        for (int j = 0; j < ab_count; j++)
                        {
                                //if ((champs)[i][j].fitness() > MIN_FITNESS)
                                {
                                        fprintf(stderr, "\n\tAbout to call doMatch\n\n");
                                        fflush(stderr);
                                        ((ptest_param)v)->attack += (doMatch( (champs[i][j]) , (int *)(((ptest_param)v)->sig) ));
                                        //((ptest_param)v)->attack = (uint8_t)((champs)[i][j].match_debug((int *)(((ptest_param)v)->sig)));
                                }
                        }
                }
                pthread_mutex_unlock(&champs_mutex);

                fprintf(stderr, "\n\t\t[T] --- testThread: Something to log\n");
                fflush(stderr);
                pthread_mutex_lock(&log_mutex);
                log_queue.push(*(ptest_param)(v));
                pthread_mutex_unlock(&log_mutex);
                ((ptest_param)v)->flag = DONE;
        }
        else
        {
                cerr << "\n\t\t[T] --- Nothing passed to testThread\n" << flush;
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

/* child function that watches the queue for entries that need to be logged and logs new entries */
void * Stats (void * v)
{
        if (DEBUG)
        {
                fprintf(stderr, "\n\t\t[r] --- Stats: func begin\n");
        }
        test_param tmp;
        while (!quit)
        {
                pthread_mutex_lock(&log_mutex);
                while (!log_queue.empty())
                {
                        /*
                           fprintf(stderr, "\n\t\t[S] --- Stats: Something to log\n");
                           fflush(stderr);
                           */
                        /* log new item to queue */
                        Copy(tmp, log_queue.front());
                        log_queue.pop();
                        if (tmp.attack >= AGREE)
                        {
                                fout << "[A] --- Attack Identified:" << endl;
                                fout << "\t" << "Unique Tuple:    " << tmp.tuple << endl;
                                fout << "\t" << "HTTP Signature:  \t";
                                for (unsigned int i = 0; i < fngPntLen; i++)
                                {
                                        fout << tmp.sig[i] << " ";
                                }
                        }
                        else
                        {
                                fout << "[N] --- Normal Traffic ---" << endl;
                                fout << "\t" << "Unique Tuple:    " << tmp.tuple << endl;
                                fout << "\t" << "HTTP Signature:  \t";
                                for (unsigned int i = 0; i < fngPntLen; i++)
                                {
                                        fout << tmp.sig[i] << " ";
                                }
                        }
                        fout << endl;
                }
                pthread_mutex_unlock(&log_mutex);
        }
        /* HERE - dump overall statistics to log file */
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
        if (v != 000)
        {
                /* spawn log process */
                if ((pthread_create(&(log_tid), NULL, Stats, (void *)0)) < 0)
                {
                        perror("pthread_create()");
                }
                int32_t i = 0;
                /* while not done */
                while (!quit)
                {
                        i = 0;
                        /* check all necessary testing params */
                        while (i < testing)
                        {
                                //cerr << "testMgr, i = " << i << ", testing = " << testing << endl << flush;
                                //cerr << "flag = " << ((ptest_param)v)[i].flag << endl << flush;
                                if (((ptest_param)v)[i].flag == START)
                                {
                                        ((ptest_param)v)[i].flag = WORKING;
                                        /* begin a test, pass (ptest_param)&((test_param *)v[i]), i.e., a ptest_param */
                                        /* HERE - exit? if unable to spawn testing threads */
                                        if ((pthread_create(&(((ptest_param)v)[i].tid), NULL, testThread, (void *)(&(((ptest_param)v)[i])))) < 0)
                                        {
                                                perror("pthread_create()");
                                        }
                                        /*
                                           else
                                           {
                                           if (DEBUG)
                                           {
                                           fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_create succeeded, starting testing on a test_param struct\n");
                                           fflush(stderr);
                                           }
                                           }
                                           */
                                }
                                else if (((ptest_param)v)[i].flag == DONE)
                                {
                                        /* HERE - exit? if unable to return from testing threads */
                                        if ((pthread_join((((ptest_param)v)[i].tid), NULL)) < 0)
                                        {
                                                perror("pthread_join()");
                                        }
                                        /*
                                           else
                                           {
                                           if (DEBUG)
                                           {
                                           fprintf(stderr, "\n\t\t[r] --- testMgr: pthread_join succeeded, done testing on a test_param\n");
                                           fflush(stderr);
                                           }
                                           }
                                           */
                                        //cerr << "about to decrement testing, testing = " << testing << "...\n" << flush;
                                        testing--;
                                        //cerr << "testing decremented, testing = " << testing << "\n" << flush;
                                        ((ptest_param)v)[i].flag = COMPLETE;
                                }
                                /*
                                   else if (((ptest_param)v)[i].flag == COMPLETE)
                                   {
                                   }
                                   else
                                   {
                                   cerr << "\n\t\t[M] --- Thread " << i << " incomplete, flag = " << ((ptest_param)v)[i].flag << flush;
                                   }
                                   */
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

//void pull(Antibody ** pop, const int32_t pipefd)
void pull(Antibody ** pop)
{
        if (pop == NULL)
        {
                /* attempt to pull a population from file */
                /* maybe fork and exec breed and train module automatically, regularly */
                if ((champs = importChamps()) == 0)
                {
                        cerr << "Unable to get antibodies to test against\nMaybe try running lifetime first?\nQuitting\n" << flush;
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

        uint32_t i = 0, j = 0;

        i = 0;
        pthread_mutex_init(&champs_mutex, NULL);
        pthread_mutex_init(&log_mutex, NULL);
        char * n = (char *)"./traffic.log\0";

        /* open log file */
        fout.open(n);
        if (fout.fail())
        {
                if (DEBUG)
                {
                        cerr << "Unable to open file for logging traffic results\n" << flush;
                }
        }

        test_param params[MAX_THREADS];
        for (i = 0; i < MAX_THREADS; i++)
        {
                params[i].flag = UNUSED;
                params[i].tnum = i;
                //                params[i].debug_buf = new string;
        }
        /* start testMgr with &params[] */
        /* HERE - cleanup and exit */
        if ((pthread_create(&(test_mgr_tid), NULL, testMgr, (void *)(&(params)))) < 0)
        {
                perror("pthread_create()");
                /* HERE */
                cleanup();
        }
        testing = 0;
        if ((pthread_create(&(import_mgr_tid), NULL, importManager, (void *)(0))) < 0)
        {
                perror("pthread_create()");
                cerr << "Unable to start importManager\n";
                import_mgr_tid = 0;
        }

        else
        {
                /* setup champs update signal */
                signal(SIGUSR1, &onDemandImport);
        }
        /*
           if (DEBUG)
           {
        // Retrieve
        fprintf(stderr, "\tentering loop\n");
        }
        */

        bool started = false;

        /* accept signal from producer to quit */
        while (shm[CTL][FLAGS] != PDONE && shm[CTL][FLAGS] != CDONE)
        {
                /* only copy when dhs isn't writing and while there are pending headers to get
                */
                while (shm[CTL][PEND] > 0)
                {
                        if (shm[CTL][FLAGS] == PWTEN || shm[CTL][FLAGS] == CREAD)
                        {
                                /*
                                   if (DEBUG)
                                   {
                                // Retrieve
                                fprintf(stderr, "\tshm[CTL][FLAGS] == PWTEN\n");
                                }
                                */
                                shm[CTL][FLAGS] = CRING;
                                memcpy((void *)retrieved_sigs[ct], (void *)shm[(shm[CTL][POS] == 1 ? (SIGQTY) : (shm[CTL][POS] - 1))], sizeof(sig_atomic_t) * fngPntLen);
                                memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS] == 1 ? (SIGQTY - 1) : (shm[CTL][POS] - 2))], sizeof(sig_atomic_t) * t5TplLen);
                                if ((testing) < MAX_THREADS)
                                {
                                        started = false;
                                        j = 0;
                                        while (j <= MAX_THREADS && !started)
                                        {
                                                if (params[j].flag != START && params[j].flag != WORKING && params[j].flag != DONE && params[j].flag != LOG)
                                                {
                                                        memcpy((void *)&(params[j].sig), (void *)retrieved_sigs[ct], sizeof(sig_atomic_t) * fngPntLen);
                                                        /* convert sig_atomic_t to char */
                                                        Convert(params[j].tuple, retrieved_t5s[ct]);
                                                        /* these two lines signal the testMgr to spawn a testing thread */
                                                        testing++;
                                                        params[j].flag = START;
                                                        started = true;
                                                }
                                                j++;
                                        }
                                        if (!started)
                                        {
                                                fprintf(stderr, "Insufficient threads to process incoming signatures, testing = %d\n", testing);
                                                fflush(stderr);
                                        }
                                }
                                else
                                {
                                        fprintf(stderr, "Insufficient threads to process incoming signatures, testing = %d\n", testing);
                                        fflush(stderr);
                                }
                                /*
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
                                   */
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
        if (shm != 000)
        {
                shm[CTL][FLAGS] = CDONE;
        }
        /* join testMgr. since fork/logging is not a priority, make sure there is actually a child process before waiting for child process */
        if (import_mgr_tid != 0)
        {
                if ((pthread_join(import_mgr_tid, NULL)) < 0)
                {
                        perror("pthread_join()");
                }
        }
        if ((pthread_join(test_mgr_tid, NULL)) < 0)
        {
                perror("pthread_join()");
        }
        for (i = 0; i < MAX_THREADS; i++)
        {
                //                delete(params[i].debug_buf);
        }

        cleanup();

        exit(EXIT_SUCCESS);

        /* dummy return */
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

