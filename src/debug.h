#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG
//#define DEBUG_PORT Serial

#ifdef DEBUG

#ifndef DEBUG_PORT
#define DEBUG_PORT Serial1
#endif

#define DBUGF(...)  DEBUG_PORT.printf(__VA_ARGS__)
#define DBUG(...)   DEBUG_PORT.print(__VA_ARGS__)
#define DBUGLN(...) DEBUG_PORT.println(__VA_ARGS__)

#else

#define DBUGF(...)
#define DBUG(...)
#define DBUGLN(...)

#endif // DEBUG

#endif // __DEBUG_H
