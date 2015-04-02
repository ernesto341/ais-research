/* contains.h 
 *
 * strict boolean contains
 * 1 == true
 * 0 == false/bad input
 *
 *
 * TO DO:
 * check sum of total for equality as secondary confirmation
 *    keep a running sum of each character as you parse for comparison later
 *
 */

#ifndef __CONTAINS_H__
#define __CONTAINS_H__

#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

char * Contains (char * str, char * pattern)
{
        if (str == NULL || pattern == NULL)
        {
                if (DEBUG == 1)
                {
                        fprintf(stderr, "In Contains, str or pattern are NULL\r\n");
                        fflush(stderr);
                }
                return (NULL);
        }
        uint32_t str_len = strlen(str);
        uint32_t pattern_len = strlen(pattern);
        if (&(str[0]) == &(pattern[0]) && str_len == pattern_len)
        {
                if (DEBUG == 1)
                {
                        fprintf(stderr, "In Contains, str == pattern\r\n");
                        fflush(stderr);
                }
                return (str);
        }
        if (pattern_len < 1)
        {
                if (DEBUG == 1)
                {
                        fprintf(stderr, "In Contains, pattern_len < 1\r\n");
                        fflush(stderr);
                }
                return (NULL);
        }
        if (str_len < pattern_len)
        {
                if (DEBUG == 1)
                {
                        fprintf(stderr, "In Contains, str_len < pattern_len\r\n");
                        fflush(stderr);
                }
                return (NULL);
        }
        if (DEBUG == 1)
        {
                fprintf(stderr, "\r\nIn Contains\r\ngot str = %s, pattern = %s\r\n", str, pattern);
                fprintf(stderr, "str_len = %d, pattern_len = %d\r\n", str_len, pattern_len);
                fflush(stderr);
        }
        uint32_t i = 0, j = 0, k = 1;
        uint8_t good = 0;
        if (pattern_len == 1)
        {
                while (i < str_len)
                {
                        if (str[i] == pattern[0])
                        {
                                /* found */
                                return (&(str[i]));
                        }
                        i++;
                }
                return (NULL);
        }
        /* only check up to max len */
        if (DEBUG == 1)
        {
                fprintf(stderr, "\r\nabout to enter outer while loop with the following confitions:\r\n\r\nwhile ( i < %d )\r\ni starting at %d\r\n", str_len-pattern_len, i);
                fflush(stderr);
        }
        while (i < str_len-pattern_len+1)
        {
                /* check first and last chars for match */
                if (DEBUG == 1)
                {
                        fprintf(stderr, "first if test: str[%d] ^ pattern[%d] == 0\r\n\t%c ^ %c == 0: %s\r\n", i, 0, str[i], pattern[0], (((str[i]) ^ (pattern[0])) == 0) ? "TRUE" : "FALSE");
                        fflush(stderr);
                }
                if (((str[i]) ^ (pattern[0])) == 0)
                {
                        if (DEBUG == 1)
                        {
                                fprintf(stderr, "second if test: %c ^ %c == 0: %s\r\n", str[i + pattern_len-1], pattern[pattern_len-1], (((str[i + pattern_len-1]) ^ (pattern[pattern_len-1])) == 0) ? "TRUE" : "FALSE");
                                fflush(stderr);
                        }
                        if (((str[i + pattern_len-1]) ^ (pattern[pattern_len-1])) == 0)
                        {
                                /* check digrams for equality, exit on false.
                                 * possible for even number of chars, perfect only check inner letters.
                                 * possible for odd number of chars, last check should check second to last + last */
                                j = i+1;
                                k = 1;
                                good = 0;
                                while (j-i < pattern_len-2 && good == 0)
                                {
                                        if (DEBUG == 1)
                                        {
                                                fprintf(stderr, "inside inner while loop\r\ntest: !((((str[%d]) | (str[%d])) ^ ((pattern[%d]) | (pattern[%d]))) == 0)\r\n\t((%c | %c) ^ (%c | %c)) == 0: %s\r\n", j, j+1, k, k+1, str[j], str[j+1], pattern[k], pattern[k+1], (!((((str[j]) | (str[j+1]))^((pattern[k]) | (pattern[k+1]))) == 0)) ? "NO GOOD" : "STILL GOOD");
                                                fflush(stderr);
                                        }
                                        if (!((((str[j]) | (str[j+1])) ^ ((pattern[k]) | (pattern[k+1]))) == 0))
                                        {
                                                good = 1;
                                        }
                                        j+=2;
                                        k+=2;
                                }
                                if (good == 0)
                                {
                                        return (&(str[i]));
                                }
                        }
                }
                i++;
        }
        return (NULL);
}

#endif

/* contains.h */
