/* file: index.c
   インデクス吐き出し処理 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "read.h"
#include "read2ch.h"
#include "r2chhtml.h"

/****************************************************************/
/*	スレダイジェスト(index2.html)を出力してみる。		*/
/*	path_depth == 1 のはず					*/
/****************************************************************/
void dat_out_index(void)
{
	char *dat_name_4digest[N_INDEX_DIGESTS];

	int i;
	int max = 0;

	pPrintf(pStdout,
		R2CH_HTML_INDEX_HEADER("%s", "%s"),
		"プログラム技術＠2ch掲示板",
		"プログラム技術＠2ch掲示板");

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
				R2CH_HTML_INDEX_LABEL("%.64s/%.*s",
						      "%d",
						      "#%d",
						      "%.*s"),
				zz_bs,
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
				R2CH_HTML_INDEX_ANCHOR("%.64s/%.*s",
						       "%d",
						       "%.*s"),
				zz_bs,
				datn, p,
				1 + i,
				subjn, subj);
		}
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_AD("%.64s/"),
		zz_bs);

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
		char fname[1024];

		/* ファイルを読み込んでくる */
		sprintf(fname,
			"../%.64s/dat/%.64s.dat",
			zz_bs,
			dat_name_4digest[i]);
		dat_read(fname, 0, 0, 10);

		/* せっかくだから表示してあげる */
		pPrintf(pStdout,
			"<HR><P><A name=\"#%d\">%s\n",
			1 + i, fname);
		dat_out();
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
