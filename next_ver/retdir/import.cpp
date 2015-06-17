/* TO DO
 *
 * confirm data is properly formatted when reading from file
 */

#include <import.hpp>

using namespace std;

struct stat buf;

/**
 * @brief Function signaled by SIGUSR1 to initiate an import operation.
 */
void onDemandRebreed (int sign)
{
	if (DEBUG)
	{
		cerr << "recieved rebreed signal!\n" << flush;
	}
	if (sign != SIGUSR2)
	{
		if (DEBUG)
		{
			cerr << "onDemandRebreed signaled by invalid signal!\n" << flush;
		}
		return;
	}
	do_import = 2;
}

/**
 * @brief Function signaled by SIGUSR1 to initiate an import operation.
 */
void onDemandImport (int sign)
{
	if (DEBUG)
	{
		cerr << "recieved import signal!\n" << flush;
	}
	if (sign != SIGUSR1)
	{
		if (DEBUG)
		{
			cerr << "onDemandImport signaled by invalid signal!\n" << flush;
		}
		return;
	}
	do_import = 1;
}

/**
 * @brief Generic resize function for character arrays.
 */
inline static void resizeChar(char ** a = 0, unsigned int * s = 0)
{
	if (a == 0)
	{
		return;
	}
	int32_t size = strlen(*a);
	char * tmp = (char *)malloc(sizeof(char) * size * 2);
	memcpy(tmp, *a, size);
	free(*a);
	*a = tmp;
	if (s != 0)
	{
		*s = (size * 2);
	}
}

unsigned int arr_size = 100;
static bool checked = false;

/**
 * @brief Checks for the existence of a file.
 */
inline static bool Exists (const char * n)
{
	return((bool)(stat(n, &buf) == 0));
}
Antibody ** t = 0;

/**
 * @brief Imports champions antibody pool from champions.abs or another specified file. If no such file exists, this function tries to run the lifetime.25 binary to generate a champions pool
 */
Antibody ** importChamps (char * fin)
{
	/* import */
	ifstream in;
	if (do_import == 1 || do_import == 0)
	{
		if (fin == (char *)0)
		{
			fin = (char *)"./ais/champions.abs\0";
		}
		if (DEBUG)
		{
			cerr << "importChamps: filename = " << fin << endl << flush;
		}
		in.open(fin);
		if (in.fail())
		{
			fin = (char *)"./../ais/champions.abs\0";
			if (DEBUG)
			{
				cerr << "importChamps: filename = " << fin << endl << flush;
			}
			in.open(fin);
			if (in.fail())
			{
				if (DEBUG)
				{
					cerr << "Unable to open champs file\nAttempting to generate...\n" << flush;
				}
				/* HERE */
				if (!checked)
				{
					system(Exists((const char *)("./../ais/lifetime.25\0")) ? (char *)("cd ./../ais/ && ./lifetime.25 && cd ../retdir/\0") : (Exists((const char *)("./ais/lifetime.25\0")) ? (char *)("cd ./ais/ && ./lifetime.25 && cd ../\0") : (char *)("")));
					checked = true;

					t = importChamps((char *)(0));
					checked = false;
					return (t);
				}
				else
				{
					if (DEBUG)
					{
						cerr << "Unable to generate champs file. Aborting.\n" << flush;
					}
					checked = false;
					return (t);
				}
			}
		}
	}
	/* rebreed */
	else if (do_import == 2)
	{
		if (!checked)
		{
			system(Exists((const char *)("./../ais/lifetime.25\0")) ? (char *)("cd ./../ais/ && ./lifetime.25 -e Champions.abs && cd ../retdir/\0") : (Exists((const char *)("./ais/lifetime.25\0")) ? (char *)("cd ./ais/ && ./lifetime.25 -e Champions.abs && cd ../\0") : (char *)("")));
			checked = true;
			/* just do import after rebreed op */
			do_import = 1;

			t = importChamps((char *)(0));
			if (t)
			{
				checked = false;
			}
			return (t);
		}
		else
		{
			if (DEBUG)
			{
				cerr << "Unable to rebreed champs file. Aborting.\n" << flush;
			}
			checked = false;
			return (t);
		}
	}

	/* ok, do the import operation */
	char * data = 0;
	data = (char *)malloc(sizeof(char) * arr_size);
	if (!data)
	{
		return (0);
	}
	int i = 0;
	ab_count = 0;
	alen = 0;
	class_count = 0;
	/* maybe check read/calculated values below against what they should be? */
	while (in.get(data[i]))
	{
		if (DEBUG)
		{
			cerr << data[i] << flush;
		}
		if (ab_count == 0)
		{
			if (data[i] == ',')
			{
				alen++;
			}
			else if (data[i] == ';')
			{
				class_count++;
			}
		}
		if (data[i] == '\n')
		{
			ab_count++;
		}
		i++;
		if ((unsigned int)i == arr_size - 1)
		{
			resizeChar(&data, &arr_size);
		}
	}
	if (DEBUG)
	{
		cerr << endl << flush;
	}
	in.close();
	if (DEBUG)
	{
		/* we use the following values for the entire module */
		cerr << "Got alen: " << alen << endl;
		cerr << "Got class_count: " << class_count << endl;
	}
	/* HERE
	 * check for zero class_count, return failure */
	ab_count /= class_count;
	int data_size = strlen(data);
	if (DEBUG)
	{
		cerr << "Got ab_count: " << ab_count << endl;
		cerr << "Got data_size: " << data_size << endl << flush;
	}

	Antibody ** pop = 0;
	pop = new Antibody *[class_count];
	for (int k = 0; k < class_count; k++)
	{
		pop[k] = new Antibody [ab_count];
	}

	int j = 0, k = 0, l = 0, tmp = 0, begin = 0;
	i = 0;

	/* read from file. assume good data. */
	while (i < class_count && l < data_size)
	{
		j = 0;
		while (j < ab_count && l < data_size)
		{
			k = 0;
			/* first val:   flag[i] (i < alen) */
			/* second val:  a[i] - attribute */
			/* third val:   offset[i] */
			/* fourth val:  max[i] */
			/* comma separates value set, iterate iterator */
			while (k < alen && l < data_size)
			{
				begin = l;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				pop[i][j].setFlag(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				pop[i][j].setAttr(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				pop[i][j].setOffset(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				pop[i][j].setMax(k, tmp);
				while (data[l] != 32)
				{
					l++;
				}
				l++;
				k++;
			}
			/* tab to stats */
			while (data[l] != 9)
			{
				l++;
			}
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			pop[i][j].setTests(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			pop[i][j].setPos(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			pop[i][j].setFalsePos(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			pop[i][j].setNeg(tmp);
			begin = l;
			tmp = atoi(&data[begin]);
			pop[i][j].setFalseNeg(tmp);
			while (data[l] != 9)
			{
				l++;
			}
			k = 0;
			while (k < class_count)
			{
				begin = l;
				tmp = atoi(&data[begin]);
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				pop[i][j].setCat(k, tmp);
				k++;
			}
			/* use a tab to delimit totals */
			while (data[l] != 9)
			{
				l++;
			}
			k = 0;
			while (k < class_count)
			{
				begin = l;
				tmp = atoi(&data[begin]);
				l++;
				/* spaces between values */
				while (data[l] != 32)
				{
					l++;
				}
				if (DEBUG)
				{
					cerr << "Precalc percent: " << pop[i][j].getCatPerc(k) << endl << flush;
					cerr << "Got total count: " << tmp << endl << flush;
				}
				pop[i][j].setCatTotal(k, tmp);
				/* figure percentages */
				pop[i][j].calcCategory(k, tmp);
				if (DEBUG)
				{
					cerr << "Postcalc percent: " << pop[i][j].getCatPerc(k) << endl << endl << flush;
				}
				k++;
			}
			/* newline, end of line */
			while (data[l] != 10)
			{
				l++;
			}
			l++;
			j++;
		}
		i++;
	}


	if (DEBUG)
	{
		cerr << endl << flush;
	}

	/* Excessive output for debugging
	*/
	//if (DEBUG)
	{
		fprintf(stderr, "\n\nDone importing. Got the following:\n");
		for (int i = 0; i < class_count; i++)
		{
			fprintf(stderr, "Class %d :\n", i + 1);
			for (int j = 0; j < ab_count; j++)
			{
				cerr << "\t" << pop[i][j] << "\n";
			}
			cerr << endl;
		}
		cerr << endl << endl << flush;
	}

	return (pop);
}

/**
 * @brief Thread function that continually checks for quit condition and updates champions pool on request (SIGUSR1)
 */
void * importManager (void * v)
{
	if (DEBUG)
	{
		cerr << "start import manager\n" << flush;
	}

	char * fname = (char *)v;
	while (!quit)
	{
		if (do_import != 0)
		{
			if (DEBUG)
			{
				switch (do_import)
				{
					case 1:
						cerr << "import initiated!\n" << flush;
						break;
					case 2:
						cerr << "rebreed initiated!\n" << flush;
						break;
					default:
						cerr << "Unsupported signal\n" << flush;
				}
			}
			if (do_import != 1 && do_import != 2)
			{
				return (void *)0;
			}
			/* lock access to champs */
			/* In reference to the optimal use of time, I chose not to save the new, imported antibody pool in a temp var
			 * and then lock the champs mutex and copy the new abs over, as
			 *    1. Upon signaling of an import operation, we only want to use the new abs for all future testing, including any currently queued up; and
			 *    2. The design of pull allows for SOME backup. This parameter is manually expandable in a header somewhere, probably retglobals
			 */
			pthread_mutex_lock(&champs_mutex);
			champs = importChamps(fname);
			t = 0;
			/* log import success/failure */

			if (DEBUG)
			{
				if (champs)
				{
					cerr << "In onDemandImport. Got the following:\n";
					for (int i = 0; i < class_count; i++)
					{
						cerr << "Class " << i+1 << ":\n";
						for (int j = 0; j < ab_count; j++)
						{
							cerr << "\t" << champs[i][j] << "\n";
						}
						cerr << endl;
					}
					cerr << endl << flush;
				}
				else
				{
					cerr << "In onDemandImport, got champs == 000 from importChamps with filename " << (fname != 000 ? fname : "(NULL)") << endl << flush;
				}
			}
			/* unlock access to champs */
			pthread_mutex_unlock(&champs_mutex);
			do_import = 0;
		}
	}

	return ((void *)0);
}
