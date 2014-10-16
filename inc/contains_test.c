/* contains test program */

#include <stdio.h>

#include "contains.h"

int main()
{
        printf("\r\n--------------------------------\r\nTesting some basic situations\r\n--------------------------------\r\n");
        char str1 [] = "this is a sample sentance with a good length\0";
        char str2 [] = "samzze\0";

        printf("\r\nstr1 = %s, str2 = %s\r\n", str1, str2);
        printf("calling Contains(str1, str2)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str1, str2), (Contains(str1, str2)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str3 [] = "this is a sample sentance with a good length\0";
        char str4 [] = "sample\0";

        printf("\r\nstr4 = %s, str3 = %s\r\n", str4, str3);
        printf("calling Contains(str4, str3)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str4, str3), (Contains(str4, str3)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str5 [] = "this is a sample sentance with a good length\0";
        char str6 [] = "";

        printf("\r\nstr5 = %s, str6 = %s\r\n", str5, "(An empty string)");
        printf("calling Contains(str5, str6)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str5, str6), (Contains(str5, str6)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str7 [] = "\0";
        char str8 [] = "\0";

        printf("\r\nstr7 = %s, str8 = %s\r\n", "(A NULL character)", "(A NULL character)");
        printf("calling Contains(str7, str8)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str7, str8), (Contains(str7, str8)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str9 [] = "tester tester\0";
        char str10 [] = "\0";

        printf("\r\nstr9 = %s, str10 = %s\r\n", str9, "(A NULL character)");
        printf("calling Contains(str9, str10)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str9, str10), (Contains(str9, str10)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str11 [] = "this is a sample sentance with a good length\0";
        char str12 [] = " ";

        printf("\r\nstr11 = %s, str12 = %s\r\n", str11, "(A space)");
        printf("calling Contains(str11, str12)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str11, str12), (Contains(str11, str12)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str13 [] = "this is a sample sentance with a good length\0";

        printf("\r\nstr13 = %s, str13 = %s\r\n", str13, str13);
        printf("calling Contains(str13, str13)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str13, str13), (Contains(str13, str13)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str14 [] = "this is a sample sentance with a good length\0";
        char str15 [] = "sam\0";

        printf("\r\nstr14 = %s, str15 = %s\r\n", str14, str15);
        printf("calling Contains(str14, str15)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str14, str15), (Contains(str14, str15)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str16 [] = "this is a sample sentance with a good length\0";
        char str17 [] = "ngth\0";

        printf("\r\nstr16 = %s, str17 = %s\r\n", str16, str17);
        printf("calling Contains(str16, str17)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %d:%s\r\n", Contains(str16, str17), (Contains(str16, str17)) ? "TRUE" : "FALSE");
        fflush(stdout);

        /* length of testing strings */
        int tlen = 4;

        printf("\r\n--------------------------------\r\nNow brute force testing string length %d\r\n--------------------------------\r\n", tlen);

        char * string = NULL;
        char * pattern = NULL;

        string = (char *)malloc(sizeof(char) * tlen+1);
        pattern = (char *)malloc(sizeof(char) * tlen+1);

        int e = 0;
        long int matches = 0;
        long int nonmatches = 0;
        long int test_ct = 0;
        long int false_pos = 0;
        long int false_neg = 0;

        /* std ascii space ( ) - first regular character */
        int start = 97;
        /* std ascii tilde (~) - last regular character */
        int end = 122;

        /* std ascii space ( ) - first regular character */
        //int start = 32;
        /* std ascii tilde (~) - last regular character */
        //int end = 126;

        int i = 0;
        int ret = 0;

        int i1 = 0;
        int i2 = 0;
        int i3 = 0;
        int i4 = 0;

        int j1 = 0;
        int j2 = 0;
        int j3 = 0;
        int j4 = 0;
        /* initialize testing strings */
        while (i < tlen)
        {
                string[i] = (char)start;
                pattern[i] = (char)start;
                i++;
        }
        string[tlen] = '\0';
        pattern[tlen] = '\0';


        /* algo:
         * 1. change first char once
         * 2. repeat 1 range times, wrapping ...
         * 3. change second char once
         * 4. repeat 1, 2, 3 range times, wrapping change st changed char ends at initial value
         * 5. change third char once
         * 6. repeat 1, 2, 3, 4, 5 range times
         * 7. ...
         *
         * i = qty of chars - 1
         * while i >= 0
         *   // do this set range number of times with a iteration of a char once after each set
         *   j = 0
         *   while j < i // at completion iterate next number
         *     while k < range
         *       str[j]++ % range
         *       k++
         *     j++
         *     str[i]++ % range
         *     str[j]++ % range
         *   i--
         *
         *
         */
        /* change each character in each string */

        int range = end - start;
        while (j1 <= range)
        {
                j2 = 0;
                while (j2 <= range)
                {
                        j3 = 0;
                        while (j3 <= range)
                        {
                                j4 = 0;
                                while (j4 <= range)
                                {



                                        i1 = 0;
                                        while (i1 <= range)
                                        {
                                                i2 = 0;
                                                while (i2 <= range)
                                                {
                                                        i3 = 0;
                                                        while (i3 <= range)
                                                        {
                                                                i4 = 0;
                                                                while (i4 <= range)
                                                                {


                                                                        if (strcmp(string, pattern) == 0)
                                                                        {
                                                                                e = 1;
                                                                        }
                                                                        else
                                                                        {
                                                                                e = 0;
                                                                        }
                                                                        //printf("string = %s\r\npattern = %s\r\n", string, pattern);
                                                                        ret = Contains(string, pattern);
                                                                        test_ct++;
                                                                        if (ret == 1)
                                                                        {
                                                                                if (e == 1)
                                                                                {
                                                                                        matches++;
                                                                                }
                                                                                else
                                                                                {
                                                                                        false_pos++;
                                                                                }
                                                                        }
                                                                        else
                                                                        {
                                                                                if (e == 1)
                                                                                {
                                                                                        false_neg++;
                                                                                }
                                                                                else
                                                                                {
                                                                                        nonmatches++;
                                                                                }
                                                                        }



                                                                        pattern[3] = (pattern[4]+1) % (end);
                                                                        if (pattern[3] == 0)
                                                                        {
                                                                                pattern[3] = start;
                                                                        }
                                                                        i4++;
                                                                }
                                                                pattern[2] = (pattern[2]+1) % (end);
                                                                if (pattern[2] == 0)
                                                                {
                                                                        pattern[2] = start;
                                                                }
                                                                i3++;

                                                                /*
                                                                printf("t:%ld\r\n", test_ct);
                                                                printf("m:%ld\r\n", matches);
                                                                printf("n:%ld\r\n", nonmatches);
                                                                printf("fp:%ld\r\n", false_pos);
                                                                printf("fn:%ld\r\n", false_neg);
                                                                */
                                                        }
                                                        pattern[1] = (pattern[1]+1) % (end);
                                                        if (pattern[1] == 0)
                                                        {
                                                                pattern[1] = start;
                                                        }
                                                        i2++;
                                                }
                                                pattern[0] = (pattern[0]+1) % (end);
                                                if (pattern[0] == 0)
                                                {
                                                        pattern[0] = start;
                                                }
                                                i1++;
                                        }


                                        string[3] = (string[3]+1) % (end);
                                        if (string[3] == 0)
                                        {
                                                string[3] = start;
                                        }
                                        j4++;
                                }
                                string[2] = (string[2]+1) % (end);
                                if (string[2] == 0)
                                {
                                        string[2] = start;
                                }
                                j3++;
                        }
                        string[1] = (string[1]+1) % (end);
                        if (string[1] == 0)
                        {
                                string[1] = start;
                        }
                        j2++;
                }
                string[0] = (string[0]+1) % (end);
                if (string[0] == 0)
                {
                        string[0] = start;
                }
                j1++;
        }


        printf("tests ran: %ld\r\n", test_ct);
        printf("correct matches: %ld\r\n", matches);
        printf("correct non-matches: %ld\r\n", nonmatches);
        printf("false positives: %ld\r\n", false_pos);
        printf("false negatives: %ld\r\n", false_neg);

        printf("\r\n");

        return (0);
}
