/* file: read.h */

#ifndef READ_H__
#define READ_H__

#include "read2ch.h"

#ifdef USE_MMAP
#ifndef MAP_FAILED
#define MAP_FAILED (void *)(-1)
#endif
#endif

enum html_error_t {
	ERROR_TOO_HUGE,
	ERROR_NOT_FOUND,
	ERROR_NO_MEMORY,
	ERROR_MAINTENANCE,
	ERROR_LOGOUT,
#ifdef	Katjusha_DLL_REPLY
	ERROR_ABORNED,
#endif
};

#ifdef ZLIB

#ifndef GZIP
# define GZIP			/* gzip由来のコードも使用するので */
#endif
#include <zlib.h>

typedef int (*zz_printf_t) (gzFile, const char *, ...);

extern gzFile pStdout; /*  = (gzFile) stdout; */
extern zz_printf_t pPrintf;

#else

#define pPrintf fprintf
#define pStdout stdout

#endif

extern int lineMax;
extern char zz_bs[1024];
extern char zz_ky[1024];
extern char zz_nf[1024];
extern char *BigBuffer;
extern char const *BigLine[RES_RED + 16];

extern int dat_read(char const *fname,
		    int st,
		    int to,
		    int ls);
extern int dat_out(int level);
extern void html_error(enum html_error_t errorcode);

#endif /* READ_H__ */
