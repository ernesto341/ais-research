/* globals.h */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static const uint8_t CTL = 0;
static const uint8_t POS = 0;
static const uint8_t PEND = 1;
static const uint8_t FLAGS = 2;

static const uint8_t PWING = 1;
static const uint8_t PWTEN = 2;
static const uint8_t PDONE = 3;
static const uint8_t CRING = 4;
static const uint8_t CREAD = 5;
static const uint8_t CDONE = 6;

static const uint32_t fngPntLen = 14;
static const uint32_t t5TplLen = 44;

static const uint32_t SIGQTY = 5;

//static uint8_t pending_more_hdr_data = 0;
//static unsigned char * hdr_data = 0;
//static uint32_t hdr_size = 850;

#endif

/* globals.h */

