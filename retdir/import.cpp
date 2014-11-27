#include <import.hpp>

using namespace std;

void onDemandImport (int sign)
{
        /* stop accepting new signals to import while involved in an import */
        signal(SIGUSR1, &onDemandImport);
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
        /* wait for import operation to finish */
        while (do_import != 0);
        /* accept new signals */
        signal(SIGUSR1, &onDemandImport);
}

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

Antibody ** importChamps (char * fin)
{
        if (fin == (char *)0)
        {
                fin = (char *)"./ais/champions.abs\0";
        }
        cerr << "importChamps: filename = " << fin << endl << flush;
        ifstream in(fin);
        if (in.fail())
        {
                cerr << "Unable to open champs file\n" << flush;
                return (0);
        }
        char * data = 0;
        data = (char *)malloc(sizeof(char) * arr_size);
        if (!data)
        {
                return (0);
        }
        int i = 0;
        alen = 0;
        class_count = 0;
        /* maybe check read/calculated values below against what they should be? */
        while (in.get(data[i]))
        {
                cerr << data[i] << flush;
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
        cerr << endl << flush;
        in.close();
        if (DEBUG)
        {
                cerr << "In import function\n";
                cerr << "Got alen: " << alen << endl;
                cerr << "Got class_count: " << class_count << endl;
        }
        ab_count /= class_count;
        const int data_size = strlen(data);
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
                        while (data[l] != 10)
                        {
                                l++;
                        }
                        l++;
                        j++;
                }
                i++;
        }
        cerr << endl << flush;

        /* Excessive output
        */
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

        return (pop);
}

void * importManager (void * v)
{
        if (DEBUG)
        {
                cerr << "start import manager\n" << flush;
        }

        char * fname = (char *)v;
        while (!quit)
        {
                if (do_import == 1)
                {
                        if (DEBUG)
                        {
                                cerr << "import initiated!\n" << flush;
                        }
                        /* lock access to champs */
                        pthread_mutex_lock(&champs_mutex);
                        champs = importChamps(fname);
                        /* log import success/failure */

                        if (DEBUG)
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
                        /* unlock access to champs */
                        pthread_mutex_unlock(&champs_mutex);
                        do_import = 0;
                }
        }

        return ((void *)0);
}
