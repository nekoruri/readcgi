/*  file: datindex.c
 *
 *  インデクス運用
 *
 *  $Id: datindex.c,v 1.3 2001/09/01 05:36:48 2ch Exp $ */

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "datindex.h"
#include "read.h"

/****************************************************************
 *	日付を解析する
 *	解析に成功したら、スキャンした文字数を返す
 *	失敗したら 0 を返す
 *	…なぜか秒の取得にも対応しているぞ(ｗ
 ****************************************************************/
static int get_date(struct tm *datetime,
		    char const *p)
{
	char buf[24];
	char const *dp;
	int n, ofs, yy;

	memset(datetime, 0, sizeof(struct tm));

	ofs = strcspn(p, "<>\n0123456789");
	if (!isdigit(p[ofs]))
	return 0;

	/* yymmddを得る */
	n = strspn(&p[ofs], "/0123456789");
	if (n >= 20)
		return 0;
	memcpy(buf, &p[ofs], n);
	buf[n] = 0;
	ofs += n;
	if (!(dp = strtok(buf, "/")))
		return 0;
	yy = atoi(dp);
	if (yy >= 1900)
		yy -= 1900;
	else if (yy < 70)
		yy += 100;
	datetime->tm_year = yy;
	if (!(dp = strtok(NULL, "/")))
		return 0;
	datetime->tm_mon = atoi(dp) - 1;
	if (!(dp = strtok(NULL, "/")))
		return 0;
	datetime->tm_mday = atoi(dp);

	/* hhmmssを得る */
	ofs += strcspn(&p[ofs], "<>\n0123456789");
	if (!isdigit(p[ofs]))
		return 0;
	n = strspn(&p[ofs], ":0123456789");
	if (n >= 20)
		return 0;
	memcpy(buf, &p[ofs], n);
	buf[n] = 0;
	ofs += n;
	if (!(dp = strtok(buf, ":")))
		return 0;
	datetime->tm_hour = atoi(dp);
	if (!(dp = strtok(NULL, ":")))
		return 0;
	datetime->tm_min = atoi(dp);
	if ((dp = strtok(NULL, ":")) != NULL)
		datetime->tm_sec = atoi(dp);

	return ofs;
}
/****************************************************************
 *	用意された領域に
 *	新たにインデクスをつくりあげる
 *	private_datにファイルが読まれているのが前提
 *	こいつは署名を行わない
 ****************************************************************/
static int buildup_index(DATINDEX_OBJ *dat,
		       DATINDEX *idx)
{
	long ofs;
	int chunk, linenum;
	long datlen = dat->dat_stat.st_size;
	char const *const p = dat->private_dat;

	for (ofs = 0, chunk = 0, linenum = 0;
	     (ofs < datlen
	      && (DATINDEX_CHUNK_SIZE * chunk + linenum
		  < DATINDEX_MAX_ARTICLES));
	     linenum++) {
		int i, n;
		long nt;
		struct tm datetime;

		if (linenum >= DATINDEX_CHUNK_SIZE) {
			/* 繰り上がり */
			chunk++;
			linenum = 0;
			idx->idx[chunk].ofs = ofs;
			idx->idx[chunk].lastmod = 0; /* epoch(藁 */
		}

		/* name<>mailto<>date id<>text<>[subject]\n */
		/* name, mailtoをスキップする */
		for (i = 0; i < 2; i++) {
			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}
			ofs += 2;
		}

		/* 日付を解析する
		   日付文字がまったく得られなかったら、
		   あぼーんと見なす */
		n = get_date(&datetime, &p[ofs]);
		if (n == 0) {
			/* あぼーんと見なす */
			if (idx->idx[chunk].lastmod < dat->dat_stat.st_mtime)
				idx->idx[chunk].lastmod = dat->dat_stat.st_mtime;
			/* このまま行の処理を打ち切ってもいいのだが
			   スレタイトルを取得したい場合があるので、
			   放っておく */
		} else {
			/* 取得した時刻を L-M としてセットしておく */
			nt = mktime(&datetime);
			if (idx->idx[chunk].lastmod < nt)
				idx->idx[chunk].lastmod = nt;
		}
		/* dateの残り、本文をスキップする */
		for (i = 0; i < 2; i++) {
			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}
			ofs += 2;
		}
		/* ここまで来れたらあぼーんではないと見なす */
		n = DATINDEX_CHUNK_SIZE * chunk + linenum;
		idx->valid_bitmap[n >> 5] |= 1 << (n & 31);
		/* スレタイトル取得を試みる */
		if (chunk == 0 && linenum == 0) {
			idx->title_ofs = ofs;
		}
		ofs += 1 + strcspn(&p[ofs], "\n");
	can_not_parse:
		;
	}

	return 1;
}
/****************************************************************
 *	ヒープに領域を確保し、インデクスを汲み上げる
 *	こいつも署名を行わない
 ****************************************************************/
static int create_local_index(DATINDEX_OBJ *dat)
{
	DATINDEX *idx = calloc(1, sizeof(DATINDEX));
	if (!idx)
		return 0;
	/* private idxとshared idxは、
	   同じものが見えるようにしておく */
	dat->private_idx = idx;
	dat->shared_idx = idx;
	return buildup_index(dat, idx);
}
/****************************************************************
 *	インデクスファイルを作成してみる
 *	作成できないときはローカルに作る
 ****************************************************************/
static int create_index(DATINDEX_OBJ *dat,
			char const *fn)
{
	int fd;
	DATINDEX *tidx;

	/* まず、ローカルにインデクスを作ってみる */
	if (!create_local_index(dat)) {
		/* .datファイルが不正だったとみなす */
		return 0;
	}

	fd = open(fn, O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0) {
		return 0;
	}

	/* ファイル領域を作り出す */
	if (write(fd, dat->private_idx, sizeof(DATINDEX)) < 0) {
		/* XXX 本来ならば unlink した方がいい */
		return 0;
	}
	tidx = mmap(NULL,
		    sizeof(DATINDEX),
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (tidx == MAP_FAILED) {
		return 0;
	}
	dat->shared_idx = tidx;

	/* 最後に、署名などを書き入れる */
	dat->shared_idx->signature = DATINDEX_VERSION;
	dat->shared_idx->version   = DATINDEX_VERSION;

	return 1;
}
/****************************************************************
 *	インデクスファイルをメモリに固定する
 *	固定できないときは、エラーを返す
 ****************************************************************/
int read_index(DATINDEX_OBJ *dat,
	       char const *bs, long ky)
{
	char fn[64];
	int fd;
	int ver;

	/* まずは本体.datがメモリに
	   固定できるかどうか挑戦してみる
	   いまはBigBufferのことは考慮してない */
	sprintf(fn, "../%.64s/dat/%ld.dat", bs, ky);
	fd = open(fn, O_RDONLY);
	if (fd < 0)
		return 0;
	if (fstat(fd, &dat->dat_stat) < 0)
		return 0;
	dat->private_dat = mmap(NULL,
				dat->dat_stat.st_size + 1,
				PROT_READ,
				MAP_PRIVATE,
				fd,
				0);
	if (dat->private_dat == MAP_FAILED)
		return 0;

	/* もちろんfdの後始末はサボる XXX */

	/* idxのファイルハンドルをオープン */
	sprintf(fn, "../%.64s/dat/%ld.idx", bs, ky);
	fd = open(fn, O_RDWR);
	if (fd < 0) {
		/* インデクスを作成 */
		return create_index(dat, fn);
	}

	/* まずは private map を譲り受ける */
	dat->private_idx = mmap(NULL,
				sizeof(DATINDEX),
				PROT_READ,
				MAP_PRIVATE,
				fd,
				0);
	if (dat->private_idx == MAP_FAILED) {
		/* 自力でインデクス作れ */
		return create_local_index(dat);
	}

	/* インデクスのバージョンを検査 */
	ver = dat->private_idx->version;
	if (ver == 0) {
		/* 思いっきり時間が経ってないかい? */

		/* 自力でインデクス作れや */
		return create_local_index(dat);
	} else if (ver > DATINDEX_VERSION) {
		/* 未知のバージョンなので泣き寝入り */
		return create_local_index(dat);
	} else if (ver < DATINDEX_VERSION
		   || dat->private_idx->signature != DATINDEX_VERSION) {
		/* 潰して作り直しや */

	}

	/* これでやっと、shared mapが与えられる */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		/* 自力でインデクス建てろ */
		return create_local_index(dat);
	}
	return 1;
}

/*EOF*/
