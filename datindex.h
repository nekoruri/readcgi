/*  file: datindex.h
 *
 *  世界共通インデクスの定義
 *
 *  $Id: datindex.h,v 1.6 2001/09/03 14:53:57 2ch Exp $ */

#ifndef DATINDEX_H__
#define DATINDEX_H__

#include <time.h>
#include <sys/stat.h>

#define DATINDEX_VERSION 1

/*
 *  効率を考えて、インデクスはなるべく4096バイト(page size)以内に
 *  収まるようにする。逆に言うと、4096バイトより小さくしても、
 *  メモリ効率・ディスク容量の点でメリットはない。
 *  (Linux ext2は、fragmentを作らないはずなので、
 *   minimum allocation unitは4096バイトのハズ)
 *
 *  【運用の最低約束】
 *
 *    * version が0のインデクスに遭遇、
 *	あるいはインデクスのサイズが0だったら、
 *	基本的に何もせず、インデクスに頼らず自力で処理する。
 *	ただし、versionが0のまま一定時間経過したものは、
 *	廃棄-再作成を行うべき。(作成途中で氏んだと考えれ)
 *    * version が知ってるものより大きいときは、
 *	何もせず、自力で処理する。
 *    * version が知ってるものより小さい場合は、
 *	廃棄して新規作成。
 *
 *  【新規作成のやり方】
 *    * creat(2)でつくれ。
 *	creat()に勝ったプロセスが責任を持って
 *	インデクスを構築しる!
 *	creat()で負けたプロセスは、自力で処理しる!
 *	インデクス構築が終わったら、versionをセットしる!
 *
 *  【オープンのやり方】
 *    * MAP_SHAREDで開くべし。
 *
 *  【古いバージョンの廃棄のやり方】
 *    * ファイルをディレクトリからunlink
 *    * あとは【新規作成】へ。
 *
 *  【破壊更新のやり方】
 *    * versionを0にする。
 *	version0に成功したプロセスが、
 *	責任を持って後の作業を執り行う。
 *	version0を取れなかったプロセスは、自力で処理しる!
 *    * いろいろ再構築終わったら、versionをセットしる!
 *
 */

/* 想定記事総数
   キリのいい数字であるべきだ */
#define DATINDEX_MAX_ARTICLES 1024

/* インデクス単位
   CHUNK_NUMの約数であるべきだ */
#define DATINDEX_CHUNK_SIZE 10

#define DATINDEX_IDX_SIZE ((DATINDEX_MAX_ARTICLES + DATINDEX_CHUNK_SIZE - 1) \
			   / DATINDEX_CHUNK_SIZE)

/* インデクス
   サイズが4096を超えるのは歓迎できない */
typedef struct DATINDEX
{
	/* バージョン情報。破壊操作のセマフォにもなっている */
	unsigned long volatile version;

	/* 発言数
	   chunk idx更新のセマフォにもなっている */
	unsigned long linenum;

	/* .dat の最終更新時刻 */
	time_t lastmod;

	/* スレタイトルへのオフセット */
	unsigned title_ofs;

	/* インデクス
	   本来ならば、全記事に対して
	   L-Mを持ちたいところなのだが、
	   そうするとサイズが大きくなりすぎて鬱だ…

	   各記事のL-Mは、chunk L-Mからの差分(16bit)で
	   持つようにし、差が18時間以上あるものは
	   特別扱いする、というkludgeも考えられるが… */
	struct
	{
		unsigned nextofs;
		time_t lastmod;
		unsigned valid_bitmap;
		long pad;
	} idx[DATINDEX_IDX_SIZE];

	/* XXX うめぐさ1 */
	unsigned pad1[607];
#if 0
	/* 有効発言 bitmap
	   little endian なので、LSBから数えること。
	   あぼーんされている発言は 0 になる。*/
	unsigned long valid_bitmap[DATINDEX_MAX_ARTICLES / 32];

	/* XXX うめぐさ2 */
	unsigned pad2[223];
#endif

	/* シグネチャ */
	unsigned long signature;
} DATINDEX;

struct DATINDEX_STRING
{
	char const *p;
	int len;
};

struct DATINDEX_LINE
{
	/* これらのポインタ同士を再計算すべきではない XXX */
	struct DATINDEX_STRING name;
	struct DATINDEX_STRING mailto;
	struct DATINDEX_STRING date;
	struct DATINDEX_STRING text;
	int len;	/* 未計算の時は 0 が入る */
	time_t lastmod;	/* あぼーん時には 0 を入れれ */
};

/* index管理
   べつにグローバル変数でもいいけどね */
typedef struct DATINDEX_OBJ
{
	/* .dat本体 */
	struct stat dat_stat;
	char *private_dat;

	/* .datを読んで生成されるテーブル */
	struct DATINDEX_LINE *line;
	/* lineの個数 */
	int linenum;

	/* インデクス */
	DATINDEX volatile *shared_idx;
} DATINDEX_OBJ;

/* 注意: i386 では動きませーん(藁 */
#define DATINDEX_CMPXCHG(sem, cur, new) \
	({ \
		long eax; \
		__asm__ ("cmpxchg %2,%3": \
			 "=a"(eax) : \
			 "0"(cur), \
			 "r"(new), \
			 "g"(sem)); \
		eax; \
	 })

/* 破壊更新のために、idxを排他取得する
   勝ったら 非0 (というよりDATINDEX_VERSION)を戻す
   負けたら 0 を戻す
   注意: i386 では動きませーん(藁 */
#if 0
#define DATINDEX_GET_DESTRUCTIVE_UPDATE(idx) \
	((idx)->version == DATINDEX_VERSION \
	 ? ({ \
		long eax; \
		__asm__ ("cmpxchg %2,%3": \
			 "=a"(eax) : \
			 "0"(DATINDEX_VERSION), \
			 "r"(0), \
			 "g"((idx)->version)); \
		eax; \
	 }) : 0)
#endif

/* 命名規則を間違えた…鬱
   公開インタフェイスは、
   <module>_<method>()
   って命名にするようにしよう、今後は。 */

/* インデクスを dat に読み込む
   dat は完全に初期化が行われる */
extern int datindex_open(DATINDEX_OBJ *dat,
			 char const *bs, long ky);

/* lastmodを拾い上げる
   first は、!is_nofirst() であることに注意 */
extern time_t datindex_lastmod(DATINDEX_OBJ const *dat,
			       int first,	/* 1番目を含める */
			       int st,
			       int to,
			       int ls);

/* 実は漏れ、C++大好きなんだ…
   なんとなくそれを匂わせる書き方になってるでしょ?(鬱
   (6411) */

#endif /* DATINDEX_H__ */
