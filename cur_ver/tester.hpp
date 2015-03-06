#pragma once
#define TARGET "http://localhost:80"

#ifndef _NORMALFILE
#define _NORMALFILE
const char * NORMALFILE = "./ais/normal\0";
#endif

#ifndef _ATTACKFILE
#define _ATTACKFILE
const char * ATTACKFILE = "./ais/attack\0";
#endif

#ifndef _CMD
#define _CMD
static const char CMD[] = "curl ";
#endif

#ifndef _KILL
#define _KILL
static const char KILL[] = "pkill curl\0";
#endif

