/* file: read.h */

#ifndef READ_H__
#define READ_H__

#include "read2ch.h"

#ifdef ZLIB

#ifndef GZIP
# define GZIP			/* gzip由来のコードも使用するので */
#endif
#include <zlib.h>

typedef int (*zz_printf_t) (gzFile, const char *, ...);

extern gzFile pStdout; /*  = (gzFile) stdout; */
extern zz_printf_t pPrintf;
#endif

extern int lineMax;
extern char zz_bs[1024];
extern char zz_nf[1024];
extern char *BigBuffer;
extern char const *BigLine[RES_RED + 16];

extern int dat_read(char const *fname,
		    int st,
		    int to,
		    int ls);
extern int dat_out(void);

#endif /* READ_H__ */
