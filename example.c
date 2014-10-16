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

/*
 * PWING = 1 --- Producer is writing to shared memory
 * PWTEN = 2 --- Producer has written to shared memory
 * CRING = 3 --- Consumer is reading from shared memory
 * CREAD = 4 --- Consumer has read from shared memory
 *
 *
 * TO DO:
 *
 * in dumptoshm routine,
 *    use try_lock instead of lock
 *    use sigs to buffer
 *    sigs gets incremented when a new signature is discovered
 *    shm gets incremented when a signature is dumped from sigs to shm
 *    track buffered in following way:
 *
 *       if lock
 *          do
 *             regular dump
 *             inCtr(shm)
 *          while shm[0] != sigs[0]
 *
 *    need to check that after an overwrite
 *    everything still operates the same,
 *    or if special consideration needs to be
 *    taken
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

#ifndef fngPntLen
#define fngPntLen 14
#endif

#ifndef _t5TplLen
#define _t5TplLen
static const uint32_t t5TplLen = 44;
#endif

typedef struct
{
        unsigned char *data;
        size_t data_len;
        char *path;
} peer_info_t , *ppeer_info_t;

#define RECV_CLIENT	1
#define RECV_SERVER	2

#define SIGQTY 5

/* capture handle */
pcap_t 					*handle = 0;
pntoh_tcp_session_t		tcp_session = 0;
pntoh_ipv4_session_t	ipv4_session = 0;
unsigned short			receive = 0;

/* header extract and signiture storage, memory */
uint8_t pending_more_hdr_data = 0;
unsigned char * hdr_data = NULL;
/* low end of average HTTP header size is 200
 * high end can be over 2kb, hdr_data size will auto adjust as the program runs
 * 850 given as sufficient for most headers to minimize resize operations
 * thanks to stackoverflow forum, typo.pl
 */
uint32_t hdr_size = 850;
uint32_t shmkey[6] = {6511, 5433, 9884, 1763, 5782, 6284};
uint32_t t5shmkey[5] = {959, 653, 987, 627, 905};
int * shmid = NULL;
int * t5shmid = NULL;
int ** shm = NULL;
int ** sigs = NULL;
char ** t5s = NULL;
char ** t5shm = NULL;

inline static void inCtr(int ** s)
{
        (s[0][0])++;
        (s[0][0]) = (((s[0][0]) % SIGQTY) != 0 ? ((s[0][0]) % SIGQTY) : 5); // 1 - 5
}

/**
 * @brief Exit function (closes the capture handle and releases all resource from libntoh)
 */
void shandler ( int sign )
{
        if ( sign != 0 )
                signal ( sign , &shandler );

        freeMem();

        pcap_close( handle );

        ntoh_exit();

        fprintf( stderr, "\n\n[+] Capture finished!\n" );
        exit( sign );
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

        return ret;
}

/**
 * @brief Frees the ppeer_info_t struct
 */
void free_peer_info ( ppeer_info_t pinfo )
{
        /* free peer info data */
        if ( ! pinfo )
                return;

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
                        return "ICMP";

                case IPPROTO_TCP:
                        return "TCP";

                case IPPROTO_UDP:
                        return "UDP";

                case IPPROTO_IGMP:
                        return "IGMP";

                case IPPROTO_IPV6:
                        return "IPv6";

                case IPPROTO_FRAGMENT:
                        return "IPv6 Fragment";

                default:
                        return "Undefined";
        }
}

void generic_write_data ( void * data )
{
        if (data == NULL || strlen((char *)data) < 1)
        {
                fprintf( stderr, "No data passed to generic_write_data\n");
                return;
        }
        int fd = 0;

        char path [2056];
        strcpy (path, "/home/ernest/research/");
        strcat (path, "generic_dump\0");

        if ( (fd = open ( path , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                fprintf ( stderr , "\n[e] Error opening data file \"%s\"" , path );
                return;
        }

        strcpy (path, "\0");
        strcpy (path, (char *)data);
        strcat (path, "\n\0");

        write ( fd , path , strlen(path) );
        close ( fd );
        return;
}


void write_hdr_data ( void )
{
        if (hdr_data == NULL || strlen((char *)hdr_data) < 1)
        {
                return;
        }
        int fd = 0;

        char path [10240];
        strcpy (path, "/home/ernest/research/");
        strcat (path, "hdr_and_sig\0");

        if ( (fd = open ( path , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                fprintf ( stderr , "\n[e] Error opening data file \"%s\"" , path );
                return;
        }

        // write ( fd , (char *)hdr_data , strlen((char *)hdr_data) );

        //write ( fd , "\n" , 1 );

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

        strcpy (path, "Finger Print:");
        if (sigs == NULL || sigs[(sigs[0][0])] == NULL)
        {
                strcat(path, "\nNo fingerprint found\n\0");
                write(fd, path, strlen(path));
                close ( fd );
                pending_more_hdr_data = 0;
                //free(hdr_data);
                //hdr_data = NULL;
                //hdr_size = 850;
                return;
        }
        int i = 0;
        while (i < fngPntLen)
        {
                strcpy (path, "\n");
                if (i == 0)
                {
                        strcat (path, "CMD  -  ");
                }
                else if (i == 1)
                {
                        strcat (path, "PROT -  ");
                }
                else if (i == 2)
                {
                        strcat (path, "LEN  -  ");
                }
                else if (i == 3)
                {
                        strcat (path, "VARS -  ");
                }
                else if (i == 4)
                {
                        strcat (path, "%    -  ");
                }
                else if (i == 5)
                {
                        strcat (path, "'    -  ");
                }
                else if (i == 6)
                {
                        strcat (path, "+    -  ");
                }
                else if (i == 7)
                {
                        strcat (path, "..   -  ");
                }
                else if (i == 8)
                {
                        strcat (path, "\\    -  ");
                }
                else if (i == 9)
                {
                        strcat (path, "(    -  ");
                }
                else if (i == 10)
                {
                        strcat (path, ")    -  ");
                }
                else if (i == 11)
                {
                        strcat (path, "//   -  ");
                }
                else if (i == 12)
                {
                        strcat (path, "<    -  ");
                }
                else if (i == 13)
                {
                        strcat (path, ">    -  ");
                }
                strcat (path, itoa(sigs[((sigs[0][0])-1) > 0 ? (sigs[0][0])-1 : SIGQTY-((sigs[0][0])-1)][i]));
                strcat (path, "\n");
                write ( fd , path , strlen(path) );
                i++;
        }

        strcpy (path, "");
        write ( fd , "\n" , 1 );
        close ( fd );
        pending_more_hdr_data = 0;
        strcpy((char *)hdr_data, "");
        //free(hdr_data);
        //hdr_data = NULL;
        //hdr_size = 850;

        return;
}

/**
 * @brief Writes the ppeer_info_t data field to disk
 */
void write_data ( ppeer_info_t info )
{
        int fd = 0;

        if ( !info )
                return;

        if ( (fd = open ( info->path , O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW , S_IRWXU | S_IRWXG | S_IRWXO )) < 0 )
        {
                fprintf ( stderr , "\n[e] Error %d writting data to \"%s\": %s" , errno , info->path , strerror( errno ) );
                return;
        }

        write ( fd , info->data , info->data_len );
        close ( fd );

        return;
}

//create the firnger print
int * pcktFingerPrint(unsigned char * curPcktData, unsigned int dataLen)
{
        if(curPcktData == NULL || dataLen == 0 )
        {
                printf("There was not header to parse\n");
                return NULL;
        }

        int i = 0;
        int cmd = 8;
        int cmdSet = 0;
        int proto = 8;
        int protoSet = 0;
        int len = dataLen;   //calculated during parsing
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
        unsigned char *target = curPcktData;
        int *fngPnt = malloc(sizeof(int) * fngPntLen);

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
                                target--;
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
                                        target--;
                        }
                        else
                                target--;
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
                                                target--;
                                }
                                else
                                        target--;
                        }
                        else
                                target--;
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
                                                target--;
                                }
                                else
                                        target--;
                        }
                        else
                                target--;
                }

                if(*target == 46)                    //.. counter
                {
                        target++;
                        if(*target == 46)
                                cdot++;
                        else
                                target--;
                }

                if(*target == 47)
                {                       // // counter
                        target++;
                        if(*target == 47)
                                fwrd++;
                        else
                                target--;
                }

                if(*target == 63){                        //conditional for variables
                        qstnmrk = 1;
                        //      continue;
                }

                else if(*target == 37){                        //percent counter
                        pcnt++;
                        //        continue;
                }

                else if (qstnmrk == 1 && *target == 38){        //variable counter
                        var++;
                        //          continue;
                }

                else if(*target == 39){                        //apostrophe counter
                        apos++;
                        //            continue;
                }

                else if(*target == 43){                        //addition counter
                        plus++;
                        //              continue;
                }

                else if(*target == 40){                        //open parentheses counter
                        oparen++;
                        //                continue;
                }

                else if(*target == 41){                        //close parentheses counter
                        cparen++;
                        //                  continue;
                }

                else if(*target == 60){                        //less than counter
                        lt++;
                        //                    continue;
                }

                else if(*target == 62){                        //greater than counter
                        gt++;
                        //                      continue;
                }

                else if(*target == 92){                        //backslash counter
                        bckslsh++;
                        //                        continue;
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

        return fngPnt;
}

inline static void resizeHdr(void)
{
        unsigned char * tmp = NULL;
        tmp = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size * 2);
        memcpy(tmp, hdr_data, hdr_size);
        free(hdr_data);
        hdr_data = tmp;
        hdr_size *= 2;
}

uint8_t extractHttpHdr (const char * udata)
{
        unsigned int i = 0, j = 0;
        if (pending_more_hdr_data != 0)
        {
                j = strlen((char *)hdr_data);
        }
        if (hdr_data == NULL)
        {
                hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size * 5);
                //hdr_data = (unsigned char *)malloc(sizeof(unsigned char) * hdr_size);
        }
        uint8_t eoh = 1;
        unsigned int data_len = strlen((char *)udata);
        while (i < data_len && eoh != 0)
        {
                hdr_data[j++] = udata[i++];
                /* HERE *
                 * not sure about effectiveness of these statement
                 * when i dump the header, i get the entire stream
                 */
                if (i > 3 && (udata[i] == 0x0a && udata[i-1] == 0x0d && udata[i-2] == 0x0a && udata[i-3] == 0x0d))
                {
                        eoh = 0;
                }
                else if (i > 3 && (udata[i] == 0x0d && udata[i-1] == 0x0a && udata[i-2] == 0x0d && udata[i-3] == 0x0a))
                {
                        eoh = 0;
                }
                else if (j >= hdr_size-1)
                {
                        resizeHdr();
                }
        }
        return (eoh);
}

inline uint8_t dumpToShm(void)
{
        //        fprintf(stderr, "in dumpToShm, shm[0][0] = %d\r\n", shm[0][0]);
        //        fflush(stderr);
        /* lock shared memory */
        if (shm[0][FLAGS] == CREAD || shm[0][FLAGS] == 0)
        {
                shm[0][FLAGS] = PWING;
                /* do the dump to shm routine */
                while (shm[0][0] != sigs[0][0])
                {
                        memcpy(shm[((shm[0][0]))], sigs[(shm[0][0])], (sizeof(int) * fngPntLen));
                        memcpy(t5shm[((shm[0][0] - 1))], t5s[(shm[0][0]) - 1], (sizeof(char) * 44));
                        if ((shm[0][1]) == 5)
                        {
                                /* unlock shared memory */
                                shm[0][FLAGS] = PWTEN;
                                write(2, "\n\t[ALERT]\t---\tOverwriting a signature!\n\n", 40);
                        }
                        else
                        {
                                /* unlock shared memory */
                                shm[0][FLAGS] = PWTEN;
                                shm[0][1] += 1; // pending
                        }
                        inCtr(shm); // pos
                }
                /* reset vars */
                pending_more_hdr_data = 0;
                return (0);
        }
        else
        {
                fprintf(stderr, "%d\r\n", shm[0][FLAGS]);
                fflush(stderr);
                return ((uint8_t)(shm[0][FLAGS]));
        }
}

inline static void extractSig(void)
{
        sigs[(sigs[0][0])] = pcktFingerPrint(hdr_data, strlen((char *)hdr_data));
        inCtr(sigs);
}

inline uint32_t getSrcPrt(const char * path)
{
        int i = 0;
        while (path[i] != ':')
        {
                i++;
        }
        i++;
        return ((uint32_t)(atoi((char *)(&(path[i])))));
}

inline uint32_t getDstPrt(const char * path)
{
        int i = 0;
        while (path[i] != ':')
        {
                i++;
        }
        while (path[i] != ':')
        {
                i++;
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
        int					ret;
        unsigned int		error;

        size_ip = iphdr->ip_hl * 4;
        total_len = ntohs( iphdr->ip_len );

        tcp = (struct tcphdr*)((unsigned char*)iphdr + size_ip);
        if ( (size_tcp = tcp->th_off * 4) < sizeof(struct tcphdr) )
                return;

        payload = (unsigned char *)iphdr + size_ip + size_tcp;
        size_payload = total_len - ( size_ip + size_tcp );

        ntoh_tcp_get_tuple5 ( iphdr , tcp , &tcpt5 );

        /* find the stream or creates a new one */
        if ( !( stream = ntoh_tcp_find_stream( tcp_session , &tcpt5 ) ) )
        {
                if ( ! ( stream = ntoh_tcp_new_stream( tcp_session , &tcpt5, callback , 0 , &error , 1 , 1 ) ) )
                {
                        fprintf ( stderr , "\n[e] Error %d creating new stream: %s" , error , ntoh_get_errdesc ( error ) );
                        return;
                }
        }

        if ( size_payload > 0 )
                pinfo = get_peer_info ( payload , size_payload , &tcpt5 );
        else
                pinfo = 0;

        if (pinfo != 0)
        {
                /* HERE 
                 *
                 * maybe replace dst port with Contians(header, "HTTP")
                 *
                 * */
                uint32_t port = getDstPrt((pinfo->path));
                if (port == 80 || port == 8080)
                {
                        pending_more_hdr_data = extractHttpHdr((const char *)(payload));
                        /* got entire header, dump to shm */
                        if (pending_more_hdr_data == 0)
                        {
                                /* lock memory */
                                memcpy(t5s[(shm[0][0]) - 1], (char *)(pinfo->path), strlen(pinfo->path));
                                /* unlock memory */
                                //                                write(2, "\r\n\ttcp tuple 5\t---\t", 19);
                                //                                write(2, (char *)(pinfo->path), strlen(pinfo->path));
                                //                                fflush(stderr);
                                extractSig();
                                ret = dumpToShm();
                                if(ret != 0)
                                {
                                        fprintf(stderr, "\n\t[Error] --- Unable to dump HTTP header to shared memory\n\t\tReason: %s\n", ret == CRING ? "CRING" : (ret == PWING ? "PWING" : "Unknown"));
                                }
                                else
                                {
                                        fprintf(stderr, "\n\tSuccessfully dumped signature to shared memory\n");
                                }
                                ret = 0;
                                //write_hdr_data();
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
                        fprintf( stderr, "\n[e] Error %d adding segment: %s", ret, ntoh_get_retval_desc( ret ) );
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
        int 			ret;
        unsigned int		error;

        total_len = ntohs( iphdr->ip_len );

        ntoh_ipv4_get_tuple4 ( iphdr , &ipt4 );

        if ( !( flow = ntoh_ipv4_find_flow( ipv4_session , &ipt4 ) ) )
                if ( ! (flow = ntoh_ipv4_new_flow( ipv4_session , &ipt4, callback, 0 , &error )) )
                {
                        fprintf ( stderr , "\n[e] Error %d creating new IPv4 flow: %s" , error , ntoh_get_errdesc ( error ) );
                        return;
                }

        if ( ( ret = ntoh_ipv4_add_fragment( ipv4_session , flow, iphdr, total_len ) ) )
                fprintf( stderr, "\n[e] Error %d adding IPv4: %s", ret, ntoh_get_retval_desc( ret ) );

        return;
}

/* TCP Callback */
void tcp_callback ( pntoh_tcp_stream_t stream , pntoh_tcp_peer_t orig , pntoh_tcp_peer_t dest , pntoh_tcp_segment_t seg , int reason , int extra )
{
        /* receive data only from the peer given by the user */
        if ( receive == RECV_CLIENT && stream->server.receive )
        {
                stream->server.receive = 0;
                return;
        }else if ( receive == RECV_SERVER && stream->client.receive )
        {
                stream->client.receive = 0;
                return;
        }

        fprintf ( stderr , "\n[%s] %s:%d (%s | Window: %lu) ---> " , ntoh_tcp_get_status ( stream->status ) , inet_ntoa( *(struct in_addr*) &orig->addr ) , ntohs(orig->port) , ntoh_tcp_get_status ( orig->status ) , orig->totalwin );
        fprintf ( stderr , "%s:%d (%s | Window: %lu)\n\t" , inet_ntoa( *(struct in_addr*) &dest->addr ) , ntohs(dest->port) , ntoh_tcp_get_status ( dest->status ) , dest->totalwin );

        if ( seg != 0 )
                fprintf ( stderr , "SEQ: %lu ACK: %lu Next SEQ: %lu" , seg->seq , seg->ack , orig->next_seq );

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
                                if ( extra == NTOH_REASON_CLOSED )
                                        fprintf ( stderr , "\n\t+ Connection closed by %s (%s)" , stream->closedby == NTOH_CLOSEDBY_CLIENT ? "Client" : "Server" , inet_ntoa( *(struct in_addr*) &(stream->client.addr) ) );
                                else
                                        fprintf ( stderr , "\n\t+ %s/%s - %s" , ntoh_get_reason ( reason ) , ntoh_get_reason ( extra ) , ntoh_tcp_get_status ( stream->status ) );

                                break;
                }

                break;

                /* Data segment */
                case NTOH_REASON_DATA:
                fprintf ( stderr , " | Data segment | Bytes: %i" , seg->payload_len );

                if ( extra != 0 )
                        fprintf ( stderr , "- %s" , ntoh_get_reason ( extra ) );

                break;
        }

        if ( seg != 0 )
                free_peer_info ( (ppeer_info_t) seg->user_data );

        fprintf ( stderr , "\n" );

        return;
}

/* IPv4 Callback */
void ipv4_callback ( pntoh_ipv4_flow_t flow , pntoh_ipv4_tuple4_t tuple , unsigned char *data , size_t len , unsigned short reason )
{
        unsigned int i = 0;

        fprintf( stderr, "\n\n[i] Got an IPv4 datagram! (%s) %s --> ", ntoh_get_reason(reason) , inet_ntoa( *(struct in_addr*) &tuple->source ) );
        fprintf( stderr, "%s | %i/%i bytes - Key: %04x - ID: %02x - Proto: %d (%s)\n\n", inet_ntoa( *(struct in_addr*) &tuple->destination ), (int)len, (int)(flow->total) , flow->key, ntohs( tuple->id ), tuple->protocol, get_proto_description( tuple->protocol ) );

        if ( tuple->protocol == IPPROTO_TCP )
                send_tcp_segment ( (struct ip*) data , &tcp_callback );
        else
                for ( i = 0; i < flow->total ; i++ )
                        fprintf( stderr, "%02x ", data[i] );

        fprintf( stderr, "\n" );

        return;
}

int main ( int argc , char *argv[] )
{
        /* parameters parsing */
        int c;

        /* pcap */
        char 				errbuf[PCAP_ERRBUF_SIZE];
        struct bpf_program 	fp;
        char 				filter_exp[] = "ip";
        char 				*source = 0;
        char 				*filter = filter_exp;
        const unsigned char *packet = 0;
        struct pcap_pkthdr 	header;

        /* packet dissection */
        struct ip		*ip;
        unsigned int	error;

        /* extra */
        unsigned int ipf,tcps;

        fprintf( stderr, "\n###########################" );
        fprintf( stderr, "\n#     libntoh Example     #" );
        fprintf( stderr, "\n# ----------------------- #" );
        fprintf( stderr, "\n# Written by Chema Garcia #" );
        fprintf( stderr, "\n# ----------------------- #" );
        fprintf( stderr, "\n#  http://safetybits.net  #" );
        fprintf( stderr, "\n#   chema@safetybits.net  #" );
        fprintf( stderr, "\n#   sch3m4@brutalsec.net  #" );
        fprintf( stderr, "\n###########################\n" );

        fprintf( stderr, "\n[i] libntoh version: %s\n", ntoh_version() );

        if ( argc < 3 )
        {
                fprintf( stderr, "\n[+] Usage: %s <options>\n", argv[0] );
                fprintf( stderr, "\n+ Options:" );
                fprintf( stderr, "\n\t-i | --iface <val> -----> Interface to read packets from" );
                fprintf( stderr, "\n\t-f | --file <val> ------> File path to read packets from" );
                fprintf( stderr, "\n\t-F | --filter <val> ----> Capture filter (must contain \"tcp\" or \"ip\")" );
                fprintf( stderr, "\n\t-c | --client ----------> Receive client data only");
                fprintf( stderr, "\n\t-s | --server ----------> Receive server data only\n\n");
                exit( 1 );
        }

        /* check parameters */
        while ( 1 )
        {
                int option_index = 0;
                static struct option long_options[] =
                {
                        { "iface" , 1 , 0 , 'i' } ,
                        { "file" , 1 , 0 , 'f' } ,
                        { "filter" , 1 , 0 , 'F' } ,
                        { "client" , 0 , 0 , 'c' },
                        { "server" , 0 , 0 , 's' },
                        { 0 , 0 , 0 , 0 } };

                if ( ( c = getopt_long( argc, argv, "i:f:F:cs", long_options, &option_index ) ) < 0 )
                        break;

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

        if ( !receive )
                receive = (RECV_CLIENT | RECV_SERVER);

        if ( !handle )
        {
                fprintf( stderr, "\n[e] Error loading %s: %s\n", source, errbuf );
                exit( -1 );
        }

        if ( pcap_compile( handle, &fp, filter, 0, 0 ) < 0 )
        {
                fprintf( stderr, "\n[e] Error compiling filter \"%s\": %s\n\n", filter, pcap_geterr( handle ) );
                pcap_close( handle );
                exit( -2 );
        }

        if ( pcap_setfilter( handle, &fp ) < 0 )
        {
                fprintf( stderr, "\n[e] Cannot set filter \"%s\": %s\n\n", filter, pcap_geterr( handle ) );
                pcap_close( handle );
                exit( -3 );
        }
        pcap_freecode( &fp );

        /* verify datalink */
        if ( pcap_datalink( handle ) != DLT_EN10MB )
        {
                fprintf ( stderr , "\n[e] libntoh is independent from link layer, but this example only works with ethernet link layer\n");
                pcap_close ( handle );
                exit ( -4 );
        }

        fprintf( stderr, "\n[i] Source: %s / %s", source, pcap_datalink_val_to_description( pcap_datalink( handle ) ) );
        fprintf( stderr, "\n[i] Filter: %s", filter );

        fprintf( stderr, "\n[i] Receive data from client: ");
        if ( receive & RECV_CLIENT )
                fprintf( stderr , "Yes");
        else
                fprintf( stderr , "No");

        fprintf( stderr, "\n[i] Receive data from server: ");
        if ( receive & RECV_SERVER )
                fprintf( stderr , "Yes");
        else
                fprintf( stderr , "No");

        signal( SIGINT, &shandler );
        signal( SIGTERM, &shandler );

        /*******************************************/
        /** libntoh initialization process starts **/
        /*******************************************/

        initMem();
        ntoh_init ();

        if ( ! (tcp_session = ntoh_tcp_new_session ( 0 , 0 , &error ) ) )
        {
                fprintf ( stderr , "\n[e] Error %d creating TCP session: %s" , error , ntoh_get_errdesc ( error ) );
                exit ( -5 );
        }

        fprintf ( stderr , "\n[i] Max. TCP streams allowed: %d" , ntoh_tcp_get_size ( tcp_session ) );

        if ( ! (ipv4_session = ntoh_ipv4_new_session ( 0 , 0 , &error )) )
        {
                ntoh_tcp_free_session ( tcp_session );
                fprintf ( stderr , "\n[e] Error %d creating IPv4 session: %s" , error , ntoh_get_errdesc ( error ) );
                exit ( -6 );
        }

        fprintf ( stderr , "\n[i] Max. IPv4 flows allowed: %d\n\n" , ntoh_ipv4_get_size ( ipv4_session ) );

        fflush(stderr);

        shm[0][0] = 1;
        sigs[0][0] = 1;
        shm[0][1] = 0;
        sigs[0][1] = 0;
        shm[0][FLAGS] = 0;
        sigs[0][FLAGS] = 0;


        /* capture starts */
        while ( ( packet = pcap_next( handle, &header ) ) != 0 )
        {
                /* get packet headers */
                ip = (struct ip*) ( packet + sizeof ( struct ether_header ) );
                if ( (ip->ip_hl * 4 ) < (int)sizeof(struct ip) )
                        continue;

                /* it is an IPv4 fragment */
                if ( NTOH_IPV4_IS_FRAGMENT(ip->ip_off) )
                        send_ipv4_fragment ( ip , &ipv4_callback );
                /* or a TCP segment */
                else if ( ip->ip_p == IPPROTO_TCP )
                {
                        send_tcp_segment ( ip , &tcp_callback );
                }
        }

        tcps = ntoh_tcp_count_streams( tcp_session );
        ipf = ntoh_ipv4_count_flows ( ipv4_session );

        /* no streams left */
        if ( ipf + tcps > 0 )
        {
                fprintf( stderr, "\n\n[+] There are currently %i stored TCP stream(s) and %i IPv4 flow(s). You can wait them to get closed or press CTRL+C\n" , tcps , ipf );
                pause();
        }

        shandler( 0 );

        //dummy return
        return 0;
}
