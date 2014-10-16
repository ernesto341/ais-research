/* contains test program */

#include <stdio.h>

#include "contains.h"

int main()
{
        char str1 [] = "this is a sample sentance with a good length\0";
        char str2 [] = "samzze\0";

        printf("\r\nstr1 = %s, str2 = %s\r\n", str1, str2);
        printf("calling Contains(str1, str2)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str1, str2), (Contains(str1, str2)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str3 [] = "this is a sample sentance with a good length\0";
        char str4 [] = "sample\0";

        printf("\r\nstr4 = %s, str3 = %s\r\n", str4, str3);
        printf("calling Contains(str4, str3)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str4, str3), (Contains(str4, str3)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str5 [] = "this is a sample sentance with a good length\0";
        char str6 [] = "";

        printf("\r\nstr5 = %s, str6 = %s\r\n", str5, "(An empty string)");
        printf("calling Contains(str5, str6)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str5, str6), (Contains(str5, str6)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str7 [] = "\0";
        char str8 [] = "\0";

        printf("\r\nstr7 = %s, str8 = %s\r\n", "(A NULL character)", "(A NULL character)");
        printf("calling Contains(str7, str8)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str7, str8), (Contains(str7, str8)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str9 [] = "tester tester\0";
        char str10 [] = "\0";

        printf("\r\nstr9 = %s, str10 = %s\r\n", str9, "(A NULL character)");
        printf("calling Contains(str9, str10)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str9, str10), (Contains(str9, str10)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str11 [] = "this is a sample sentance with a good length\0";
        char str12 [] = " ";

        printf("\r\nstr11 = %s, str12 = %s\r\n", str11, "(A space)");
        printf("calling Contains(str11, str12)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str11, str12), (Contains(str11, str12)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str13 [] = "this is a sample sentance with a good length\0";

        printf("\r\nstr13 = %s, str13 = %s\r\n", str13, str13);
        printf("calling Contains(str13, str13)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str13, str13), (Contains(str13, str13)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str14 [] = "this is a sample sentance with a good length\0";
        char str15 [] = "sam\0";

        printf("\r\nstr14 = %s, str15 = %s\r\n", str14, str15);
        printf("calling Contains(str14, str15)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str14, str15), (Contains(str14, str15)) ? "TRUE" : "FALSE");
        fflush(stdout);

        char str16 [] = "this is a sample sentance with a good length\0";
        char str17 [] = "ngth\0";

        printf("\r\nstr16 = %s, str17 = %s\r\n", str16, str17);
        printf("calling Contains(str16, str17)...\r\n\r\n");
        fflush(stdout);
        printf("Contains returned %p:%s\r\n", Contains(str16, str17), (Contains(str16, str17)) ? "TRUE" : "FALSE");
        fflush(stdout);


        return (0);
}
