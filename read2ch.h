/* read2ch.h */

/* 混雑時間帯に、>>000 形式のレスへのリンクを削除する */
#define CUTRESLINK

/* 一番最後に、「更新したレスを表示」のリンクを表示する */
/* #define RELOADLINK */

/* Last-Modified を追加する */
#define LASTMOD

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

/* Cookie による名前、E-mail フィールドの初期値の埋め込みを CGI 側で行う */
#define COOKIE

/* 書き込み直後のリロードを防止する */
#define PREVENTRELOAD

/* 上記の PREVENTRELOAD を指定したときに、更新されていても 304 を返す秒数 */
#define FORCE_304_TIME 30

/* gzip を使用して圧縮する */
#define GZIP

/* zlib を使用して圧縮する */
#define ZLIB
