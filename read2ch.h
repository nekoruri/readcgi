/* read2ch.h */

#ifndef READ2CH_H__
#define READ2CH_H__

#define	CGINAME	"read.cgi"
#define	CGIVER	"read.cgi ver5.25+ (01/10/12-)"
#define	RES_YELLOW	900
#define	RES_REDZONE	950
#define	RES_RED		1000
#define	RES_IMODE	10
#define	RES_NORMAL	100
#define	MAX_FILESIZE	(8192 * 64)
#define	SIZE_BUF	0xa000

/* 英語版にする(r2chhtml.h ではなく r2chhtml_en.h がインクルードされる) */ 
/* #define ENGLISH */

#define CONFIG_TIMEZONE "JST-9"
#define	LIMIT_PM	22
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
 *	[+OK size/maxK]
 *        2行目以降はxxx番以降の差分データ。sizeは差分のデータサイズ。
 *        maxはMAX_FILESIZE/1024。 
 *	[-INCR size/maxK] (yyy が一致していなかった場合に起きる; あぼーんの可能性?)
 *        2行目以降はスレのデータ全部。sizeはデータサイズ。
 *        maxはMAX_FILESIZE/1024。 
 *	[-ERR string]
 *        なんかエラーが起きた。stringはエラーの内容。
 *          "このスレッド大きすぎます。"
 *          "そんな板orスレッドないです。"
 *          "過去ログ倉庫で発見 %s" %sは相対パスで.datの位置を表す。
 *          "html化待ち"
 *          "メモリの確保に失敗しました。"
 *          "調整中。。。"
 *          "なんか不調です。"
 *  リクエストにIf-Modified-Sinceが無く、
 *  取得済みサイズとdatのファイルサイズが一致した場合、非圧縮の
 *  [+OK 0/maxK]
 *  の一行だけが返り、LastModified,Content-Length,Cotent-Encoding等は含まれない
 */
#define RAWOUT

/* raw=0.0のリクエスト時、st=nn, to=nn, ls=nnを参照する
 * ・有効な場合にはステータスが以下のように変わる
 * [+PARCIAL size/maxK]
 * ・またステータス行に、以下の情報がTAB区切りで加えられる。
 * Range:n1-n2/n3
 *     送信バイトの位置情報。
 *     HTTP レスポンスヘッダのContent-Range: bytes 以下と同じもの。
 * Res:n1-n2/n3
 *     送信レスのレス番号情報。
 *     Rangeと異なり、1から始まり最後まで送った場合はn2==n3となる。
 * ・その他、以下の情報が含まれるようになる可能性がある。
 * Status:Stopped
 *     スレッドストップがかかったと判定された場合。
 * Location:temp/
 *     datはhtml化を待っている状態の過去ログである(書きこめない)
 * Location:kako/
 *     datはhtml化されている過去ログである(書きこめない)
 */
#define	RAWOUT_PARTIAL

/* mmap(2) を活用。
   資源の開放は積極的にサボりたい。 */
#define USE_MMAP

/* munmap(2)等の明示的な資源開放 */
/* #undef EXPLICIT_RELEASE */

/* read.cgi側によるindexの実装(experimental) */
/* #define USE_INDEX */


/** Header custumization **/

/* 「全部読む」をつける */
/* #define ALL_ANCHOR 常にON */

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

/* 設定にSETTINGファイルではなく、SPECIAL_SETTINGを利用する */
#define USE_INTERNAL_SETTINGS
#define SPECIAL_SETTING \
	{ "saku", "LINKTAGCUT=0" "\n" }, \
/*	{ "dancesite", "FORCE_304_TIME=3" "\n" }, */

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

/* スレッドストップ扱いにするサイズ(KB単位)
   MAX_FILESIZE - WRITESTOP_FILESIZE*1024 を超えたらFORM等を出さない
   これが未定義なら、大きくてもスレッドストップ扱いはしない */
#define WRITESTOP_FILESIZE	5

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

/* かちゅ〜しゃgzip化DLLのリクエストに応答する
 * raw=.nnn形式のリクエストを受け付け、
 * あぼーんが想定される場合、
 * [-ERR どこかであぼーんがあったみたいです。]
 * を返し、本文の再送は行わない。
 */
#define Katjusha_DLL_REPLY

/* 名前の緑表示をCSSを使って表示 */
/* #define USE_CSS */

/* 混雑時間帯に、>>nn形式のリンクから参照された場合に
   出力htmlから余分なリンク等を省く */
#define	REFERDRES_SIMPLE /**/

/* 混雑時間帯にレス本文内のURLをリンク化しないようにする */
/* #define NO_LINK_URL_BUSY */

/* ETagを出力する */
/* #define PUT_ETAG */

/* dat落ち、過去ログをread.cgiで読む */
/* #define READ_KAKO */
/* #define READ_TEMP */
/* #define AUTO_KAKO */

/* READ_KAKOで過去ログに誘導するときに範囲指定を継承する */
#define READ_KAKO_THROUGH

/* AUTO_KAKOで過去ログを取得/閲覧できる制限を設定
  0 = RAWモード時のみ、temp/,kako/どちらのdatも取得可能
  1 = RAWモード以外は、temp/のみ閲覧可能
  2 = RAWモード以外でも、temp/,kako/どちらも閲覧可能  */
#define	AUTO_KAKO_MODE	0

/* ログ形式を実行時に判別する。READ_KAKO/AUTO_KAKO用 */
/* #define AUTO_LOGTYPE */

/* 名前末尾・メール末尾・<br>タグ前・レス末尾の空白を
(可能なら)全て削除する */
#define CUT_TAIL_BLANK

/* '<',"http://"の直前の文字を調べ、必要ならば空白を加える。 */
/* #define STRICT_ILLEGAL_CHECK */

#endif /* READ2CH_H__ */
