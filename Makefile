zlibdir       = zlib

CC            = gcc
DEFS          = -DHAVE_READ2CH_H
# DEFS        = -DCUTRESLINK -DRELOADLINK -DLASTMOD -DNEWBA -DGSTR2 \
# 		-DTYPE_TERI -DCOOKIE -DPREVENTRELOAD
CFLAGS        = -I$(zlibdir) -g -O2 -Wall -funsigned-char
LIBS          = 
OBJS          = datindex.o digest.o read.o util_date.o $(zlibdir)/libz.a
SRCS          = datindex.c digest.c read.c util_date.c \
		r2chhtml.h read2ch.h datindex.h digest.h read.h util_date.h \
		Makefile

.SUFFIXES: .c .o .cgi

# phony targets
.PHONY:all clean test dat strip tags dist

# targets
read.cgi: $(OBJS)

all: read.cgi TAGS

clean:
	cd $(zlibdir); make distclean
	rm -f read.cgi *.o

# バイナリのあるファイルの下にtech/dat/998845501.datを置いてから実行
test:
	( export HTTP_COOKIE='NAME=abcde%21;MAIL=hoge@huga.net' \
	export QUERY_STRING='bbs=tech&key=998845501'; \
	export HTTP_USER_AGENT='console'; \
	cd tech; \
	../read.cgi )

dat:
	( mkdir tech; mkdir tech/dat; \
	wget -O tech/dat/998845501.dat \
		http://piza2.2ch.net/tech/dat/998845501.dat )

strip: read.cgi
	strip -v $^ $>

dist: read.tgz

read.tgz: $(SRCS) $(zlibdir)/gzio.c
	tar cf - $(SRCS) $(zlibdir)/*.[ch] $(zlibdir)/configure \
	$(zlibdir)/Makefile.in \
	| gzip -9 > read.tgz

tags: TAGS

TAGS: *.c *.h
	etags $^ $>

$(zlibdir)/libz.a: $(zlibdir)/gzio.c
	( cd $(zlibdir) && ./configure --libdir=. --includedir=. \
		&& make libz.a)

# implicit rules
.o.cgi:
	$(CC) $^ $> $(LIBS) -o $@

.c.o:
	$(CC) -c $< $(DEFS) $(CFLAGS) -o $@

# dependencies
datindex.o: datindex.c datindex.h read.h read2ch.h
digest.o: digest.c digest.h read.h read2ch.h r2chhtml.h
read.o: read.c read2ch.h datindex.h digest.h read.h r2chhtml.h \
 util_date.h
util_date.o: util_date.c util_date.h
