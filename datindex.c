/*  file: datindex.c
 *
 *  インデクス運用
 *
 *  $Id: datindex.c,v 1.9 2001/09/04 07:26:48 2ch Exp $ */

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "datindex.h"
#include "read.h"

/* /board/dat/idx/XXXXXXXXXX.idx */
#define DATINDEX_PATH "dat/idx"

/* version が 0 のときの、作り直し時間(sec) */
#define DATINDEX_EXPIRATION 60

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

	/* hhmm[ss]を得る */
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
 *	指定された行(ただしchunk以内)の
 *	行インデクスをつくりあげる
 *	こいつはshared_idxをいじらない
 *	進んだオフセットを返す
 ****************************************************************/
static int buildup_line(struct DATINDEX_LINE *line,
			int line_n,
			char const *dat,
			int datlen,
			time_t dat_lastmod,
			int *valid_lines,
			time_t *newest_lastmod)
{
	int ofs;
	int linenum;

	/* まずは epoch から始めよ */
	*newest_lastmod = 0;

	for (ofs = 0, linenum = 0;
	     ofs < datlen && linenum < line_n;
	     line++, linenum++) {
		int i, n;
		int orig_ofs;
		struct tm datetime;

		/* わかんないフラグが多すぎてごめんね */
		int aborted = 0;	/* あぼーん? */
		int completed = 0;	/* パースが通ったらセットすべし */
		time_t lastmod = dat_lastmod;

		orig_ofs = ofs;

		/* name<>mailto<>date id<>text<>[subject]\n */
		/* name, mailtoをスキップする */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->name.p = &dat[ofs];
			else if (i == 1)
				line->mailto.p = &dat[ofs];

			while (!(dat[ofs] == '<' && dat[ofs + 1] == '>')) {
				if (dat[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&dat[ofs], "<>\n");
			}

			if (i == 0)
				line->name.len = &dat[ofs] - line->name.p;
			else if (i == 1)
				line->mailto.len = &dat[ofs] - line->mailto.p;

			ofs += 2;
		}

		/* 日付を解析する
		   日付文字がまったく得られなかったら、
		   あぼーんと見なす */
		n = get_date(&datetime, &dat[ofs]);
		if (n == 0) {
			/* あぼーんと見なす
			   このまま行の処理を打ち切ってもいいのだが
			   スレタイトルを取得したい場合があるので、
			   その場合はフラグを立てて続行(鬱 */
			aborted = 1;
		} else {
			/* 取得した時刻を L-M としてセットしておく */
			lastmod = mktime(&datetime);
		}
		/* dateの残り、本文をスキップする */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->date.p = &dat[ofs];
			else if (i == 1)
				line->text.p = &dat[ofs];

			while (!(dat[ofs] == '<' && dat[ofs + 1] == '>')) {
				if (dat[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&dat[ofs], "<>\n");
			}

			if (i == 0)
				line->date.len = &dat[ofs] - line->mailto.p;
			else if (i == 1)
				line->text.len = &dat[ofs] - line->date.p;

			ofs += 2;
		}
		/* ここまで来れたらあぼーんではないと見なす
		   (除 aborted がセットされてる場合 )*/
		if (!aborted) {
			completed = 1;
		}
		/* スレタイトルがあるかもしれないが、読み飛ばす */

	can_not_parse:
		/* XXX ファイル終端のケツを破る可能性あり
		   修正しる!! */
		ofs += 1 + strcspn(&dat[ofs], "\n");

		if (completed) {
			line->lastmod = lastmod;
		} else {
			/* 行の解析が完了していない場合 */

			static struct DATINDEX_LINE ab =
			{
				{"あぼーん", 8},
				{"あぼーん", 8},
				{"あぼーん", 8},
				{"あぼーん", 8},
				0, 0
			};

			*line = ab;
		}

		line->len = ofs - orig_ofs;	/* \n までの長さ */

		/* あぼーん行であるときは、
		   lastmod が最新時刻のままであるはず */
		if (*newest_lastmod < lastmod)
			*newest_lastmod = lastmod;
	}

	*valid_lines = linenum;
	return ofs;
}
/****************************************************************
 *	用意された領域に
 *	新たにインデクスをつくりあげる
 *	private_datにファイルが読まれているのが前提
 *	こいつは署名を行わない
 *	idx に NULL を与えることで、
 *	行インデクスのみ更新できる。
 *	処理した最終行数を返す
 ****************************************************************/
static int buildup_index(DATINDEX_OBJ *dat,
			 int linenum,	/* 0 origin */
			 DATINDEX volatile *idx,
			 long ofs,
			 long datlen)
{
	int chunk;
	char const *const p = dat->private_dat;
	int n_line_processed;

	/* .dat の mtime は記録しておく */
	if (idx)
		idx->lastmod = dat->dat_stat.st_mtime;

	chunk = linenum / DATINDEX_CHUNK_SIZE;
	linenum %= DATINDEX_CHUNK_SIZE;

	for (;
	     (ofs < datlen
	      && (DATINDEX_CHUNK_SIZE * chunk + linenum
		  < DATINDEX_MAX_ARTICLES));
	     linenum += n_line_processed,
		     (linenum >= DATINDEX_CHUNK_SIZE
		      ? chunk++, linenum = 0
		      : 0)) {
		struct DATINDEX_LINE *line;
		int i, n;

		time_t chunk_lastmod;

		n = DATINDEX_CHUNK_SIZE * chunk + linenum;
		line = &dat->line[n];

		/* 行インデクスを生成させる */
		ofs += buildup_line(line,
				    DATINDEX_CHUNK_SIZE - linenum,
				    &p[ofs],
				    datlen - ofs,
				    dat->dat_stat.st_mtime,
				    &n_line_processed,
				    &chunk_lastmod);
		if (!idx)
			continue;

		/* 以降、idxの更新に入る */

		if (linenum == 0
		    && n_line_processed == DATINDEX_CHUNK_SIZE) {
			/* chunkを完全に捉えきっていないと
			   インデクスは更新しない */
			idx->idx[chunk].lastmod = chunk_lastmod;
			idx->idx[chunk].nextofs = ofs;

			/* 有効な行に対してフラグ立ててく */
			for (i = 0; i < n_line_processed; i++) {
				if (line[i].lastmod)
					idx->idx[chunk].valid_bitmap |= 1 << i;
			}
		}

		/* サブジェクトが採れていそうだったら、採る */
		if (n == 0
		    && line->len
		    && memcmp(&line->text.p[line->text.len],
			      "<>",
			      2) == 0)
			idx->title_ofs = (line->text.p
					  - line->name.p
					  + line->text.len + 2);
	}

	/* linenum が CHUNK_SIZE を超えてる場合があるが、
	   下記の式は適切に働く。 */
	linenum = DATINDEX_CHUNK_SIZE * chunk + linenum;
	
	if (idx)
		idx->linenum = linenum;

	return linenum;
}
/****************************************************************
 *	ヒープに領域を確保し、インデクスを汲み上げる
 *	こいつも署名を行わない
 *	こいつは作成したINDEXを返すことに注意
 ****************************************************************/
static DATINDEX *create_local_index(DATINDEX_OBJ *dat)
{
	DATINDEX *idx = calloc(1, sizeof(DATINDEX));
	if (!idx)
		return 0;
	dat->shared_idx = idx;
	if (buildup_index(dat,
			  0,	/* 開始行 */
			  idx,
			  0,	/* ファイルオフセット */
			  dat->dat_stat.st_size)) {
		dat->linenum = idx->linenum;
		return idx;
	} else
		return NULL;
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

	/* まず、ローカルにインデクスを作ってみる
	   dat中各フィールドは、ローカル内容で
	   満たされるので、このまま帰っても氏なない。 */
	tidx = create_local_index(dat);
	if (!tidx) {
		/* .datファイルが不正だったとみなす */
		return 0;
	}

	fd = open(fn, O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0) {
		/* たとえば、idxディレクトリが
		   存在しなかった場合でも、open(2)は
		   失敗する。インデクスはローカルに作成される */
		return 0;
	}

	/* ファイル領域を作り出す
	   作成してもらった private idx を書き出す*/
	if (write(fd, tidx, sizeof(DATINDEX)) < 0) {
		/* XXX 本来ならば unlink した方がいい */
		return 0;
	}
	/* 書きだしたものを、mapする */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		return 0;
	}

	/* 最後に、署名などを書き入れる */
	dat->shared_idx->signature = DATINDEX_VERSION;
	dat->shared_idx->version   = DATINDEX_VERSION;

	return 1;
}
/****************************************************************
 *	本体.datがメモリに
 *	固定できるかどうか挑戦してみる
 *	いまはBigBufferのことは考慮してない
 *	.datを開くモジュールは、これだけである。
 ****************************************************************/
static int open_dat(DATINDEX_OBJ *dat,
		    char const *bs, long ky)
{
	int fd;
	char fn[64];

	/* 行インデクスを、空の状態で作成 */
	dat->line = calloc(sizeof(dat->line[0]),
			   DATINDEX_MAX_ARTICLES);
	if (!dat->line)
		return 0;

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

	/* 成功したということにしておこう
	   もちろん fd の後始末はサボる XXX */

	return 1;
}
/****************************************************************
 *	インデクスファイルをメモリに固定する
 *	固定できないときは、エラーを返す
 ****************************************************************/
int datindex_open(DATINDEX_OBJ *dat,
		  char const *bs, long ky)
{
	int fd;
	int ver;
	int local_ofs, local_st, current_n, old_n;
	int i;

	char fn[64];

	/* まずは本体.datがメモリに
	   固定できるかどうか挑戦してみる
	   いまはBigBufferのことは考慮してない */
	if (!open_dat(dat, bs, ky))
		return 0;

	/* idx のファイルハンドルをオープン */
	sprintf(fn, "../%.64s/" DATINDEX_PATH "/%ld.idx", bs, ky);
	fd = open(fn, O_RDWR);
	if (fd < 0) {
		/* インデクスを作成
		   (もしかするとローカルに
		   作られるかもしれない) */
		return create_index(dat, fn);
	}

	/* まずは map を譲り受ける */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		/* 自力でインデクス作れ */
		return !!create_local_index(dat);
	}

	/* インデクスのバージョンを検査 */
	ver = dat->shared_idx->version;
	if (ver == 0) {
		/* 思いっきり時間が経ってないかい? */
		struct stat idx_stat;
		if (fstat(fd, &idx_stat) >= 0
		    && time(NULL) >= idx_stat.st_mtime + DATINDEX_EXPIRATION
		    && unlink(fn) >= 0) {
			/* あまり時間が経ちすぎなヤツは、
			   造り損ねと考え、
			   もいちど作成を試みる */
			return create_index(dat, fn);
		}

		/* 自力でインデクス作れや */
		return !!create_local_index(dat);
	} else if (ver > DATINDEX_VERSION) {
		/* 未知のバージョンなので泣き寝入り */
		return !!create_local_index(dat);
	} else if (ver < DATINDEX_VERSION
		   || dat->shared_idx->signature != DATINDEX_VERSION) {
		/* 潰して作り直しや */
		if (unlink(fn) >= 0)
			return create_index(dat, fn);
	}

	/* すべてが揃った
	   ここからがホントの闘い
	   まずは、インデクスに足りない
	   行インデクス(ローカルだ)つくっとく */

	/* すでに用意されているインデクスを計上 */
	local_ofs = 0;
	for (i = 0; 
	     i < DATINDEX_IDX_SIZE
		     && dat->shared_idx->idx[i].nextofs != 0;
	     i++)
		local_ofs = dat->shared_idx->idx[i].nextofs;

	local_st = DATINDEX_CHUNK_SIZE * i;

	/* 次に、ローカル行インデクス(不足分)を構築 */
	dat->linenum = buildup_index(dat,
				     local_st,
				     NULL,
				     local_ofs,
				     dat->dat_stat.st_size);
	/* 更新行がなかった場合は何もすることがない、はず */
	if (!dat->linenum)
		return 1;

	/* indexを更新すべきかどうかの判断 */
	current_n = dat->shared_idx->linenum;
	if (current_n >= dat->linenum)
		return 1;

	/* 進んだ分を申請 */
	old_n = DATINDEX_CMPXCHG(dat->shared_idx->linenum,
				 current_n,
				 dat->linenum);

	/* 申請が通らなかったら、あきらめて帰れ */
	if (old_n == dat->linenum)
		return 1;

	/* chunk boundaryをまたいでいなかったら、何もする必要ナシ */
	if (old_n / DATINDEX_CHUNK_SIZE
	    == dat->linenum / DATINDEX_CHUNK_SIZE)
		return 1;

	/* 申請が通って、かつ、chunk boundaryを
	   またいだら、chunk indexを更新する義務を負う */
	for (i = old_n / DATINDEX_CHUNK_SIZE;
	     i < dat->linenum / DATINDEX_CHUNK_SIZE;
	     i++) {
		int j;
		dat->shared_idx->idx[i].lastmod = 0;
		for (j = 0; j < DATINDEX_CHUNK_SIZE; j++)
			if (dat->line[DATINDEX_CHUNK_SIZE * i + j].lastmod)
				dat->shared_idx->idx[i].valid_bitmap |= 1 << j;
	}

	return 1;
}

/****************************************************************
 *	lastmodを拾い上げる
 *	first は、!is_nofirst() であることに注意
 *	与えられるパラメータは、1 originを想定。
 ****************************************************************/
time_t datindex_lastmod(DATINDEX_OBJ const *dat,
			int first,	/* 1番目を含める */
			int st,
			int to)
{
	int i;
	int st_chunk = (st - 1) / DATINDEX_CHUNK_SIZE;
	time_t lastmod = 0;

	for (i = (first ? 0 : st_chunk);
	     i <= (to - 1) / DATINDEX_CHUNK_SIZE;
	     (i == 0 && st_chunk > 0
	      ? i = st_chunk
	      : i++)) {
		if (dat->shared_idx->idx[i].nextofs == 0) {
			if (lastmod < dat->dat_stat.st_mtime)
				lastmod = dat->dat_stat.st_mtime;
		} else if (lastmod < dat->shared_idx->idx[i].lastmod)
			lastmod = dat->shared_idx->idx[i].lastmod;
	}
	return lastmod;
}
/*EOF*/
