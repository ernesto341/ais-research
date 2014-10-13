#include <stdio.h>
#include <stdlib.h>

// Below thanks to bhuwnsahni, modified only slightly
// http://stackoverflow.com/questions/9655202/how-to-convert-integer-to-string-in-c

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
