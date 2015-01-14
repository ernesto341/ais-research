/* Written by Ernest Richards
 *
 * Modified from 'example.c' written by Chema Garcia
 * Major contributions by Alfonso Puga
 * Other contributions by the Hardins, as well as all of the tutors in the tutoring center.
 *
 */

/********************************************************************************
 * Copyright (c) 2011, Chema Garcia                                             *
 * All rights reserved.                                                         *
 *                                                                              *
 * Redistribution and use in source and binary forms, with or                   *
 * without modification, are permitted provided that the following              *
 * conditions are met:                                                          *
 *                                                                              *
 *    * Redistributions of source code must retain the above                    *
 *      copyright notice, this list of conditions and the following             *
 *      disclaimer.                                                             *
 *                                                                              *
 *    * Redistributions in binary form must reproduce the above                 *
 *      copyright notice, this list of conditions and the following             *
 *      disclaimer in the documentation and/or other materials provided         *
 *      with the distribution.                                                  *
 *                                                                              *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  *
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE    *
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         *
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      *
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      *
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   *
 * POSSIBILITY OF SUCH DAMAGE.                                                  *
 ********************************************************************************/

/* TO DO:
 *
 * ensure that pcktFingerPrint function handles hex substitutions?
 * handle non http packets in extractHttpHdr
 *
 */


#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef __FAVOR_BSD
# define __FAVOR_BSD
#endif

#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pcap.h>
#include <libntoh.h>

#include <itoa.h>
#include <share.h>
#include <globals.h>
#include <contains.h>

#define DEBUG 1

#define RECV_CLIENT 1
#define RECV_SERVER 2

typedef struct
{
        unsigned char *data;
        size_t data_len;
        char *path;
} peer_info_t, *ppeer_info_t;

pcap_t *handle = 0;
pntoh_tcp_session_t tcp_session = 0;
pntoh_ipv4_session_t ipv4_session = 0;
unsigned short	receive = 0;

#ifndef _keys
#define _keys
uint32_t shmkey[] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[] = {959, 653, 987, 627, 905};
#endif
snc_t snc;

char * tmp = 0;
sig_atomic_t * t5Convert;
static uint8_t pending_more_hdr_data = 0;
int32_t fd = 0;

uint32_t i = 0;

/**
 * @brief Increments a double ptr by reference
 */
inline static void inCtr(sig_atomic_t *** s)
{
        ((*s)[0][0])++;
        ((*s)[0][0]) = ((((*s)[0][0]) % SIGQTY) != 0 ? (((*s)[0][0]) % SIGQTY) : SIGQTY); // 1 - 5
}

/**
 * @brief Exit function (closes the capture handle and releases all resource from libntoh)
 */
void shandler ( int sign )
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );

        uint32_t len = 0;
        if (DEBUG)
        {
                if (snc.smem.shm[CTL][FLAGS] == CDONE)
                {
                        strncpy(buf, "\n\t\t[i] --- Signaled to quit by consumer\n", 40);
                        write(2, buf, 40);
                }
                strncpy(buf, "\n\t\t[i] --- Signal: ", 19);
                tmp = itoa(sign);
                len = strlen(tmp);
                strncat(buf, tmp, len);
                len += 19;
                write(2, buf, len);
        }
        snc.smem.shm[CTL][FLAGS] = PDONE;

        freeMem(&snc);

        if (t5Convert)
        {
                free(t5Convert);
        }

        pcap_close( handle );

        ntoh_exit();

        strncpy(buf, "\n\t\tX      -----   Inactive   -----      X\n\n", 43);
        write(2, buf, 43);

        exit(sign);
}

/**
 * @brief Returns a struct which stores some peer information
 */
ppeer_info_t get_peer_info ( unsigned char *payload , size_t payload_len , pntoh_tcp_tuple5_t tuple )
{
        ppeer_info_t ret = 0;
        size_t len = 0;
        char path[1024] = {0};

        /* gets peer information */
        ret = (ppeer_info_t) calloc ( 1 , sizeof ( peer_info_t ) );
        ret->data_len = payload_len;
        ret->data = (unsigned char*) calloc ( ret->data_len , sizeof ( unsigned char ) );
        memcpy ( ret->data , payload , ret->data_len );

        snprintf ( path , sizeof(path) , "%s:%d-" , inet_ntoa ( *(struct in_addr*)&(tuple->source) ) , ntohs(tuple->sport) );
        len = strlen(path);
        snprintf ( &path[len] , sizeof(path) - len, "%s:%d" , inet_ntoa ( *(struct in_addr*)&(tuple->destination) ) , ntohs(tuple->dport) );

        ret->path = strndup ( path , sizeof(path) );

        return (ret);
}

/**
 * @brief Frees the ppeer_info_t struct
 */
void free_peer_info ( ppeer_info_t pinfo )
{
        /* free peer info data */
        if ( ! pinfo )
        {
                return;
        }

        free ( pinfo->data );
        free ( pinfo->path );
        free ( pinfo );

        return;
}

/**
 * @brief Returns the name of a protocol
 */
inline char *get_proto_description ( unsigned short proto )
{
        switch ( proto )
        {
                case IPPROTO_ICMP:
                        return ("ICMP");

                case IPPROTO_TCP:
                        return ("TCP");

                case IPPROTO_UDP:
                        return ("UDP");

                case IPPROTO_IGMP:
                        return ("IGMP");

                case IPPROTO_IPV6:
                        return ("IPv6");

                case IPPROTO_FRAGMENT:
                        return ("IPv6 Fragment");

                default:
                        return ("Undefined");
        }
}

/**
 * @brief Writes anything to a dump file in the current directory. Currently unused.
 */
void generic_write_data ( void * data )
{
        if (data == 0 || strlen((const char *)data) < 1)
        {
                if (DEBUG)
                {
                        write(2, "[e] No data passed to generic_write_data\n", 41);
                }
                return;
        }

        if ( (fd = open ( (const char *)"./generic_dump" , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                if (DEBUG)
                {
                        write(2 , "\n[e] Error opening data file \"./generic_dump\"\n", 46);
                }
                return;
        }

        write ( fd , (const char *)data , strlen((const char *)data) );
        close ( fd );
        return;
}

/**
 * @brief Writes whatever is currently in the 'hdr_data' global to a file in the current directory. Currently unused.
 */
void write_hdr_data ( void )
{
        if (hdr_data == 0 || strlen((char *)hdr_data) < 1)
        {
                if (DEBUG)
                {
                        write(2 , "\n[e] Nothing to write\n", 22);
                }
                return;
        }

        char path [102];

        if ( (fd = open ( (const char *)"./hdr_and_sig" , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                if (DEBUG)
                {
                        write(2 , "\n[e] Error opening data file \"./hdr_and_sig\"\n", 45);
                }
                return;
        }

        write ( fd , (const char *)hdr_data , strlen((const char *)hdr_data) );
        write ( fd , "\n" , 1 );

        /*FINGER PRINT EXPLANATION:
         * array of integers, each slot contains a specified number (integer) that represents the character count 
         *INDEX 0               HTTP command    GET = 1         POST = 2        HEAD = 4        OTHER = 8
         *INDEX 1               HTTP PROTOCOL   0.9 = 1         1.0 = 1         1.1 = 4         OTHER = 8       
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

        write (fd, "Finger Print:", 13);
        if (snc.mem.sigs == 0 || snc.mem.sigs[(snc.mem.sigs[CTL][POS])] == 0)
        {
                write(fd, "\nNo fingerprint found\n", 21);
                close ( fd );
                pending_more_hdr_data = 0;
                return;
        }
        i = 0;
        while (i < fngPntLen)
        {
                strncpy (path, "\n", 1);
                if (i == 0)
                {
                        strncat (path, "CMD  -  ", 8);
                }
                else if (i == 1)
                {
                        strncat (path, "PROT -  ", 8);
                }
                else if (i == 2)
                {
                        strncat (path, "LEN  -  ", 8);
                }
                else if (i == 3)
                {
                        strncat (path, "VARS -  ", 8);
                }
                else if (i == 4)
                {
                        strncat (path, "%    -  ", 8);
                }
                else if (i == 5)
                {
                        strncat (path, "'    -  ", 8);
                }
                else if (i == 6)
                {
                        strncat (path, "+    -  ", 8);
                }
                else if (i == 7)
                {
                        strncat (path, "..   -  ", 8);
                }
                else if (i == 8)
                {
                        strncat (path, "\\    -  ", 8);
                }
                else if (i == 9)
                {
                        strncat (path, "(    -  ", 8);
                }
                else if (i == 10)
                {
                        strncat (path, ")    -  ", 8);
                }
                else if (i == 11)
                {
                        strncat (path, "//   -  ", 8);
                }
                else if (i == 12)
                {
                        strncat (path, "<    -  ", 8);
                }
                else if (i == 13)
                {
                        strncat (path, ">    -  ", 8);
                }
                uint16_t t = snc.mem.sigs[CTL][POS] - 1;
                char * tmp = itoa(snc.mem.sigs[(t > 0 ? t : SIGQTY)][i]);
                strncat (path, tmp, strlen(tmp));
                strncat (path, "\n", 1);
                write ( fd , path , strlen(path) );
                i++;
        }

        write ( fd , "\n" , 1 );
        close ( fd );
        pending_more_hdr_data = 0;
        free(hdr_data);
        hdr_data = 0;

        return;
}

/**
 * @brief Writes the ppeer_info_t data field to disk
 */
void write_data ( ppeer_info_t info )
{
        if ( !info )
        {
                return;
        }

        if ( (fd = open ( info->path , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                if (DEBUG)
                {
                        fprintf(stderr , "\n[e] Error %d writting data to \"%s\": %s" , errno , info->path , strerror( errno ) );
                }
                return;
        }

        write ( fd , info->data , info->data_len );
        close ( fd );

        return;
}

/**
 * @brief Create the firnger print from a HTTP header
 */
int * pcktFingerPrint(const unsigned char * curPcktData, const uint32_t dataLen)
{
        if(curPcktData == 0 || dataLen == 0)
        {
                if (DEBUG)
                {
                        write(2, "There was not header to parse\n", 30);
                }
                return (0);
        }

        i = 0;

        if (DEBUG)
        {
                strcpy (buf, "\n\t-----\tHEADER BEGIN\t-----\n");
                write (2, buf, 27);
                while (i < dataLen)
                {
                        write(2, (const char *)(&(curPcktData[i++])), 1);
                        fflush(stderr);
                }
                strcpy (buf, "\n\t------\tHEADER END\t------\n");
                write (2, buf, 27);
                fflush(stderr);
        }

        i = 0;

        int32_t cmd = 8;
        int32_t cmdSet = 0;
        int32_t proto = 8;
        int32_t protoSet = 0;
        uint32_t len = dataLen;   //calculated during parsing
        int32_t var = 0;
        int32_t pcnt = 0;
        int32_t apos = 0;
        int32_t plus = 0;
        int32_t cdot = 0;
        int32_t bckslsh = 0;
        int32_t oparen = 0;
        int32_t cparen = 0;
        int32_t fwrd = 0;
        int32_t lt = 0;
        int32_t gt = 0;
        int32_t qstnmrk = 0;
        unsigned char *target = 0;
        int32_t *fngPnt = malloc(sizeof(int32_t) * fngPntLen);
        if (fngPnt == 0)
        {
                if (DEBUG)
                {
                        write(2, "\n\t[e] --- Unable to allocate sufficient memory\n", 47);
                }
                shandler(0);
        }

        target = (unsigned char *)curPcktData;
        for(;(*target != '\n' && *target != '\r') && i < len; i++)
        {
                if(protoSet == 0 && (*target == 48 || *target == 49))             //proto
                {
                        target++;
                        if(*target == 46)                              //http version 
                        {
                                target++;
                                if(*target == 48)
                                {
                                        proto = 2;
                                        protoSet = 1;
                                }
                                else if(*target == 57)
                                {
                                        proto = 1;
                                        protoSet = 1;
                                }
                                else if(*target == 49)
                                {
                                        proto = 4;
                                        protoSet = 1;
                                }
                                else
                                {
                                        target--;
                                        target--;
                                }
                        }  

                        else
                        {
                                target--;
                        }
                }

                if(cmdSet == 0 && *target == 71)                  //cmd get
                {
                        target++;
                        if(*target == 69)
                        {
                                target++;
                                if(*target == 84)
                                {
                                        cmd = 1;
                                        cmdSet = 1;
                                }
                                else
                                {
                                        target--;
                                }
                        }
                        else
                        {
                                target--;
                        }
                }

                if(*target == 72 && cmdSet == 0)                  //cmd head
                {
                        target++;
                        if(*target == 69)
                        {
                                target++;
                                if(*target == 65)
                                {
                                        target++;
                                        if(*target == 68)
                                        {
                                                cmd = 4;
                                                cmdSet = 1;
                                        }
                                        else
                                        {
                                                target--;
                                        }
                                }
                                else
                                {
                                        target--;
                                }
                        }
                        else
                        {
                                target--;
                        }
                }
                if(*target == 80 && cmdSet == 0)                  //cmd post
                {
                        target++;
                        if(*target == 79)
                        {
                                target++;
                                if(*target == 83)
                                {
                                        target++;
                                        if(*target == 84)
                                        {
                                                cmd = 2;
                                                cmdSet = 1;
                                        }
                                        else
                                        {
                                                target--;
                                        }
                                }
                                else
                                {
                                        target--;
                                }
                        }
                        else
                        {
                                target--;
                        }
                }

                if(*target == 46)                    //.. counter
                {
                        target++;
                        if(*target == 46)
                        {
                                cdot++;
                        }
                        else
                        {
                                target--;
                        }
                }

                if(*target == 47)
                {                       // // counter
                        target++;
                        if(*target == 47)
                        {
                                fwrd++;
                        }
                        else
                        {
                                target--;
                        }
                }

                if(*target == 63)
                {                        //conditional for variables
                        qstnmrk = 1;
                }

                else if(*target == 37)
                {                        //percent counter
                        pcnt++;
                }

                else if (qstnmrk == 1 && *target == 38) //variable counter
                {
                        var++;
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
         *INDEX 1               HTTP PROTOCOL   0.9 = 1         1.0 = 1         1.1 = 4         OTHER = 8       
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

        return (fngPnt);
}

/**
 * @brief Function to resize 'hdr_data' global if necessary
 */
inline static void resizeHdr(void)
{
        unsigned char * tmp = 0;
        tmp = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size * 2);
        if (tmp == 0)
        {
                if (DEBUG)
                {
                        write(2, "\n\t[e] --- Unable to allocate sufficient memory!\n", 47);
                }
                shandler(0);
        }
        memcpy(tmp, hdr_data, hdr_size);
        free(hdr_data);
        hdr_data = tmp;
        hdr_size *= 2;
}

/**
 * @brief Function to extract the header from a packet
 */
uint8_t extractHttpHdr (const char * udata)
{
        uint32_t j = 0;
        if (pending_more_hdr_data != 0)
        {
                j = strlen((const char *)hdr_data);
        }
        if (hdr_data == 0)
        {
                hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size);
                if (hdr_data == 0)
                {
                        if (DEBUG)
                        {
                                write(2, "\n\t[e] --- Unable to allocate sufficient memory!\n", 47);
                        }
                        shandler(0);
                }
        }
        uint8_t eoh = 1;
        uint32_t data_len = strlen((const char *)udata);
        while (i < data_len && eoh != 0)
        {
                hdr_data[j++] = udata[i++];
                /* HERE
                 * maybe check to see if system is little endian or big endian?
                 */
                if (i > 3 && (udata[i] == 0x0a && udata[i-1] == 0x0d && udata[i-2] == 0x0a && udata[i-3] == 0x0d))
                {
                        if (DEBUG)
                        {
                                write(2, "\n\t[i] --- Found end of header with 0d0a 0d0a\n", 45);
                        }
                        eoh = 0;
                }
                else if (i > 3 && (udata[i] == 0x0d && udata[i-1] == 0x0a && udata[i-2] == 0x0d && udata[i-3] == 0x0a))
                {
                        if (DEBUG)
                        {
                                write(2, "\n\t[i] --- Found end of header with 0a0d 0a0d\n", 45);
                        }
                        eoh = 0;
                }
                else if (j >= hdr_size-1)
                {
                        if (DEBUG)
                        {
                                write(2, "\n\t[a] --- Resizing header\n", 26);
                        }
                        resizeHdr();
                }
        }
        return (eoh);
}

/**
 * @brief Function to dump Fingerprint and Uniqe tuple to shared memory for pickup by receive and pull
 */
inline uint8_t dumpToShm(void)
{
        if (DEBUG)
        {
                fprintf(stderr, "\nin dumpToShm, snc.smem.shm[CTL][POS] = %d\n", snc.smem.shm[CTL][POS]);
                fflush(stderr);
        }
        /* try-lock shared memory */
        if (snc.smem.shm[CTL][FLAGS] == CREAD || snc.smem.shm[CTL][FLAGS] == 0)
        {
                snc.smem.shm[CTL][FLAGS] = PWING;
                /* do the dump to shm routine */
                while (snc.smem.shm[CTL][POS] != snc.mem.sigs[CTL][POS])
                {
                        memcpy((sig_atomic_t *)snc.smem.t5shm[((snc.smem.shm[CTL][POS] - 1))], (sig_atomic_t *)snc.mem.t5s[(snc.smem.shm[CTL][POS]) - 1], (sizeof(sig_atomic_t) * t5TplLen));
                        memcpy((sig_atomic_t *)snc.smem.shm[snc.smem.shm[CTL][POS]], (sig_atomic_t *)snc.mem.sigs[snc.smem.shm[CTL][POS]], (sizeof(sig_atomic_t) * fngPntLen));
                        /* unlock shared memory */
                        if ((snc.smem.shm[CTL][PEND]) == 5)
                        {
                                if (DEBUG)
                                {
                                        write(2, "\n\t[ALERT]\t---\tOverwriting a signature!\n\n", 40);
                                }
                        }
                        else
                        {
                                snc.smem.shm[CTL][PEND] += 1; // pending
                        }
                        inCtr((sig_atomic_t ***)(&snc.smem.shm)); // pos
                }
                snc.smem.shm[CTL][FLAGS] = PWTEN;
                /* reset vars */
                pending_more_hdr_data = 0;
                return (0);
        }
        else
        {
                if (DEBUG)
                {
                        fprintf(stderr, "\n\t[i] --- Blocked with flag %d\n", snc.smem.shm[CTL][FLAGS]);
                        fflush(stderr);
                }
                return ((uint8_t)(snc.smem.shm[CTL][FLAGS]));
        }
}

/**
 * @brief Fingerprints header, increments counter, and resets hdr_data
 */
inline static void extractSig(void)
{
        snc.mem.sigs[(snc.mem.sigs[CTL][POS])] = pcktFingerPrint(hdr_data, strlen((const char *)hdr_data));
        memset(hdr_data, '\0', hdr_size);
        inCtr(&snc.mem.sigs);
}

/**
 * @brief Attempts to manually pull source port from a t5tuple string. currently not in use.
 */
inline uint32_t getSrcPrt(const char * path)
{
        uint32_t len = strlen(path);
        while (path[i] != ':' && i < len)
        {
                i++;
        }
        if (i >= len)
        {
                return (0);
        }
        i++;
        return ((uint32_t)(atoi((char *)(&(path[i])))));
}

/**
 * @brief Attempts to manually pull destination port from a t5tuple string. currently not in use.
 */
inline uint32_t getDstPrt(const char * path)
{
        uint32_t len = strlen(path);
        while (path[i] != ':' && i < len)
        {
                i++;
        }
        while (path[i] != ':' && i < len)
        {
                i++;
        }
        if (i >= len)
        {
                return (0);
        }
        i++;
        return ((uint32_t)(atoi((char *)(&(path[i])))));
}

/**
 * @brief Send a TCP segment to libntoh
 */
void send_tcp_segment ( struct ip *iphdr , pntoh_tcp_callback_t callback )
{
        ppeer_info_t		pinfo;
        ntoh_tcp_tuple5_t	tcpt5;
        pntoh_tcp_stream_t	stream;
        struct tcphdr 		*tcp;
        size_t 				size_ip;
        size_t				total_len;
        size_t				size_tcp;
        size_t				size_payload;
        unsigned char		*payload;
        int32_t					ret;
        unsigned int		error;

        size_ip = iphdr->ip_hl * 4;
        total_len = ntohs( iphdr->ip_len );

        tcp = (struct tcphdr*)((unsigned char*)iphdr + size_ip);
        if ( (size_tcp = tcp->th_off * 4) < sizeof(struct tcphdr) )
        {
                return;
        }

        payload = (unsigned char *)iphdr + size_ip + size_tcp;
        size_payload = total_len - ( size_ip + size_tcp );

        ntoh_tcp_get_tuple5 ( iphdr , tcp , &tcpt5 );

        /* find the stream or creates a new one */
        if ( !( stream = ntoh_tcp_find_stream( tcp_session , &tcpt5 ) ) )
        {
                if ( ! ( stream = ntoh_tcp_new_stream( tcp_session , &tcpt5, callback , 0 , &error , 1 , 1 ) ) )
                {
                        if (DEBUG)
                        {
                                fprintf ( stderr , "\n[e] Error %d creating new stream: %s" , error , ntoh_get_errdesc ( error ) );
                        }
                        return;
                }
        }

        if ( size_payload > 0 )
        {
                pinfo = get_peer_info ( payload , size_payload , &tcpt5 );
        }
        else
        {
                pinfo = 0;
        }

        if (pinfo != 0)
        {
                //if (Contains((char *)payload, "HTTP") && (Contains((char *)payload, "GET") || Contains((char *)payload, "POST") || Contains((char *)payload, "HEAD")))
                if (ntohs(tcpt5.dport) == 80)
                {
                        pending_more_hdr_data = extractHttpHdr((const char *)(payload));
                        if (pending_more_hdr_data == 0)
                        {
                                size_t l = (strlen((const char *)(pinfo->path)));
                                i = 0;
                                while (i < l)
                                {
                                        snc.mem.t5s[(snc.smem.shm[CTL][POS]) - 1][i] = (sig_atomic_t)(pinfo->path[i]);
                                        i++;
                                }
                                snc.mem.t5s[(snc.smem.shm[CTL][POS]) - 1][i] = (sig_atomic_t)((const char)'\0');
                                if (DEBUG)
                                {
                                        write(2, "\n\t[i] --- tcp tuple 5 --- ", 27);
                                        write(2, (const char *)(pinfo->path), strlen((const char *)(pinfo->path)));
                                        fflush(stderr);
                                }
                                extractSig();
                                ret = dumpToShm();
                                if(ret != 0)
                                {
                                        if (DEBUG)
                                        {
                                                fprintf(stderr, "\n\t[Error] --- Unable to dump HTTP header to shared memory\n\t\tReason: %s\n", ret == CRING ? "CRING" : (ret == PWING ? "PWING" : "Unknown"));
                                        }
                                }
                                else
                                {
                                        if (DEBUG)
                                        {
                                                write(2, "\n\tSuccessfully dumped signature to shared memory\n", 49);
                                        }
                                }
                                ret = 0;
                        }
                }
        }

        /* add this segment to the stream */
        switch ( ( ret = ntoh_tcp_add_segment( tcp_session , stream, iphdr, total_len, (void*)pinfo ) ) )
        {
                case NTOH_OK:
                        break;

                case NTOH_SYNCHRONIZING:
                        free_peer_info ( pinfo );
                        break;

                default:
                        if (DEBUG)
                        {
                                fprintf( stderr, "\n[e] Error %d adding segment: %s", ret, ntoh_get_retval_desc( ret ) );
                        }
                        free_peer_info ( pinfo );
                        break;
        }

        return;
}

/**
 * @brief Sends a IPv4 fragment to libntoh
 */
void send_ipv4_fragment ( struct ip *iphdr , pipv4_dfcallback_t callback )
{
        ntoh_ipv4_tuple4_t 	ipt4;
        pntoh_ipv4_flow_t 	flow;
        size_t			total_len;
        int32_t 			ret;
        uint32_t		error;

        total_len = ntohs( iphdr->ip_len );

        ntoh_ipv4_get_tuple4 ( iphdr , &ipt4 );

        if ( !( flow = ntoh_ipv4_find_flow( ipv4_session , &ipt4 ) ) )
        {
                if ( ! (flow = ntoh_ipv4_new_flow( ipv4_session , &ipt4, callback, 0 , &error )) )
                {
                        if (DEBUG)
                        {
                                fprintf ( stderr , "\n[e] Error %d creating new IPv4 flow: %s" , error , ntoh_get_errdesc ( error ) );
                        }
                        return;
                }
        }

        if ( ( ret = ntoh_ipv4_add_fragment( ipv4_session , flow, iphdr, total_len ) ) )
        {
                if (DEBUG)
                {
                        fprintf( stderr, "\n[e] Error %d adding IPv4: %s", ret, ntoh_get_retval_desc( ret ) );
                }
        }

        return;
}

/**
 * @brief TCP callback function
 */
void tcp_callback ( pntoh_tcp_stream_t stream , pntoh_tcp_peer_t orig , pntoh_tcp_peer_t dest , pntoh_tcp_segment_t seg , int reason , int extra )
{
        /* receive data only from the peer given by the user */
        if ( receive == RECV_CLIENT && stream->server.receive )
        {
                stream->server.receive = 0;
                return;
        }
        else if ( receive == RECV_SERVER && stream->client.receive )
        {
                stream->client.receive = 0;
                return;
        }
        if (DEBUG)
        {

                fprintf ( stderr , "\n[%s] %s:%d (%s | Window: %lu) ---> " , ntoh_tcp_get_status ( stream->status ) , inet_ntoa( *(struct in_addr*) &orig->addr ) , ntohs(orig->port) , ntoh_tcp_get_status ( orig->status ) , orig->totalwin );
                fprintf ( stderr , "%s:%d (%s | Window: %lu)\n\t" , inet_ntoa( *(struct in_addr*) &dest->addr ) , ntohs(dest->port) , ntoh_tcp_get_status ( dest->status ) , dest->totalwin );

                if ( seg != 0 )
                {
                        fprintf ( stderr , "SEQ: %lu ACK: %lu Next SEQ: %lu" , seg->seq , seg->ack , orig->next_seq );
                }
        }

        switch ( reason )
        {
                switch ( extra )
                {
                        case NTOH_REASON_MAX_SYN_RETRIES_REACHED:
                        case NTOH_REASON_MAX_SYNACK_RETRIES_REACHED:
                        case NTOH_REASON_HSFAILED:
                        case NTOH_REASON_EXIT:
                        case NTOH_REASON_TIMEDOUT:
                        case NTOH_REASON_CLOSED:
                                if (DEBUG)
                                {
                                        if ( extra == NTOH_REASON_CLOSED )
                                        {
                                                fprintf ( stderr , "\n\t+ Connection closed by %s (%s)" , stream->closedby == NTOH_CLOSEDBY_CLIENT ? "Client" : "Server" , inet_ntoa( *(struct in_addr*) &(stream->client.addr) ) );
                                        }
                                        else
                                        {
                                                fprintf ( stderr , "\n\t+ %s/%s - %s" , ntoh_get_reason ( reason ) , ntoh_get_reason ( extra ) , ntoh_tcp_get_status ( stream->status ) );
                                        }
                                }

                                break;
                }

                break;

                /* Data segment */
                case NTOH_REASON_DATA:
                if (DEBUG)
                {
                        fprintf ( stderr , " | Data segment | Bytes: %i" , seg->payload_len );

                        if ( extra != 0 )
                        {
                                fprintf ( stderr , "- %s" , ntoh_get_reason ( extra ) );
                        }
                }

                break;
        }

        if ( seg != 0 )
        {
                free_peer_info ( (ppeer_info_t) seg->user_data );
        }

        if (DEBUG)
        {
                write(2, "\n", 1);
        }

        return;
}

/**
 * @brief IPv4 callback function
 */
void ipv4_callback ( pntoh_ipv4_flow_t flow , pntoh_ipv4_tuple4_t tuple , unsigned char *data , size_t len , unsigned short reason )
{
        i = 0;

        if (DEBUG)
        {
                fprintf( stderr, "\n\n[i] Got an IPv4 datagram! (%s) %s --> ", ntoh_get_reason(reason) , inet_ntoa( *(struct in_addr*) &tuple->source ) );
                fprintf( stderr, "%s | %i/%i bytes - Key: %04x - ID: %02x - Proto: %d (%s)\n\n", inet_ntoa( *(struct in_addr*) &tuple->destination ), (int)len, (int)(flow->total) , flow->key, ntohs( tuple->id ), tuple->protocol, get_proto_description( tuple->protocol ) );
        }

        if ( tuple->protocol == IPPROTO_TCP )
        {
                send_tcp_segment ( (struct ip*) data , &tcp_callback );
        }
        else
        {
                if (DEBUG)
                {
                        for ( i = 0; i < flow->total ; i++ )
                        {
                                fprintf( stderr, "%02x ", data[i] );
                        }
                }
        }

        if (DEBUG)
        {
                write(2, "\n", 1);
        }

        return;
}

int main (int argc , char *argv[])
{
        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );
        signal( SIGSEGV, &shandler );

        write(2, "\n\t\t######################################", 41);
        write(2, "\n\t\t#           Dump HTTP Sigs           #", 41);
        write(2, "\n\t\t# ---------------------------------- #", 41);
        write(2, "\n\t\t#     Written by Ernest Richards     #", 41);
        write(2, "\n\t\t#  Based on code from Chema Garcia   #", 41);
        write(2, "\n\t\t# ---------------------------------- #", 41);
        write(2, "\n\t\t# Github.com/ernesto341/ais-research #", 41);
        write(2, "\n\t\t######################################\n", 42);
        write(2, "\n\t\tX      -----    Active    -----      X\n\n", 43);

        if (DEBUG)
        {
                sprintf(buf, "\n[i] libntoh version: %s\n", ntoh_version());
                write(2, buf, strlen(buf));
        }

        if ( argc < 3 )
        {
                sprintf(buf, "\n[+] Usage: %s <options>\n", argv[0]);
                write(2, buf, strlen(buf));
                write(2, "\n+ Options:", 11);      // 28
                write(2, "\n\t-i | --iface <val> -----> Interface to read packets from", 58);
                write(2, "\n\t-f | --file <val> ------> File path to read packets from", 58);
                write(2, "\n\t-F | --filter <val> ----> Capture filter (must contain \"tcp\" or \"ip\")", 71);
                write(2, "\n\t-c | --client ----------> Receive client data only", 52);
                write(2, "\n\t-s | --server ----------> Receive server data only\n\n", 54);
                exit(1);
        }

        /* parameters parsing */
        int32_t c = 0;

        /* pcap */
        char errbuf[PCAP_ERRBUF_SIZE];
        struct bpf_program fp;
        char filter_exp[] = "ip";
        char *source = 0;
        char *filter = filter_exp;
        const unsigned char *packet = 0;
        struct pcap_pkthdr header;

        /* packet dissection */
        struct ip	*ip;
        uint32_t error = 0;

        /* extra */
        uint32_t ipf, tcps;

        /* check parameters */
        while ( c >= 0 )
        {
                int32_t option_index = 0;
                static struct option long_options[] =
                {
                        { "iface" , 1 , 0 , 'i' },
                        { "file" , 1 , 0 , 'f' },
                        { "filter" , 1 , 0 , 'F' },
                        { "client" , 0 , 0 , 'c' },
                        { "server" , 0 , 0 , 's' },
                        { 0 , 0 , 0 , 0 }
                };

                c = getopt_long( argc, argv, "i:f:F:cs", long_options, &option_index );

                if (c >= 0)
                {

                        switch ( c )
                        {
                                case 'i':
                                        source = optarg;
                                        handle = pcap_open_live( optarg, 65535, 1, 0, errbuf );
                                        break;

                                case 'f':
                                        source = optarg;
                                        handle = pcap_open_offline( optarg, errbuf );
                                        break;

                                case 'F':
                                        filter = optarg;
                                        break;

                                case 'c':
                                        receive |= RECV_CLIENT;
                                        break;

                                case 's':
                                        receive |= RECV_SERVER;
                                        break;
                        }
                }
        }

        if ( !receive )
        {
                receive = (RECV_CLIENT | RECV_SERVER);
        }

        if ( !handle )
        {
                if (DEBUG)
                {
                        fprintf( stderr, "\n[e] Error loading %s: %s\n", source, errbuf );
                }
                exit( -1 );
        }

        if ( pcap_compile( handle, &fp, filter, 0, 0 ) < 0 )
        {
                if (DEBUG)
                {
                        fprintf( stderr, "\n[e] Error compiling filter \"%s\": %s\n\n", filter, pcap_geterr( handle ) );
                }
                pcap_close( handle );
                exit( -2 );
        }

        if ( pcap_setfilter( handle, &fp ) < 0 )
        {
                if (DEBUG)
                {
                        fprintf( stderr, "\n[e] Cannot set filter \"%s\": %s\n\n", filter, pcap_geterr( handle ) );
                }
                pcap_close( handle );
                exit( -3 );
        }
        pcap_freecode( &fp );

        /* verify datalink */
        if ( pcap_datalink( handle ) != DLT_EN10MB )
        {
                if (DEBUG)
                {
                        fprintf ( stderr , "\n[e] libntoh is independent from link layer, but this code only works with ethernet link layer\n");
                }
                pcap_close ( handle );
                exit ( -4 );
        }

        if (DEBUG)
        {
                fprintf( stderr, "\n[i] Source: %s / %s", source, pcap_datalink_val_to_description( pcap_datalink( handle ) ) );
                fprintf( stderr, "\n[i] Filter: %s", filter );

                fprintf( stderr, "\n[i] Receive data from client: ");
                if ( receive & RECV_CLIENT )
                {
                        fprintf( stderr , "Yes");
                }
                else
                {
                        fprintf( stderr , "No");
                }

                fprintf( stderr, "\n[i] Receive data from server: ");
                if ( receive & RECV_SERVER )
                {
                        fprintf( stderr , "Yes");
                }
                else
                {
                        fprintf( stderr , "No");
                }
        }

        /*******************************************/
        /** libntoh initialization process starts **/
        /*******************************************/

        initMem(&snc);

        if ((t5Convert = (sig_atomic_t *)malloc(sizeof(sig_atomic_t) * t5TplLen)) < (sig_atomic_t *)0)
        {
                write(2, "\n\t[e] --- Unable to allocate sufficient memory\n", 47);
                fflush(stderr);
                _exit(-1);
        }

        ntoh_init ();

        if ( ! (tcp_session = ntoh_tcp_new_session ( 0 , 0 , &error ) ) )
        {
                if (DEBUG)
                {
                        fprintf ( stderr , "\n[e] Error %d creating TCP session: %s" , error , ntoh_get_errdesc ( error ) );
                }
                exit ( -5 );
        }

        if (DEBUG)
        {
                fprintf ( stderr , "\n[i] Max. TCP streams allowed: %d" , ntoh_tcp_get_size ( tcp_session ) );
        }

        if ( ! (ipv4_session = ntoh_ipv4_new_session ( 0 , 0 , &error )) )
        {
                ntoh_tcp_free_session ( tcp_session );
                if (DEBUG)
                {
                        fprintf ( stderr , "\n[e] Error %d creating IPv4 session: %s" , error , ntoh_get_errdesc ( error ) );
                }
                exit ( -6 );
        }

        if (DEBUG)
        {
                fprintf ( stderr , "\n[i] Max. IPv4 flows allowed: %d\n\n" , ntoh_ipv4_get_size ( ipv4_session ) );

                fflush(stderr);
        }

        /* capture starts */
        /* accept signal from consumer to quit */
        while ( ( packet = pcap_next( handle, &header ) ) != 0 && snc.smem.shm[CTL][FLAGS] != CDONE)
        {
                /* get packet headers */
                ip = (struct ip*) ( packet + sizeof ( struct ether_header ) );
                if ( (ip->ip_hl * 4 ) < (int)sizeof(struct ip) )
                {
                        continue;
                }

                /* it is an IPv4 fragment */
                if ( NTOH_IPV4_IS_FRAGMENT(ip->ip_off) )
                {
                        send_ipv4_fragment ( ip , &ipv4_callback );
                }
                /* or a TCP segment */
                else if ( ip->ip_p == IPPROTO_TCP )
                {
                        send_tcp_segment ( ip , &tcp_callback );
                }
        }
        if (snc.smem.shm[CTL][FLAGS] == CDONE)
        {
                shandler( 0 );
        }

        tcps = ntoh_tcp_count_streams( tcp_session );
        ipf = ntoh_ipv4_count_flows ( ipv4_session );

        /* no streams left */
        if ( ipf + tcps > 0 )
        {
                if (DEBUG)
                {
                        fprintf( stderr, "\n\n[+] There are currently %i stored TCP stream(s) and %i IPv4 flow(s). You can wait for them to get closed or press CTRL+C\n" , tcps , ipf );
                        pause();
                }
        }

        shandler( 0 );

        //dummy return, should never be called
        return (0);
}
