#include <pull.hpp>

using namespace std;

int status = 0;
int32_t import_mgr_pid = -1;

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

/* sig_atomic_t ~= signed int */
volatile static sig_atomic_t testing = 0;
volatile sig_atomic_t alen = 0;
volatile sig_atomic_t class_count = 0;
volatile sig_atomic_t ab_count = 0;

/**
 * @brief Testing data structure.
 */
typedef struct _test_param
{
	uint8_t tnum; // thread number
	pthread_t tid; // thread id
	uint32_t sig[fngPntLen]; // signature to be tested
	char tuple[t5TplLen]; // t5 tuple
	volatile sig_atomic_t flag; // signal to thread to start testing
	int32_t attack; // whether or not the tested signature was determined to be an attack
	int32_t attack_count; // how many antibodies returned attack
	char uri[MAXURI]; // what uri generated this signature
} test_param, *ptest_param;

queue<test_param> log_queue;

/**
 * @brief Function to increment our local position tracking variable in sync with the quantity of signatures defined in retglobals.
 */
inline static void inPos(void)
{
	((local_pos))++;
	((local_pos)) = ((((local_pos)) % SIGQTY) != 0 ? (((local_pos)) % SIGQTY) : SIGQTY); // 1 - 5
}

/**
 * @brief Performs match operation for each antibody in the champions pool. Returns CLASS_COUNT+1 for not attack, otherwise integer representing class of attack identified.
 */
int doMatch(Antibody a, int *test)
{
	a.incTests();
	if(a.getFlag(a.COMMAND) && !(a.getAttr(a.COMMAND) & test[a.COMMAND]))
	{
		return (class_count + 1);
	}
	if(a.getFlag(a.PROTOCOL) && !(a.getAttr(a.PROTOCOL) & test[a.PROTOCOL]))
	{
		return (class_count + 1);
	}
	for(int i = a.LENGTH; i < ALEN; i++)
	{
		if(!a.getFlag(i))
		{
			continue;
		}
		int overf = (int)pow(2.0, (double)a.getMax(i)) + (int)pow(2.0, (double)a.getMax(i) - 1.0) - 2; // HERE - why is this the optimum calculation?
		cerr << "Testing with given antibody and attribute " << i << ".\n\tAntibody classification: " << a.queryClassification() << ", test[" << i << "] = " << test[i] << ", testing if greater than overf, " << overf << endl << flush;
		if (test[i] > overf)
		{
			cerr << "Actually signaling attack, classification of AB: " << a.queryClassification() << endl << flush;
			return (a.queryClassification());  // Over max antibody match, attack
		}
		cerr << "Not an attack, is it normal?.\n\t" << "Testing test[" << i << "], " << test[i] << ", < ( a.getAttr(" << i << "), " << a.getAttr(i) << ", - a.getOff(" << i << "), " << a.getOff(i) << " (" << (a.getAttr(i) - a.getOff(i)) << ") ) OR > ( a.getAttr(" << i << "), " << a.getAttr(i) << ", + a.getOff(" << i << "), " << a.getOff(i) << " (" << (a.getAttr(i) + a.getOff(i)) << " )" << endl << flush;
		if (test[i] < (a.getAttr(i) - a.getOff(i)) || test[i] > (a.getAttr(i) + a.getOff(i)))
		{
			cerr << "True! Returning Normal traffic\n" << flush;
			return (class_count + 1);
		}
	}
	cerr << "returning default case\n" << flush;
	return (class_count + 1); // normal traffic
	return (class_count); // unknown attack - default case
}

/**
 * @brief Thread function to initiate testing and logging. Ensures mutual exclusion on champions pool. Takes a ptest_param parameter. Currently ignores fitness and tests on all antibodies regardless.
 */
void * testThread(void * v)
{
	if (v != 000)
	{
		int tmp = 0, max = 0, c = 0;
		int attacks[class_count + 2];
		for (int i = 0; i < class_count + 2; i++)
		{
			attacks[i] = 0;
		}
		((ptest_param)v)->attack = 0;
		((ptest_param)v)->attack_count = 0;

		pthread_mutex_lock(&champs_mutex);

		for (int i = 0; i < class_count; i++)
		{
			for (int j = 0; j < ab_count; j++)
			{
				//if ((champs)[i][j].fitness() > MIN_FITNESS)
				{
					c = champs[i][j].queryClassification();
					cerr << "Testing with a(n) " << ((c >= 0 && c < class_count) ? CLASS_LABELS[c] : "Unknown Type") << " antibody" << flush;
					cerr << " with fitness: " << champs[i][j].fitness() << endl << flush;
					/*
					   cerr << "Category Percentages and Counts:\n";
					   for (int k = 0; k < class_count; k++)
					   {
					   cerr << "\t" << k << ": " << champs[i][j].getCatCount(k) << " / " << champs[i][j].getCatTotal(k) << " = " << champs[i][j].getCatPerc(k) << endl << flush;
					   }
					   cerr << endl << flush;
					   */
					tmp = (doMatch( (champs[i][j]) , (int *)(((ptest_param)v)->sig) ));
					cerr << "\tReturned " << tmp << ", associated with a " << CLASS_LABELS[tmp] << " attack." << endl << flush;
					/* class_count + 1 == normal traffic */
					if (tmp != class_count + 1)
					{
						((ptest_param)v)->attack_count += 1;
					}
					attacks[tmp] += 1;
				}
			}
		}

		pthread_mutex_unlock(&champs_mutex);

		/* calculating type of attack. Determined by greatest number of antibodies returning a particular class of attack. */
		tmp = attacks[0];
		for (int i = 1; i < class_count + 1; i++)
		{
			if (tmp < attacks[i])
			{
				tmp = attacks[i];
				max = i;
			}
		}
		((ptest_param)v)->attack = max;

		fprintf(stderr, "\n\t\t[T] --- testThread: Something to log\n");
		fflush(stderr);
		/* add newest test result to logging queue */
		pthread_mutex_lock(&log_mutex);
		log_queue.push(*(ptest_param)(v));
		pthread_mutex_unlock(&log_mutex);
		/* signal completion of testing */
		((ptest_param)v)->flag = DONE;
	}
	else
	{
		cerr << "\n\t\t[T] --- Nothing passed to testThread\n" << flush;
	}

	return ((void *)0);
}

/**
 * @brief Copies a test_param structure
 */
inline static void Copy(test_param & d, test_param s)
{
	d.attack = s.attack;
	d.attack_count = s.attack_count;
	unsigned int x = 0;
	for (x = 0; x < t5TplLen; x++)
	{
		d.tuple[x] = s.tuple[x];
	}
	for (x = 0; x < fngPntLen; x++)
	{
		d.sig[x] = s.sig[x];
	}
	strncpy(d.uri, s.uri, MAXURI-1);
	d.uri[MAXURI-1] = '\0';
}

/**
 * @brief Thread function to perform logging operation.
 */
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
			/* log new item to queue */
			Copy(tmp, log_queue.front());
			log_queue.pop();
			if (!fout.is_open())
			{
				fout.open("./traffic.log\0", ios::app);
			}
			if (tmp.attack_count >= AGREE)
			{
				fout << "[A] --- Attack Identified --- [A]" << endl;
				fout << "\tURI: " << tmp.uri << endl;
				fout << "\t" << "Unique Tuple:    " << tmp.tuple << endl;
				fout << "\t" << "HTTP Signature:  \t";
				for (unsigned int i = 0; i < fngPntLen; i++)
				{
					fout << tmp.sig[i] << " ";
				}
				fout << "\n\tNumber of Antibodies Signaling Attack: " << tmp.attack_count << endl;
				fout << "\tAttack Type: " << CLASS_LABELS[tmp.attack] << " (" << tmp.attack << ")" << endl;
			}
			else
			{
				fout << "[N] --- Normal Traffic --- [N]" << endl;
				fout << "\tURI: " << tmp.uri << endl;
				fout << "\t" << "Unique Tuple:    " << tmp.tuple << endl;
				fout << "\t" << "HTTP Signature:  \t";
				for (unsigned int i = 0; i < fngPntLen; i++)
				{
					fout << tmp.sig[i] << " ";
				}
				fout << "\n\tNumber of Antibodies Signaling Attack: " << tmp.attack_count << endl;
			}
			fout << endl;
			if (fout.is_open())
			{
				fout.close();
			}
		}
		pthread_mutex_unlock(&log_mutex);
	}
	/* HERE - on quit, dump overall statistics to log file */
	return ((void *)0);
}

/**
 * @brief Function to convert the volatile sig_atomic shared memory data into a character array.
 */
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
/**
 * @brief Thread function to manage testing threads. Spawns a new testing thread when new data is retrieved from shared memory. Tracks progress of testing of individual signatures and joins completed testing routines. Initiates logging thread. Takes an array of test_param objects.
 */
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
				if (((ptest_param)v)[i].flag == START)
				{
					((ptest_param)v)[i].flag = WORKING;
					/* begin a test, pass (ptest_param)&((test_param *)v[i]), i.e., a ptest_param */
					/* HERE - exit if unable to spawn testing threads? */
					if ((pthread_create(&(((ptest_param)v)[i].tid), NULL, testThread, (void *)(&(((ptest_param)v)[i])))) < 0)
					{
						perror("pthread_create()");
					}
				}
				else if (((ptest_param)v)[i].flag == DONE)
				{
					/* HERE - exit if unable to return from testing threads? */
					if ((pthread_join((((ptest_param)v)[i].tid), NULL)) < 0)
					{
						perror("pthread_join()");
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
		/* wait for logging to complete */
		if ((pthread_join(log_tid, NULL)) < 0)
		{
			perror("pthread_join()");
		}
	}
	return ((void *)0);
}

/**
 * @brief Main operation. Init memory, mutexes, and begin testmanager. Accept a champions pool as a parameter, but default to importing one.
 */
void pull(void)
{
	/* attempt to pull a population from file, or generate a new one */
	/* maybe fork and exec breed and train module automatically, regularly */
	if ((champs = importChamps()) == 0)
	{
		cerr << "Unable to get antibodies to test against\nMaybe try running lifetime first?\nQuitting\n" << flush;
		shm[CTL][FLAGS] = CDONE;
		return;
	}

	uint32_t i = 0, j = 0;

	i = 0;
	pthread_mutex_init(&champs_mutex, NULL);
	pthread_mutex_init(&log_mutex, NULL);
	char * n = (char *)"./traffic.log\0";

	/* open log file */
	fout.open(n, ios::app);
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
	}
	/* start testMgr with &params[] */
	if ((pthread_create(&(test_mgr_tid), NULL, testMgr, (void *)(&(params)))) < 0)
	{
		perror("pthread_create()");
		cleanup();
	}
	testing = 0;
	/* start import manager. on failure, continue anyway */
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
		signal(SIGUSR2, &onDemandRebreed);
	}

	bool started = false;

	fout << endl << "Start: " << __DATE__ << " : " << __TIME__ << endl;
	fout << "======================================================================" << endl << flush;
	fout.close();

	/* accept signal from producer to quit */
	while (shm[CTL][FLAGS] != PDONE && shm[CTL][FLAGS] != CDONE)
	{
		/* only copy when dhs isn't writing and while there are pending signatures to get */
		while (shm[CTL][PEND] > 0)
		{
			if (shm[CTL][FLAGS] == PWTEN || shm[CTL][FLAGS] == CREAD)
			{
				/* pull */
				shm[CTL][FLAGS] = CRING;
				/* get signature from shm segment */
				memcpy((void *)retrieved_sigs[ct], (void *)shm[(shm[CTL][POS] == 1 ? (SIGQTY) : (shm[CTL][POS] - 1))], sizeof(sig_atomic_t) * fngPntLen);
				/* get unique tuple from shm segment */
				memcpy((void *)retrieved_t5s[ct], (void *)t5shm[(shm[CTL][POS] == 1 ? (SIGQTY - 1) : (shm[CTL][POS] - 2))], sizeof(sig_atomic_t) * t5TplLen);
				/* get uri from shm segment. copy until max, though there should be a null terminator where ever the uri ends, dubious it will be longer than max */
				memcpy((void *)retrieved_uris[ct], (void *)(&(urishm[(shm[CTL][POS] == 1 ? (SIGQTY - 1) : (shm[CTL][POS] - 2))][0])), sizeof(char) * MAXURI);
				if ((testing) < MAX_THREADS)
				{
					started = false;
					j = 0;
					while (j <= MAX_THREADS && !started)
					{
						if (params[j].flag != START && params[j].flag != WORKING && params[j].flag != DONE && params[j].flag != LOG)
						{
							/* setup new test param object */
							memcpy((void *)&(params[j].sig), (void *)retrieved_sigs[ct], sizeof(sig_atomic_t) * fngPntLen);
							memcpy((void *)&(params[j].uri), (void *)retrieved_uris[ct], sizeof(char) * MAXURI);
							/* convert sig_atomic_t to char */
							Convert(params[j].tuple, retrieved_t5s[ct]);
							/* these two lines signal the testMgr to spawn a testing thread */
							testing++;
							params[j].flag = START;
							started = true;
						}
						j++;
					}
					/* optimize number of predefined threads */
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
				/* keep an accurate record of our buffer position and dhs's buffer position */
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
	/* join importmanager. since not a priority, make sure there is actually a child process before waiting for child process */
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

	cleanup();

	exit(EXIT_SUCCESS);

	/* dummy return, should never be used */
	return;
}

/**
 * @brief Cleanup some allocated memory and destory mutexes.
 */
void cleanup(void)
{
	delete champs;
	pthread_mutex_destroy(&champs_mutex);
	pthread_mutex_destroy(&log_mutex);
	fout.close();
}

