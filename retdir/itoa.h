#include <stdlib.h>

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
        //Move to where representation ends
        do
        {
                ++p;
                shifter = shifter/10;
        }
        while (shifter);
        *p = '\0';
        //Move back, inserting digits as u go
        do
        {
                *--p = digit[i%10];
                i = i/10;
        }
        while (i);
        return (p);
}
