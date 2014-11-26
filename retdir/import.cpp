#include <import.hpp>

using namespace std;

void onDemandImport (int sign)
{
        signal(SIGUSR1, &onDemandImport);
        if (DEBUG)
        {
                cout << "recieved import signal!\n" << flush;
        }
        if (sign != SIGUSR1)
        {
                if (DEBUG)
                {
                        cout << "onDemandImport signaled by invalid signal!\n" << flush;
                }
                return;
        }
        do_import = 1;
        while (do_import != 0);
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
        cout << "importChamps: filename = " << fin << endl << flush;
        ifstream in(fin);
        if (in.fail())
        {
                cout << "Unable to open champs file\n" << flush;
                return (0);
        }
        char * data = 0;
        data = (char *)malloc(sizeof(char) * arr_size);
        if (!data)
        {
                return (0);
        }
        unsigned int i = 0;
        alen = 0;
        class_count = 0;
        /* maybe check read/calculated values below against what they should be? */
        while (in.get(data[i]))
        {
                cout << data[i] << flush;
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
                if (i == arr_size - 1)
                {
                        resizeChar(&data, &arr_size);
                }
        }
        cout << endl << flush;
        in.close();
        if (DEBUG)
        {
                cout << "In import function\n";
                cout << "Got alen: " << alen << endl;
                cout << "Got class_count: " << class_count << endl;
        }
        ab_count /= class_count;
        const unsigned int data_size = strlen(data);
        if (DEBUG)
        {
                cout << "Got ab_count: " << ab_count << endl;
                cout << "Got data_size: " << data_size << endl;
        }

        Antibody ** pop = 0;
        pop = new Antibody *[class_count];
        for (int k = 0; k < class_count; k++)
        {
                pop[k] = new Antibody [ab_count];
        }

        unsigned int j = 0, k = 0, l = 0, tmp = 0, begin = 0;
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
                                l++;
                                tmp = atoi(&data[begin]);
                                pop[i][j].setFlag(k, tmp);
                                begin = l;
                                while (data[l] != 32)
                                {
                                        l++;
                                }
                                l++;
                                tmp = atoi(&data[begin]);
                                pop[i][j].setAttr(k, tmp);
                                begin = l;
                                while (data[l] != 32)
                                {
                                        l++;
                                }
                                l++;
                                tmp = atoi(&data[begin]);
                                pop[i][j].setOffset(k, tmp);
                                begin = l;
                                while (data[l] != 32)
                                {
                                        l++;
                                }
                                l++;
                                tmp = atoi(&data[begin]);
                                pop[i][j].setMax(k, tmp);
                                /* comma space */
                                while (data[l] != 44)
                                {
                                        l++;
                                }
                                while (data[l] != 32)
                                {
                                        l++;
                                }
                                k++;
                        }
                        /* tab to stats */
                        while (data[l] != 9)
                        {
                                l++;
                        }
                        l++;
                        begin = l;
                        while (data[l] != 32)
                        {
                                l++;
                        }
                        l++;
                        tmp = atoi(&data[begin]);
                        pop[i][j].setTests(tmp);
                        begin = l;
                        while (data[l] != 32)
                        {
                                l++;
                        }
                        l++;
                        tmp = atoi(&data[begin]);
                        pop[i][j].setPos(tmp);
                        begin = l;
                        while (data[l] != 32)
                        {
                                l++;
                        }
                        l++;
                        tmp = atoi(&data[begin]);
                        pop[i][j].setFalsePos(tmp);
                        begin = l;
                        while (data[l] != 32)
                        {
                                l++;
                        }
                        l++;
                        tmp = atoi(&data[begin]);
                        pop[i][j].setNeg(tmp);
                        begin = l;
                        while (data[l] != 32)
                        {
                                l++;
                        }
                        l++;
                        tmp = atoi(&data[begin]);
                        pop[i][j].setFalseNeg(tmp);
                        while (data[l] != 9)
                        {
                                l++;
                        }
                        l++;
                        k = 0;
                        while (k < class_count)
                        {
                                begin = l;
                                tmp = atoi(&data[begin]);
                                while (data[l] != 32)
                                {
                                        l++;
                                }
                                l++;
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

        /* Excessive output
        */
        cout << "\n\nDone importing. Got the following:\n";
        for (int i = 0; i < class_count; i++)
        {
                cout << "Class " << i+1 << ":\n";
                for (int j = 0; j < ab_count; j++)
                {
                        cout << "\t" << pop[i][j] << "\n";
                }
                cout << endl;
        }
        cout << endl << endl << flush;

        return (pop);
}

void * importManager (void * v)
{
        if (DEBUG)
        {
                cout << "start import manager\n" << flush;
        }

        char * fname = (char *)v;
        while (!quit)
        {
                if (do_import == 1)
                {
                        if (DEBUG)
                        {
                                cout << "import initiated!\n" << flush;
                        }
                        /* lock access to champs */
                        pthread_mutex_lock(&champs_mutex);
                        champs = importChamps(fname);
                        /* log import success/failure */

                        if (DEBUG)
                        {
                                cout << "In onDemandImport. Got the following:\n";
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
                        /* unlock access to champs */
                        pthread_mutex_unlock(&champs_mutex);
                        do_import = 0;
                }
        }

        return ((void *)0);
}
