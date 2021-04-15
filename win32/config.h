#ifndef __CONFIG_H_INCLUDE__
#define __CONFIG_H_INCLUDE__

#ifdef WIN32
#define HAVE_WINSOCK_H
#define HAVE_WINSOCK2_H
#endif

#ifdef WIN32
#define HAVE_GETOPT_H
#endif

#ifdef WIN32
#define HAVE_DIRENT_H
#endif

#ifdef WIN32
#define HAVE_IO_H
#endif

#ifdef WIN32
#undef HAVE_PID_T
#endif

#ifdef WIN32
#define HAVE_DIRECT_H
#endif

#ifdef WIN32
#define vsprintf        _vsprintf
#define snprintf        _snprintf
#define strncasecmp     _strnicmp
#define strcasecmp      _stricmp
#endif

#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>   /* abs() */
#include <string.h>
#include <winsock2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef HAVE_IPv6
#include <ws2ip6.h>
#endif
#include <ws2tcpip.h>
#include <mmreg.h>
#include <msacm.h>
#include <mmsystem.h>
#include <windows.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <time.h>

typedef int             ttl_t;
typedef u_int           fd_t;
typedef unsigned char   byte;

#define srand48         srand
#define lrand48         rand

/*
 * the definitions below are valid for 32-bit architectures and will have to
 * be adjusted for 16- or 64-bit architectures
 */
typedef u_char          u_int8;
typedef u_short         u_int16;
typedef u_long          u_int32;
typedef char            int8;
typedef short           int16;
typedef long            int32;
typedef __int64         int64;
typedef unsigned long   in_addr_t;

#ifndef TRUE
#define FALSE   0
#define	TRUE	!(FALSE)
#endif /* TRUE */

#define MAXHOSTNAMELEN	256

#define NEED_INET_ATON
#define NEED_INET_PTON
#define NEED_IN6_IS_ADDR_MULTICAST

#define STDC_HEADERS    1

/* gettimeofday support */
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
int gettimeofday(struct timeval *tp, struct timezone *tz);

#define ENABLE_VB_TRANSLATION   1

#endif /* __CONFIG_H_INCLUDE__ */
