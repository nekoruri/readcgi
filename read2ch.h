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
#define CUTRESLINK
#define LINKTAGCUT (1)

/* 一番最後に、「更新したレスを表示」のリンクを表示する */
#define RELOADLINK

/* Last-Modified を追加する */
#define LASTMOD

/* Last-Modified に併せて、Expires: も出力してみる */
/* #define EXPIRES */

/* 最適化された BadAccess を使用する */
#define NEWBA

/* nofirst → n など、短縮形で指示できるようにする */
#define GSTR2

/* teri タイプのログを読み込む */
#define TYPE_TERI

/* PATH_INFOを処理することにより、
   read.cgi/board/nnnnnn/?query
   のリクエストを処理できるようになる */
#define USE_PATH

/* Cookie による名前、E-mail フィールドの初期値の埋め込みを CGI 側で行う
 * Last-Modified付加により、proxyでキャッシュされた場合に各種の不都合
 * (最悪の場合、キャップ・トリップのパス漏れ)が発生するため使用不可に
 */
/* #define COOKIE */

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

/* if index2.cgi exists, return link use index2.cgi */
/* #define USE_INDEX2CGI */

/* check server use mod_gzip? */
#define CHECK_MOD_GZIP

/* rewrite_href2を復活させる場合に定義 */
/* #define REWRITE_HREF2 */

/* "投稿日："を非表示にする */ 
#define CUT_DATE_STRING

/* <a name=...>をつける。これをつけないときはCHUNK_ANCHORも無効にしたほうが良い */ 
#define CREATE_NAME_ANCHOR 

#endif /* READ2CH_H__ */
