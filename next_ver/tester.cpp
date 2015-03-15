#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <fstream>
#include <curl/curl.h>

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
        string filename = NORMALFILE;
        /*

        if (argc >= 2)
        {
                filename = argv[1];
                in_file.open(filename.c_str());
                if (in_file.fail())
                {
                        filename = NORMALFILE;
                }
        }
        in_file.open(filename.c_str());
        if (in_file.fail())
        {
                cerr << "Unable to open file for reading" << endl << flush;
                exit(EXIT_FAILURE);
        }

        cerr << "Opened file with filename " << filename << endl << flush;

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
                //uris[i].append("\"");

                cerr << "Command " << i+1 << ": " << uris[i] << endl << flush;
                uris[i].append(" &");
                uris[i].insert(0, "iceweasel ");
                system(uris[i].c_str());
                fib(30);
        }
        */
        if (argc >= 3)
        {
                filename = argv[2];
                in_file.open(filename.c_str());
                if (in_file.fail())
                {
                        filename = ATTACKFILE;
                }
        }
        else
        {
                filename = ATTACKFILE;
        }
        in_file.open(filename.c_str());
        if (in_file.fail())
        {
                cerr << "Unable to open file for reading" << endl << flush;
                exit(EXIT_FAILURE);
        }

        cerr << "Opened file with filename " << filename << endl << flush;
        if (getUris(in_file, uris, uri_qty) == 000)
        {
                cerr << "Couldn't get uris from file\n" << flush;
                exit(EXIT_FAILURE);
        }
        in_file.close();
        int ret_pid[uri_qty];
        for (i = 0; i < uri_qty; i++)
        {
                if (uris[i][0] != '/')
                {
                        uris[i].insert(0, "/");
                }
                uris[i].insert(0, TARGET);
                //uris[i].append("\"");
                cerr << "Command " << i+1 << ": " << uris[i] << endl << flush;
                uris[i].append(" &");
                uris[i].insert(0, "iceweasel ");
                /* HERE - fork and execve */
                /* child */
                if ((ret_pid[i] = fork()) == 0) /* child */
                {
                        system(uris[i].c_str());
                }
                else /* parent */
                {
                        fib(40);
                        kill (ret_pid[i], SIGKILL);
                }
        }

        return (0);
}

