.SUFFIXES: .c .cgi

CC            = gcc
DEFS          = -DHAVE_READ2CH_H
# DEFS        = -DCUTRESLINK -DRELOADLINK -DLASTMOD -DNEWBA -DGSTR2 \
# 		-DTYPE_TERI -DCOOKIE -DPREVENTRELOAD
CFLAGS        = -g -Wall
LIBS          = -lz
SOURCES       = read.c util_date.c
INCLUDES      = read2ch.h read2ch.h r2chhtml.h


all: read.cgi

read.cgi: $(SOURCES) $(INCLUDES)
	$(CC) $(LIBS) $(CFLAGS) $(DEFS) -o $@ $(SOURCES)
clean:
	rm -f read.cgi *.o

# バイナリのあるファイルの下にtech/dat/998845501.datを置いてから実行
test:
	( export HTTP_COOKIE='NAME=abcde%21;MAIL=hoge@huga.net' \
	export QUERY_STRING='bbs=tech&key=998845501'; \
	cd tech; \
	../read.cgi )

dat:
	( mkdir tech; mkdir tech/dat; \
	wget -O tech/dat/998845501.dat \
		http://piza2.2ch.net/tech/dat/998845501.dat )

#%.cgi: %.c
#	$(CC) $(CFLAGS) $< -o $@

.PHONY:all clean test dat

