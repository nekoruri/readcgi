.SUFFIXES: .c .cgi

CC            = gcc
DEFS          = -DCUTRESLINK -DLASTMOD -DNEWBA -DGSTR2 -DTYPE_TERI \
		-DCOOKIE -DPREVENTRELOAD
# DEFS        = -DCUTRESLINK -DRELOADLINK -DLASTMOD -DNEWBA -DGSTR2 \
#		-DTYPE_TERI -DCOOKIE -DPREVENTRELOAD
DEFS_ZLIB     = -DZLIB -lz
DEFS_GZIP     = -DGZIP
CFLAGS        = -g -Wall
SOURCES       = read.c util_date.c
INCLUDES      = read2ch.h r2chhtml.h

# all: read-zlib.cgi read-gzip.cgi
all: read.cgi

read.cgi: $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(DEFS) $(DEFS_GZIP) -o $@ $(SOURCES)
read-gzip.cgi: $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(DEFS) $(DEFS_GZIP) -o $@ $(SOURCES)
read-zlib.cgi: $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(DEFS) $(DEFS_ZLIB) -o $@ $(SOURCES)
clean:
	rm -f read-zlib.cgi read-gzip.cgi *.o

# バイナリのあるファイルの下にtech/dat/998845501.datを置いてから実行
test:
	( export HTTP_COOKIE='NAME=abcde%21;MAIL=hoge@huga.net' \
	export QUERY_STRING='bbs=tech&key=998845501'; \
	cd tech; \
	../read-zlib.cgi )

dat:
	( mkdir tech; mkdir tech/dat; \
	wget -O tech/dat/998845501.dat \
		http://piza2.2ch.net/tech/dat/998845501.dat )

#%.cgi: %.c
#	$(CC) $(CFLAGS) $< -o $@

.PHONY:all clean test dat

