CC            = gcc
DEFS          = -DHAVE_READ2CH_H
# DEFS        = -DCUTRESLINK -DRELOADLINK -DLASTMOD -DNEWBA -DGSTR2 \
# 		-DTYPE_TERI -DCOOKIE -DPREVENTRELOAD
CFLAGS        = -g -O2 -Wall
LIBS          = -lz
#SOURCES       = read.c util_date.c
OBJS          = read.o util_date.o
INCLUDES      = read2ch.h read2ch.h r2chhtml.h

.SUFFIXES: .c .o .cgi

# phony targets
.PHONY:all clean test dat strip tags

# targets
read.cgi: $(OBJS)

all: read.cgi TAGS

#read.cgi: $(SOURCES) $(INCLUDES)
#	$(CC) $(LIBS) $(CFLAGS) $(DEFS) -o $@ $(SOURCES)

clean:
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

strip:
	$(MAKE) 'CFLAGS=-O2 -Wall' read.cgi
	strip read.cgi

tags: TAGS

TAGS: *.c *.h
	etags $^ $>

# implicit rules
.o.cgi:
	$(CC) $^ $> $(LIBS) -o $@

.c.o:
	$(CC) -c $< $(DEFS) $(CFLAGS) -o $@

# dependencies
read.o: read.c read2ch.h r2chhtml.h util_date.h
util_date.o: util_date.c util_date.h
