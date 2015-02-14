#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fstream>

#include "tester.hpp"

using namespace std;

inline static bool resize(string *& s, int &l)
{
        if (l == 0)
        {
                l = 100;
        }
        string * tmp = 000;
        try
        {
                tmp = new string [l*2];
        }
        catch(...)
        {
                return(false);
        }
        if (s != 000)
        {
                for (int i = 0; i < l; i++)
                {
                        tmp[i] = s[i];;
                }
        }
        s = tmp;
        l <<= 1;
        return (true);
}

string * getUris(istream & in_file, string *& data, int &uri_size)
{
        int uriQty = 0;
        //uri_size = 0;
        if (in_file.fail())
        {
                data = NULL;
                return (NULL);
        }
        if (data == 000)
        {
                if (!(resize(data, uri_size)))
                {
                        return (NULL);
                }
        }
        getline(in_file, (data[uriQty]));
        do
        {
                data[uriQty] = strtok((char *)(data[uriQty].c_str()), (char *)" \t\n\r");
                uriQty++;
                if (uriQty >= uri_size)
                {
                        if (!(resize(data, uri_size)))
                        {
                                return(NULL);
                        }
                }
                getline(in_file, (data[uriQty]));
        }
        while (in_file.good() && !in_file.eof());
        uri_size = uriQty;
        return (data);
}

int fib (int n)
{
        return(n == 0 ? 1 : (n == 1 ? 1 : (fib(n-1) + fib(n-2))));
}

int main (int argc, char *argv[], char *envp[])
{
        if (envp != 000)
        {
                envp = 000;
        }

        ifstream in_file;
        string filename = INFILE;

        if (argc >= 2)
        {
                filename = argv[1];
                in_file.open(filename.c_str());
                if (in_file.fail())
                {
                        filename = INFILE;
                }
        }
        in_file.open(filename.c_str());
        if (in_file.fail())
        {
                cerr << "Unable to open file for reading" << endl << flush;
                exit(EXIT_FAILURE);
        }

        cout << "Opened file with filename " << filename << endl << flush;

        string * uris = 000;
        int uri_qty = 0;
        int i = 0;
        if (getUris(in_file, uris, uri_qty) == 000)
        {
                cerr << "Couldn't get uris from file\n" << flush;
                exit(EXIT_FAILURE);
        }
        in_file.close();
        for (i = 0; i < uri_qty; i++)
        {
                if (uris[i][0] != '/')
                {
                        uris[i].insert(0, "/");
                }
                uris[i].insert(0, TARGET);
                uris[i].insert(0, CMD);
                uris[i].append("\" 2>> cmd_errs.log &");

                cerr << "Command " << i+1 << ": " << uris[i] << endl << flush;
                system(uris[i].c_str());
                fib(35);
                system(KILL);
        }

        return (0);
}

