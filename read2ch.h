/* read2ch.h */

#ifndef READ2CH_H__
#define READ2CH_H__

#define	CGINAME	"read.cgi"
#define	CGIVER	"read.cgi ver5.22+ (01/09/19-)"
#define	RES_YELLOW	900
#define	RES_REDZONE	950
#define	RES_RED		1000
#define	RES_IMODE	10
#define	RES_NORMAL	100
#define	MAX_FILESIZE	(8192 * 64)
#define	SIZE_BUF	0xa000

#define	LIMIT_PM	23
#define	LIMIT_AM	3

#define DAT_DIR "../%.256s/dat/"
#define TEMP_DIR "../%.50s/temp/"
#define KAKO_DIR "../%.50s/kako/"

/* 混雑時間帯に、>>000 形式のレスへのリンクを削除する */
#define LINKTAGCUT (1)

/* always LINKTAGCUT */
/* #define NORMAL_TAGCUT */

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

/* 非パス仕様のURLで呼ばれたときはBASEを出力して、常にパス仕様でリンクを生成 */
#define ALWAYS_PATH

/* 書き込み直後のリロードを防止する */
#define PREVENTRELOAD

/* 上記の PREVENTRELOAD を指定したときに、更新されていても 304 を返す秒数 */
#define FORCE_304_TIME 60

/* gzip を使用して圧縮する */
#define GZIP

/* zlib を使用して圧縮する */
#define ZLIB

/* raw=xxx.yyyを有効にする。 xxx=最終レス番号, yyy=そのときのサイズ。
 * 出力の一行目はステータス:
 *	[+OK size]
 *        2行目以降はxxx番以降の差分データ。sizeは差分のデータサイズ。
 *	[-INCR size] (yyy が一致していなかった場合に起きる; あぼーんの可能性?)
 *        2行目以降はスレのデータ全部。sizeはデータサイズ。
 *	[-ERR string]
 *        なんかエラーが起きた。stringはエラーの内容。
 *          "このスレッド大きすぎます。"
 *          "そんな板orスレッドないです。"
 *          "過去ログ倉庫で発見 %s" %sは相対パスで.datの位置を表す。
 *          "html化待ち"
 *          "メモリの確保に失敗しました。"
 *          "調整中。。。"
 *          "なんか不調です。"
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
#define CHUNK_NUM 100

/* CUUNK_ANCHOR使用時に>>nnをCHUNKごとのリンクにする */
/* #define USE_CHUNK_LINK */

/* 「最新レス LATEST_NUM」をつける */
/* #define LATEST_ANCHOR 常にON */
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

/* "名前："を非表示にする */ 
#define CUT_NAME_STRING

/* "投稿日："を非表示にする */ 
#define CUT_DATE_STRING

/* <a name=...>をつける。これをつけないときはCHUNK_ANCHORも無効にしたほうが良い */ 
/* #define CREATE_NAME_ANCHOR  */

/* nofirst=trueを短縮形にする。bbs.cgiの出力と同一形式が望ましい */
/* #define NO_FIRST "&n=t" */

/* 「大きすぎます」の警告を出すまでのサイズ(KB単位) 
　MAX_FILESIZE - CAUTION_FILESIZE*1024 を超えたら警告 
　これが未定義なら、警告は出さない */ 
#define CAUTION_FILESIZE 16 
/* デバッグ用 
#define MAX_FILESIZE_BUSY (MAX_FILESIZE - 64 * 1024) */ 

/* NN4.xでgzip圧縮時にContent-Lengthがあると
  Last-Modifiedが無効になる問題の対策 */
#define NN4_LM_WORKAROUND

/* select form形式で CHUNKED_ANCHORを表示する */
/* 「掲示板に戻る」「レスを全部」「最新レス」との統一が取れていない */
/* #define　CHUNKED_ANCHOR_WITH_FORM */

/* 1- 51- ...形式のリンクを上下に分けて表示する */
/* #define SEPARATE_CHUNK_ANCHOR */

/* 前50 次50 を上下につける。50=CHUNK_NUM */
#define PREV_NEXT_ANCHOR

/* かちゅーしゃによるアクセスを拒絶する */
#define Katjusha_Beta_kisei

/* 名前の緑表示をCSSを使って表示 */
/* #define USE_CSS */

/* 混雑時間帯に、>>nn形式のリンクから参照された場合に
   出力htmlから余分なリンク等を省く */
#define	REFERDRES_SIMPLE /**/

/* 混雑時間帯にレス本文内のURLをリンク化しないようにする */
/* #define NO_LINK_URL_BUSY */

/* ETagを出力する */
/* #define PUT_ETAG */

#endif /* READ2CH_H__ */
