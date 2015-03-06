#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>

using namespace std;
inline static int percentNumberConverter(const char * s)
{
        if (s == 000 || s[0] != '%' || !isdigit(s[1]) || !isdigit(s[2]))
        {
                return (-1);
        }
        return (atoi(&(s[1])));
}


/**
 * @brief Create the firnger print from a HTTP header
 */
int * pfp(const unsigned char * curPcktData, const unsigned int dataLen)
{
        if(curPcktData == 0 || dataLen == 0)
        {
                return (0);
        }

        int i = 0;

        char * tmp_hdr = (char *)curPcktData;
        char * cmd_str;
        char * uri_str;
        char * ver_str;
        uri_str = strtok(tmp_hdr, (const char *)" ");

        cmd_str = strtok(NULL, (const char *)" ");

        ver_str = strtok(NULL, (const char *)" \r\n");

        i = 0;

        int cmd = strstr((const char *)cmd_str, (const char *)"GET") != 000 ? (int)pow(2., 0.) : (strstr((const char *)cmd_str, (const char *)"POST") != 000 ? (int)pow(2., 1.) : (strstr((const char *)cmd_str, (const char *)"HEAD") != 000 ? (int)pow(2., 2.) : (int)pow(2., 3.)));
        int proto = strstr((const char *)ver_str, (const char *)"0.9") != 000 ? (int)pow(2., 0.) : (strstr((const char *)ver_str, (const char *)"1.0") != 000 ? (int)pow(2., 0.) : (strstr((const char *)ver_str, (const char *)"1.1") != 000 ? (int)pow(2., 2.) : (int)pow(2., 3.)));

        unsigned int len = (unsigned int)strlen(uri_str);
        int var = 0;
        int pcnt = 0;
        int apos = 0;
        int plus = 0;
        int cdot = 0;
        int bckslsh = 0;
        int oparen = 0;
        int cparen = 0;
        int fwrd = 0;
        int lt = 0;
        int gt = 0;
        int qstnmrk = 0;
        /* Not currently used in fingerprint */
        int dblqte = 0;
        unsigned char *target = 0;
        int *fngPnt = (int *)malloc(sizeof(int) * 14);
        if (fngPnt == 0)
        {
                return(000);
        }

        target = (unsigned char *)uri_str;
        const unsigned char * end = (const unsigned char *)(uri_str + (sizeof(char) * (len-1)));
        unsigned char * sub = (unsigned char *)malloc(sizeof(char) * 4);
        memset(sub, 0, 4);
        int converted = 0;

        for(;(*target != '\n' && *target != '\r') && i < len; i++)
        {
                if(*target == 46)                    //.. counter
                {
                        if (++target <= end)
                        {
                                if(*target == 46)
                                {
                                        ++cdot;
                                }
                                else
                                {
                                        --target;
                                }
                        }
                }

                if(*target == 47)
                {                       // // counter
                        if (++target <= end)
                        {
                                if(*target == 47)
                                {
                                        fwrd++;
                                }
                                else
                                {
                                        target--;
                                }
                        }
                }

                if(*target == 63)
                {                        //conditional for variables
                        qstnmrk = 1;
                }

                else if(*target == 37)
                {                        //percent counter
                        pcnt++;
                        if (target + 2 <= end)
                        {
                                strncpy((char *)sub, (const char *)target, 3);
                                converted = percentNumberConverter((const char *)sub);
                                if (converted == (int)'"')
                                {
                                        ++dblqte;
                                }
                                else if (converted == (int)'\'')
                                {
                                        ++apos;
                                }
                                ++target;
                                ++target;
                        }
                }

                else if (qstnmrk == 1 && *target == 38) //variable counter
                {
                        var++;
                }

                else if(*target == '"')
                {                        // double quote counter
                        dblqte++;
                }

                else if(*target == 39)
                {                        //apostrophe counter
                        apos++;
                }

                else if(*target == 43)
                {                        //addition counter
                        plus++;
                }

                else if(*target == 40)
                {                        //open parentheses counter
                        oparen++;
                }

                else if(*target == 41)
                {                        //close parentheses counter
                        cparen++;
                }

                else if(*target == 60)
                {                        //less than counter
                        lt++;
                }

                else if(*target == 62)
                {                        //greater than counter
                        gt++;
                }

                else if(*target == 92)
                {                        //backslash counter
                        bckslsh++;
                }

                target++;
        }

        /*FINGER PRINT EXPLANATION:
         * array of integers, each slot contains a specified number (integer) that represents the character count 
         *INDEX 0               HTTP command    GET = 1         POST = 2        HEAD = 4        OTHER = 8

         --->>>  MODIFY ORIGINAL VERSION HERE  <<<---
         --->>>         ORIGINAL VERSION       <<<---
         *INDEX 1               HTTP PROTOCOL   0.9 = 1         1.0 = 1         1.1 = 4         OTHER = 8       
         --->>>    NEW VERSION - NOT IN USE    <<<---
         *INDEX 1               HTTP PROTOCOL   0.9 = 1         1.0 = 2         1.1 = 4         OTHER = 8       
         *INDEX 2               LENGTH          # OF CHARS              
         *INDEX 3               VARIABLES       # OF INPUT
         *INDEX 4               PERCENT         # OF %
         *INDEX 5               APOS            # OF ' 
         *INDEX 6               PLUS            # OF +          
         *INDEX 7               CDOT            # OF .. 
         *INDEX 8               BACKSLASH       # OF \
         *INDEX 9               OPAREN          # OF (
         *INDEX 10              CPAREN          # OF )
         *INDEX 11              FORWARD         # OF //
         *INDEX 12              LT              # OF <
         *INDEX 13              GT              # OF >
         */
        fngPnt[0] = cmd;
        fngPnt[1] = proto;
        fngPnt[2] = len;
        fngPnt[3] = var;
        fngPnt[4] = pcnt;
        fngPnt[5] = apos;
        fngPnt[6] = plus;
        fngPnt[7] = cdot;
        fngPnt[8] = bckslsh;
        fngPnt[9] = oparen;
        fngPnt[10] = cparen;
        fngPnt[11] = fwrd;
        fngPnt[12] = lt;
        fngPnt[13] = gt;

        i = 0;
        if (sub)
        {
                free(sub);
        }

        return (fngPnt);
}

/* itoa.c
 *
 * Below thanks to bhuwnsahni, modified only slightly
 * http://stackoverflow.com/questions/9655202/how-to-convert-integer-to-string-in-c
 *
 */


char * itoa (int i)
{
        char const digit[] = "0123456789";
        char * p = 0;
        p = (char *)malloc(sizeof(char) * 11);
        if (!p)
        {
                return (0);
        }
        if (i < 0)
        {
                *p++ = '-';
                i = -1;
        }
        int shifter = i;
        do
        { //Move to where representation ends
                ++p;
                shifter = shifter/10;
        }
        while (shifter);
        *p = '\0';
        do
        { //Move back, inserting digits as u go
                *--p = digit[i%10];
                i = i/10;
        }
        while (i);
        return p;
}

/* itoa.c */

int main()
{
        /* setup */
        ifstream infile;
        ofstream outfile;

        infile.open((const char *)"./normal\0");

        outfile.open((const char *)"./normal.new\0");

        /* 52k lines each with 15 elements with a max length of 2055 per element */
        char * lines[60000];

        for (int i = 0; i < 60000; i++)
        {
                lines[i] = new char [2056];
        }

        int qty = 0, j = 0;
        int * fsig = 000;
        int * asig = 000;

        /* i don't actually care about the files signature, just generate a new signature to dump */
        fsig = new int [14];
        asig = new int [14];

        /* idiot check and do work */
        if (infile.good() && outfile.good())
        {
                do
                {
                        infile.getline((char *)(lines[qty]), 2055, (char)'\n');

                        /* make corrections */
                        asig = pfp((unsigned char *)lines[qty], strlen(lines[qty]));
                        outfile << lines[0] << " " << lines[1] << " " << lines[2] << " ";
                        for (j = 2; j < 13; j++)
                        {
                                outfile << asig[j] << " ";
                        }
                        outfile << asig[j] << endl;
                        ++qty;
                }
                while (qty < 60000 && infile.good() && infile.peek() != EOF && outfile.good());
        }
        else
        {
                cout << "Unable to open files\n" << flush;
                return (1);
        }

        /* cleanup */

        infile.close();
        outfile.close();

        for (int i = 0; i < 60000; i++)
        {
                delete[] lines[i];
        }
        delete[] fsig;
        delete[] asig;

        cout << "Corrected " << qty << " normal URIs. Should be same as number of lines in 'normal' file" << endl << flush;


        return (0);
}
