/* file: digest.c
   板一覧ダイジェスト吐き出し処理 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "digest.h"
#include "read.h"
#include "read2ch.h"
#include "r2chhtml.h"

/****************************************************************/
/*	板ダイジェスト(index2.html)を出力してみる。		*/
/*	path_depth == 1 のはず					*/
/****************************************************************/
void dat_out_index(void)
{
	char *dat_name_4digest[N_INDEX_DIGESTS];
	char fname[1024];
	char *html;
	int title_s = 0, title_e = 0;

	int i;
	int fd;
	int max = 0;

	/* XXX むりやり板タイトルを取得してみる
	   暴力的な方法だなあ */
	sprintf(fname, "../%.64s/index2.html", zz_bs);
	fd = open(fname, O_RDONLY);
	if (fd < 0)
		html_error(ERROR_NOT_FOUND);
	html = mmap(NULL, 65536, PROT_READ, MAP_SHARED, fd, 0);
	if (html == MAP_FAILED)
		html_error(ERROR_NO_MEMORY);
	/* <TITLE> (7 chars) を探す
	   strstr() を使いたいところだが
	   番人がいないので、手で探す */
	i = 0;
	while (i < 8192 - 7)
		switch (html[i + 6]) {
		default:	i += 7; continue;
		case '<':	i += 6; continue;
		case 'I':
		case 'i':	i += 4; continue;
		case 'T':
		case 't':	i += 3; continue;
		case 'L':
		case 'l':	i += 2; continue;
		case 'E':
		case 'e':	i += 1; continue;
		case '>':
			if (memcmp(&html[i], "<TITLE>", 7) != 0
			    && memcmp(&html[i], "<title>", 7) != 0) {
				/* ちょっと手抜き */
				i++;
				continue;
			}
			/* found! */	
			i += 7;
			title_s = i;
			/* こんどは </TITLE> (8 chars) を探す */
			while (i < 8192 - 8)
				switch (html[i + 7]) {
				default:	i += 8; continue;
				case '<':	i += 7; continue;
				case '/':	i += 6; continue;
				case 'I':
				case 'i':	i += 4; continue;
				case 'T':
				case 't':	i += 3; continue;
				case 'L':
				case 'l':	i += 2; continue;
				case 'E':
				case 'e':	i += 1; continue;
				case '>':
					if (memcmp(&html[i],
						   "</TITLE>",
						   8) != 0
					    && memcmp(&html[i],
						      "</title>",
						      8) != 0) {
						/* ちょっと手抜き */
						i++;
						continue;
					}
					/* found! */
					title_e = i;
					goto found;
				}

			/* i は進んでいるはず */
			continue;
		}

 found:
	if (title_s > 0 && title_e - title_s > 0)
		pPrintf(pStdout,
			R2CH_HTML_INDEX_HEADER("%.*s", "%.*s"),
			title_e - title_s, &html[title_s],
			title_e - title_s, &html[title_s]);
	else
		pPrintf(pStdout,
			R2CH_HTML_INDEX_HEADER("%s", "%s,%d,%d,%d"),
			"ななし板",
			"ななし板", title_s, title_e, i);

	/* fd, html, 資源の放棄は行ってない…鬱だ */

	/* スレ一覧の出力
	   この時点ではまだメモリ上に subject.txt が存在。*/
	for (i = 0; i < N_INDEX_THREADS && i < lineMax; i++) {
		char const *p = BigLine[i];
		char const *subj;
		int datn, subjn;
		datn = strspn(p, "0123456789");
		if (datn == 0)
			continue;
		if (memcmp(&p[datn], ".dat<>", 6) != 0)
			continue;
		subj = p + datn + 6;
		subjn = strcspn(subj, "\r\n");
		if (subjn == 0)
			continue;
		if (i < N_INDEX_DIGESTS) {
			char *q;
			pPrintf(pStdout,
				R2CH_HTML_INDEX_LABEL("%.*s",
						      "%d",
						      "#%d",
						      "%.*s"),
				datn, p,
				1 + i,
				1 + i,
				subjn, subj);
			q = malloc(datn + 1);
			memcpy(q, p, datn);
			q[datn] = 0;
			max = i;
			dat_name_4digest[max++] = q;
		} else {
			pPrintf(pStdout,
				R2CH_HTML_INDEX_ANCHOR("%.*s",
						       "%d",
						       "%.*s"),
				datn, p,
				1 + i,
				subjn, subj);
		}
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_AD);

	/* かなーり気休め */
#ifndef USE_MMAP
	if (BigBuffer)
		free(BigBuffer);
#endif
	BigBuffer = NULL;
	/* これ以降、グローバル変数が
	   信用おけなくなる可能性あり
	   なんだかひっくり返して書き直したい
	   …鬱だ氏のう */

	/* スレダイジェストの出力 */
	zz_nf[0] = 0;
	for (i = 0; i < N_INDEX_DIGESTS && i < max; i++) {
		/* これをセットしておかないと
		   全般的に調子悪い */
		strcpy(zz_ky, dat_name_4digest[i]);

		/* ファイルを読み込んでくる */
		sprintf(fname,
			"../%.64s/dat/%.64s.dat",
			zz_bs,
			dat_name_4digest[i]);
		dat_read(fname, 0, 0, 10);

		/* せっかくだから表示してあげる */
		pPrintf(pStdout,
			R2CH_HTML_DIGEST_HEADER_1("%d"),
			1 + i);
		dat_out(1);
#ifndef USE_MMAP
		if (BigBuffer)
			free(BigBuffer);
#endif
		BigBuffer = NULL;
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_FOOTER);
}
/****************************************************************/
/*	スレ一覧(subback.html相当)を出力してみる。		*/
/*	path_depth == 2 のはず					*/
/****************************************************************/
void dat_out_subback(void)
{
	int i;

	pPrintf(pStdout, R2CH_HTML_SUBBACK_HEADER);

	/* 行を読み込んで解析していく
	   BigLine[]の情報をとりあえず信用しておくことにする */
	for (i = 0; i < lineMax; i++) {
		char const *p = BigLine[i];
		char const *subj;
		int datn, subjn;
		datn = strspn(p, "0123456789");
		if (datn == 0)
			continue;
		if (memcmp(&p[datn], ".dat<>", 6) != 0)
			continue;
		subj = p + datn + 6;
		subjn = strcspn(subj, "\r\n");
		if (subjn == 0)
			continue;
		pPrintf(pStdout,
			R2CH_HTML_SUBBACK_ITEM("%.*s", "%d", "%.*s"),
			datn, p,
			1 + i,
			subjn, subj);
	}

	pPrintf(pStdout, R2CH_HTML_SUBBACK_FOOTER);
}

/*EOF*/
