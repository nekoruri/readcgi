/* read2ch.h */

#ifndef READ2CH_H__
#define READ2CH_H__

#define	CGINAME	"read.cgi"
#define	CGIVER	"read.cgi ver14 (01/08/28)"
#define	RES_YELLOW	900
#define	RES_REDZONE	950
#define	RES_RED		1000
#define	RES_IMODE	10
#define	RES_NORMAL	100
#define	MAX_FILESIZE	(8192 * 64)
#define	SIZE_BUF	0xa000

#define	LIMIT_PM	23
#define	LIMIT_AM	3

/* 混雑時間帯に、>>000 形式のレスへのリンクを削除する */
#define LINKTAGCUT (1)

/* 一番最後に、「更新したレスを表示」のリンクを表示する */
#define RELOADLINK

/* nofirst → n など、短縮形で指示できるようにする */
#define GSTR2

/* teri タイプのログを読み込む */
#define TYPE_TERI

/* PATH_INFOを処理することにより、
   read.cgi/board/nnnnnn/?query
   のリクエストを処理できるようになる */
#define USE_PATH

/* 書き込み直後のリロードを防止する */
#define PREVENTRELOAD

/* 上記の PREVENTRELOAD を指定したときに、更新されていても 304 を返す秒数 */
#define FORCE_304_TIME 60

/* gzip を使用して圧縮する */
#define GZIP

/* zlib を使用して圧縮する */
#define ZLIB

/* raw=xxx.yyyを有効にする。 xxx=最終レス番号, yyy=そのときのサイズ。
 * 一行目はステータス:
 *	[+OK] の場合は差分のみを送信する。
 *	[-INCR] (Incorrect)の場合はすべてのデータを送信する。
 *	[-ERR (テキスト)]の場合はなんかエラーが起きた。
 */
#define RAWOUT

/* mmap(2) を活用。
   資源の開放は積極的にサボりたい。 */
#define USE_MMAP

/* munmap(2)等の明示的な資源開放 */
/* #undef EXPLICIT_RELEASE */

/* read.cgi側によるindexの実装(experimental) */
/* #define USE_INDEX */


/** Header custumization **/

/* 「全部読む」をつける */
#undef ALL_ANCHOR 

/* pageの topに、CHUNK_NUM番ごとに区切ったレスへの anchorをつける
 */
#define CHUNK_ANCHOR
#define CHUNK_NUM 50

/* 「最新レス LATEST_NUM」をつける */
#define LATEST_ANCHOR
#define LATEST_NUM 50


/* sageレスのとき、名前を太字にしない */
/* #define SAGE_IS_PLAIN */

/* スレダイジェスト出力パラメータ
   本来ならばどこかから取得できる? */
#define N_INDEX_DIGESTS	10
#define N_INDEX_THREADS	30

/* 板毎に設定が書いてあるファイル、使うけ？ */
#define	USE_SETTING_FILE
#define	SETTING_FILE_NAME	"SETTING_R.TXT"

/* check server use mod_gzip? */
#define CHECK_MOD_GZIP

/* "投稿日："を非表示にする */ 
#define CUT_DATE_STRING

/* <a name=...>をつける。これをつけないときはCHUNK_ANCHORも無効にしたほうが良い */ 
#define CREATE_NAME_ANCHOR 

/* nofirst=trueを短縮形にする。bbs.cgiの出力と同一形式が望ましい */
/* #define NO_FIRST "&n=t" */

/* 「大きすぎます」の警告を出すまでのサイズ(KB単位) 
　MAX_FILESIZE - CAUTION_FILESIZE*1024 を超えたら警告 
　これが未定義なら、警告は出さない */ 
#define CAUTION_FILESIZE 16 
/* デバッグ用 
#define MAX_FILESIZE_BUSY (MAX_FILESIZE - 64 * 1024) */ 

#endif /* READ2CH_H__ */
