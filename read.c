#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<time.h>
#include	<unistd.h>
#ifdef HAVE_READ2CH_H
# include	"read2ch.h"
#endif
#ifdef USE_MMAP
#include	<sys/mman.h>
#endif

#include "digest.h"
#include "read.h"

#ifdef ZLIB
# ifndef GZIP
#  define GZIP			/* gzip由来のコードも使用するので */
# endif
# include	<zlib.h>
#endif

#if defined(GZIP) && !defined(ZLIB)
#include        <sys/wait.h>
#endif

#include	"r2chhtml.h"
#ifdef PREVENTRELOAD
# ifndef LASTMOD
#  define LASTMOD
# endif
# ifndef FORCE_304_TIME
#  define FORCE_304_TIME  30    /* 秒で指定 */
# endif
# include        "util_date.h" /* from Apache 1.3.20 */
#endif

#if	(defined(CHUNK_ANCHOR) && CHUNK_NUM > RES_NORMAL) 
# error "Too large CHUNK_NUM!!"
#endif

/* TERIタイプで','が置換されて格納される文字列 */
#define COMMA_SUBSTITUTE "\x81\x97\x81\x4d" /* "＠｀" */
#define COMMA_SUBSTITUTE_FIRSTCHAR 0x81
#define COMMA_SUBSTITUTE_LEN 4

char const *zz_remote_addr;
char const *zz_remote_host;
char const *zz_http_referer;
char const *zz_http_cookie;
#ifdef USE_PATH
char const *zz_path_info;
/* 0 のときは、pathは適用されていない
   read.cgi/tech/998845501/ のときは、3になる */
int path_depth;
#endif
char const *zz_query_string;
char *zz_temp;
char const *zz_http_user_agent;
char const *zz_http_language;
#ifdef GZIP
char const *zz_http_encoding;
int gzip_flag;
#endif
#ifdef CHECK_MOD_GZIP
char const *zz_server_software;
#endif

#ifdef LASTMOD
char const *zz_http_if_modified_since;
time_t zz_fileLastmod;
char lastmod_str[1024];
char expires_str[1024];
#endif

#ifdef USE_MMAP
static int zz_mmap_fd;
static size_t zz_mmap_size;
#endif

char zz_bs[1024];
char zz_ky[1024];
char zz_ls[1024];
char zz_st[1024];
char zz_to[1024];
char zz_nf[1024];
char zz_im[1024];
#ifdef RAWOUT
char zz_rw[1024];
#endif
int nn_st, nn_to, nn_ls;
char *BigBuffer = NULL;
char const *BigLine[RES_RED + 16];

#define is_imode() (*zz_im == 't')
#define is_nofirst() (*zz_nf == 't')

char *KARA = "";
int zz_fileSize = 0;
int lineMax = -1;
int out_resN = 0;

time_t t_now;
struct tm tm_now;
long currentTime;
int isbusytime = 0;

char const *LastChar(char const *src, char c);
char *zz_GetString(char *dst, size_t dst_size, char const *tgt);
char *doReplace(char *des, char const *str0, char const *str1);
void html_foot_im(int,int);
void html_head(int level, char const *title, int line);
int res_split(char **s, char *p);
void someReplace(char const *src, char *des, char const *str0, char const *str1);
void hlinkReplace(char *src);
static void html_foot(int level, int line,int);
int getLineMax(void);
int IsBusy2ch(void);
int getFileSize(char const *file);
static int isprinted(int lineNo);
#ifndef CUTRESLINK
/*int res_split(char **s, char *p);*/
char *findSplitter(char *stt, int sp);
#endif
#ifdef RELOADLINK
void html_reload(int);
#endif
#ifdef RAWOUT
int rawmode;
int raw_lastnum, raw_lastsize; /* clientが持っているデータの番号とサイズ */
#endif

#ifdef ZLIB
/*  zlib対応 */
gzFile pStdout; /*  = (gzFile) stdout; */
zz_printf_t pPrintf = (zz_printf_t) fprintf;

/* zlib独自改造 */
extern int gz_getdata(char **buf);
/*
 * gzdopen(0,"w")でmemoryに圧縮した結果を受け取る
 * len = gz_getdata(&data);
 *   int   len  : 圧縮後のbyte数
 *   char *data : 圧縮後のデータ、使用後free()すべき物
 */
#else
static int pid;
#endif

#ifdef	USE_SETTING_FILE
/*
	SETTING_R.TXTは
	---
	FORCE_304_TIME=30
	LIMIT_PM=23
	RES_NORMAL=50
	MAX_FILESIZE=512
	LINKTAGCUT=0
	---
	など。空行可。
	#と;から開始はコメント・・・というより、
	=がなかったり、マッチしなかったりしたら無視
	最後の行に改行が入ってなかったら、それも無視される(バグと書いて仕様と読む)
	
	RES_YELLOW-RES_NORMALまでは、#defineのままでいいかも。
*/
struct {
	int	Res_Yellow;
	int Res_RedZone;
	int	Res_Imode;
	int Res_Normal;
	int Max_FileSize;	/*	こいつだけ、KByte単位	*/
	int Limit_PM;
	int Limit_AM;
#ifdef PREVENTRELOAD
	int Force_304_Time;
#endif
#ifdef	LATEST_ANCHOR
	int Latest_Num;
#endif
#ifdef	CUTRESLINK
	int LinkTagCut;
#endif
} Settings = {
	RES_YELLOW,
	RES_REDZONE,
	RES_IMODE,
	RES_NORMAL,
	MAX_FILESIZE / 1024,
	LIMIT_PM,
	LIMIT_AM,
#ifdef	PREVENTRELOAD
	FORCE_304_TIME,
#endif
#ifdef	LATEST_ANCHOR
	LATEST_NUM,
#endif
#ifdef	CUTRESLINK
	LINKTAGCUT,
#endif
};
struct {
	const char *str;
	int *val;
	/*	文字列の長さをあらかじめ数えたり、２分探索用に並べておくのは、
		拡張する時にちょっと。
		負荷が限界にきていたら考えるべし。
	*/
} SettingParam[] = {
	{	"RES_YELLOW",	&Settings.Res_Yellow,	},
	{	"RES_REDZONE",	&Settings.Res_RedZone,	},
	{	"RES_IMODE",	&Settings.Res_Imode,	},
	{	"RES_NORMAL",	&Settings.Res_Normal,	},
	{	"MAX_FILESIZE",	&Settings.Max_FileSize,	},
	{	"LIMIT_PM",		&Settings.Limit_PM,		},
	{	"LIMIT_AM",		&Settings.Limit_AM,		},
#ifdef	PREVENTRELOAD
	{	"FORCE_304_TIME",	&Settings.Force_304_Time,	},
#endif
#ifdef	LATEST_ANCHOR
	{	"LATEST_NUM",	&Settings.Latest_Num,	},
#endif
#ifdef	CUTRESLINK
	{	"LINKTAGCUT",	&Settings.LinkTagCut	},
#endif
};
#undef	RES_YELLOW
#define	RES_YELLOW	Settings.Res_Yellow
#undef	RES_REDZONE
#define	RES_REDZONE	Settings.Res_RedZone
#undef	RES_IMODE
#define	RES_IMODE	Settings.Res_Imode
#undef	RES_NORMAL
#define	RES_NORMAL	Settings.Res_Normal
#undef	MAX_FILESIZE
#define	MAX_FILESIZE	(Settings.Max_FileSize * 1024)
#undef	LIMIT_PM
#define	LIMIT_PM	Settings.Limit_PM
#undef	LIMIT_AM
#define	LIMIT_AM	Settings.Limit_AM
#ifdef	PREVENTRELOAD
#undef	FORCE_304_TIME
#define	FORCE_304_TIME	Settings.Force_304_Time
#endif
#ifdef	LATEST_ANCHOR
#undef	LATEST_NUM
#define	LATEST_NUM	Settings.Latest_Num
#endif
#ifdef	CUTRESLINK
#undef	LINKTAGCUT
#define	LINKTAGCUT	Settings.LinkTagCut
#endif
#endif	/*	USE_SETTING_FILE	*/

#ifdef CUTRESLINK
/* <ctype.h>等とかぶってたら、要置換 */
#define false (0)
#define true (!false)
#define _C_ (1<<0) /* datチェック用区切り文字等 */
#define _U_ (1<<1) /* URLに使う文字 */
#define _S_ (1<<2) /* SJIS1バイト目＝<br>タグ直前の空白が削除可かを適当に判定 */

/* #define isCheck(c) (flagtable[(unsigned char)(c)] & _C_) */
#define isCheck(c) (flagtable[/*(unsigned char)*/(c)] & _C_)
#define isSJIS1(c) (flagtable[(unsigned char)(c)] & _S_)
#define hrefStop(c) (!(flagtable[(unsigned char)(c)] & _U_))

#define	LINK_URL_MAXLEN		256
		/*	レス中でURLとみなす文字列の最大長。短い？	*/

#define _0____ (1<<0)
#define __1___ (1<<1)
#define ___2__ (1<<2)
#define ____3_ (1<<3)
#define _____4 (1<<4)

#define ______ (0)
#define _01___ (_0____|__1___|0)
#define __1_3_ (__1___|____3_|0)
#define ___23_ (___2__|____3_|0)
#define _0_23_ (_0____|___2__|____3_|0)


/*
	'\n'と':'を isCheck(_C_) に追加
	TYPE_TERIの時は、'\x81'と','をはずしてみた
	すこーーしだけ違うかも
*/
char flagtable[256] = {
	_0____,______,______,______,______,______,______,______, /*  00-07 */
	______,______,_0____,______,______,______,______,______, /*  08-0F */
	______,______,______,______,______,______,______,______, /*  10-17 */
	______,______,______,______,______,______,______,______, /*  18-1F */
	_0____,__1___,______,__1___,__1___,__1___,_01___,______, /*  20-27 !"#$%&' */
#ifdef	TYPE_TERI
	______,______,__1___,__1___,__1___,__1___,__1___,__1___, /*  28-2F ()*+,-./ */
#else
	______,______,__1___,__1___,_01___,__1___,__1___,__1___, /*  28-2F ()*+,-./ */
#endif
	__1___,__1___,__1___,__1___,__1___,__1___,__1___,__1___, /*  30-37 01234567 */
	__1___,__1___,_01___,__1___,_0____,__1___,______,__1___, /*  38-3F 89:;<=>? */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  40-47 @ABCDEFG */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  48-4F HIJKLMNO */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  50-57 PQRSTUVW */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  58-5F XYZ[\]^_ */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  60-67 `abcdefg */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  68-6F hijklmno */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  70-77 pqrstuvw */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,______, /*  78-7F xyz{|} */
#ifdef	TYPE_TERI
	____3_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  80-87 */
#else
	____3_,_0_23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  80-87 */
#endif
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  88-8F */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  90-97 */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  98-9F */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  A0-A7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  A8-AF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  B0-B7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  B8-BF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  C0-C7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  C8-CF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  D0-D7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  D8-DF */
	____3_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E0-E7 */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E8-EF */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  F0-F7 */
	___23_,___23_,___23_,___23_,___23_,______,______,______, /*  F8-FF */
};

typedef struct { /*  class... */
	char **buffers; /* csvの要素 */
	int rest; /* 残りのバッファサイズ・・厳密には判定してないので、数バイトは余裕が欲しい */
	int isTeri;
} ressplitter;

/*
  初期化
  toparray ポインタ配列のアドレス
  buff コピー先のバッファの先頭
  bufsize 厳密には判定してないので、数バイトは余裕が欲しい
  */

void ressplitter_init(ressplitter *This, char **toparray, char *buff, int bufsize/*, bool isteri*/)
{
	This->buffers = toparray;
	This->rest = bufsize;
	This->isTeri = true; /* レス１をstrstr("<>")した結果を設定すべき */
	*This->buffers = buff;
}

/* <a href="xxx">をPATH向けに打ち直す
   *spは"<a "から始まっていること。
   dp, sp はポインタを進められる
   書き換える場合は</a>まで処理するが、
   書き換え不要な場合は<a ....>までママコピ(もしくは削る)だけ
   走査不能な場合は、0 を戻す

   ad hocに、中身が&gt;で始まってたら
   href情報をぜんぶ捨てて、
   <A></A>で囲まれた部位を取り出し、
   自ら打ち直すようにする
   想定フォーマットは以下のもの。
   &gt;&gt;nnn
   &gt;&gt;xxx-yyy */
static int rewrite_href(char **dp,		/* 書き込みポインタ */
			 char const **sp,	/* 読み出しポインタ */
			 int istagcut)		/* タグカットしていいか? */
{
	char depth_expr[64];
	char *d = *dp;
	char const *s = *sp;
	int n;
	int f_processed = 0;

#ifdef USE_PATH
	if (path_depth == 0) {
		sprintf(depth_expr, "./read.cgi/%.20s/%.20s/", zz_bs, zz_ky );
	} else
	if (path_depth == 2) {
		strncpy(depth_expr,
			zz_ky,
			sizeof(depth_expr) - 4);
		depth_expr[sizeof(depth_expr) - 4] = 0;
		strcat(depth_expr, "/");
	} else
		depth_expr[0] = 0;
#else
	sprintf(depth_expr, "./read.cgi?bbs=%.20s&key=%.20s", zz_bs, zz_ky );
#endif

	/* 閉じ位置を探す */
	n = strcspn(*sp, ">");
	if (n == 0)
		return 0;
	s += n + 1;	/* まだdは進めないでおく */
	if (!memcmp(s, "&gt;&gt;", 8)) {
		char const * copy_start = s;
		int copy_len;
		int st, to;
		int mst, mto;
		char buf[8];
		s += 8;
		/* 1番目の数字を処理する */
		n = strspn(s, "0123456789");
		if (n == 0)
			return 0;
		if (n > sizeof(buf) - 1)
			n = sizeof(buf) - 1;
		strncpy(buf, s, n);
		s += n;
		buf[sizeof(buf) - 1] = 0; /* sentinel */
		st = to = atoi(buf);
		f_processed = 1;
		/* 2番目の数字があるなら、処理する */
		if (s[0] == '-') {
			n = strspn(++s, "0123456789");
			if (n == 0)
				return 0;
			if (n > sizeof(buf) - 1)
				n = sizeof(buf) - 1;
			strncpy(buf, s, n);
			s += n;
			buf[sizeof(buf) - 1] = 0; /* sentinel */
			to = atoi(buf);
		}
		/* </a>があるはずなので探し、捨てる */
		s = strstr(s, "</a>");
		if (!s)
			return 0;
		s += 4;
		copy_len = s - copy_start;

		/* chunk仕様を生かすためのkludgeは以下に。 */
		mst = (st - 1) / CHUNK_NUM;
		mto = (to - 1) / CHUNK_NUM;

		if (mst == mto) {
			/* chunk範囲 */
			mst = 1 + CHUNK_NUM * mst;
			mto = CHUNK_NUM * (mto + 1);
		} else {
			/* chunkをまたぎそうなので、最小単位を。*/
			mst = st;
			mto = to;
		}

		/* 新しい表現をブチ込む */
		if (isprinted(st) && isprinted(to)) {
			d += sprintf(d,
				     "<a href=#%u>", 
				     st);
		} else
		{
#ifdef USE_PATH
			d += sprintf(d,
				     "<a href=\"%s%d-%d#%d\">",
				     depth_expr,
				     mst, mto, st );
#else
			d += sprintf(d,
				     "<a href=\"%s&st=%d&to=%d&nofirst=true#%d\">",
				     depth_expr,
				     mst, mto, st );
#endif
		}

		/* "&gt;&gt;".."</a>"を丸写し */
		memcpy( d, copy_start, copy_len );
		d += copy_len;
	}

	/* あとしまつ */
	if (f_processed) {
		/* ポインタが進んだはず */
		*sp = s;
	} else {
		/* 単純に走査し直す */
		s = *sp;
		n = strcspn(s, ">");
		if (n == 0)
			return 0;
		n++;
		if (!istagcut) {
			memcpy(d, s, n);
			d += n;
		}
		s += n;
		*sp = s;
	}
	*dp = d;
	return 1;
}

/*
	pは、"http://"の':'を指してるよん
	何文字さかのぼるかを返す。0ならnon-match
	４文字前(httpの場合)からスキャンしているので、
	安全を確認してから呼ぶ
*/
int isurltop(const char *p)
{
	if (strncmp(p-4, "http://", 7) == 0)
		return 7-3;
	if (strncmp(p-3, "ftp://", 6) == 0)
		return 6-3;
	return 0;
}
/*
	pは、"http://www.2ch.net/..."の最初の'w'を指してる
	http://wwwwwwwww等もできるだけ判別
	urlでないとみなしたら、0を返す
*/
int geturltaillen(const char *p)
{
	const char *top = p;
	int len = 0;
	while (!hrefStop(*p)) {
		if (*p == '&') {
			/*  cutWordOff(url, "&quot;");
				ちょっとやっていることがわからないが、そのまま動作を再現
				どうせならstrnicmp()の方が・・・
			*/
			if (strncmp(p, "&quot;", 6) == 0)
				break;
		}
		++len;
		++p;
	}
	if (len) {
		if (memchr(top, '.', len) == NULL	/* '.'が含まれないURLは無い */
			|| *(top + len - 1) == '.')	/* '.'で終わるURLは(普通)無い */
			len = 0;
		if (len > LINK_URL_MAXLEN)	/* 長すぎたら却下 */
			len = 0;
	}
	return len;
}

/*
	urlから始まるurllen分の文字列をURLとみなしてbufpにコピー。
	合計何文字バッファに入れたかを返す。
*/
int urlcopy(char *bufp, const char *url, int urllen)
{
	return sprintf(bufp,
		"<a href=\"%.*s\" target=\"_blank\">%.*s</a>", 
		/* 小文字にしてすこーーーしだけ圧縮効果を期待したり */
		/* "<A HREF=\"%.*s\" TARGET=\"_blank\">%.*s</A>", */
		urllen, url, urllen, url);
}

/*
	resnoが、
	出力されるレス番号である場合true
	範囲外である場合false
	判定はdat_out()を基本に
*/
static int isprinted(int lineNo)
{
	if (lineNo == 1) {
		if (is_nofirst())
			return false;
	} else {
		if (nn_st && lineNo < nn_st)
			return false;
		if (nn_to && lineNo > nn_to)
			return false;
		if (nn_ls && (lineNo-1) < lineMax - nn_ls)
			return false;
	}
	return true;
}

/*
	rewrite_href()と同じようなことをしてるので、
	rewrite_href()が、path_depth == 0 時に対応したら、
	こっちは不要。
*/
static int rewrite_href2(char **dp,		/* 書き込みポインタ */
			 char const **sp,	/* 読み出しポインタ */
			 int istagcut)		/* タグカットしていいか? */
{
	/*	全てCHUNK仕様に書き換えるのは、
		追加書きこみがある場合に常にIf-Modified-Sinceが成立し、
		転送量を増やしてしまいそうなので、
		L-M対応が煮詰まるまであとまわし */
	char bbs[30];	/* bbs=xxx, key=nnn は、bbs.cgiで書きこまれるので、*/
	char key[20];	/* 必要充分で良い。(key=../kako/nnn/nnnの可能性も無い) */
	int st;
	int to;
	char last;
	
	/* >>nnは、bbs.cgiで範囲チェックをあまりしてないようで、
	   相当長い桁数がst=やto=に渡される場合がある
	   長過ぎるかどうかは&gt;&gt;と'<'の長さで調べ、
	   長過ぎればリンクしない。
	   また、(st==0 || to == 0) || (st > lineMax || to > lineMax)も
	   リンクしない。
	   レス番号が表示範囲内におさまっているかの判断は
	   dat_out()に準じる */
	/* 手抜き */
	if ((sscanf(*sp, 
		"<a href=\"../test/read.cgi?bbs=%20[a-zA-Z0-9_]&key=%12[0-9]"
		"&st=%u&to=%u&nofirst=true\" target=\"_blank\">&gt;&gt;%*u</a%c"
		, bbs, key, &st, &to, &last) == 5 && last == '>')	/* >>nn */
	|| (sscanf(*sp, 
		"<a href=\"../test/read.cgi?bbs=%20[a-zA-Z0-9_]&key=%12[0-9]"
		"&st=%u&to=%u\" target=\"_blank\">&gt;&gt;%*u-%*u</a%c"
		, bbs, key, &st, &to, &last) == 5 && last == '>')) {	/* >>nn-nn */
		const char *pr_start = strstr(*sp + 50, "&gt;&gt;");
		const char *pr_end = strchr(pr_start, '<');
		if (is_imode()
		|| pr_end - pr_start > 8 + 4 * 2 + 1
		|| (st == 0 || to == 0 || st > lineMax || to > lineMax)
#if	1	/* #ifndef USE_PATH */
		/* #ifndef を生かせば、isbusytimeでもa name=へのリンクがつく*/
		/* USE_PATHでなくても、a name=のanchorがつくようになった??	*/
		|| (istagcut && isprinted(st) && isprinted(to))
#endif
		) {
			/* タグを全部はずしてコピー */
			memcpy(*dp, pr_start, pr_end - pr_start);
			*dp += pr_end - pr_start;
			*sp = strchr(pr_end, '>') + 1;
			return 1;
		}
#ifdef	USE_PATH
		if (isprinted(st) && isprinted(to)) {
			/* <a href="#123">&gt;&gt;123<> に書き換え */
			*dp += sprintf(*dp, "<a href=#%u>%.*s</a>", st, pr_end - pr_start, pr_start);
			*sp = strchr(pr_end, '>') + 1;
		} else {
			/* PATH形式に書き換え */
			*dp += sprintf(*dp, st == to ? 
				"<a href=\"../test/read.cgi/%s/%s/%u\">":
				"<a href=\"../test/read.cgi/%s/%s/%u-%u\">",
				bbs, key, st, to);
			*dp += sprintf(*dp, "%.*s</a>", pr_end - pr_start, pr_start);
			*sp = strchr(pr_end, '>') + 1;
		}
#else
		/* そのままコピー */
		pr_end = strchr(pr_end, '>') + 1;
		memcpy(*dp, *sp, pr_end - *sp);
		*dp += pr_end - *sp;
		*sp += pr_end - *sp;
#endif
		return 1;
	}
	return 0;
}

/*
	findSplitterの代わり
	レスを全走査するが、コピーと変換(と削除)を同時に行う
	p コピー前のレス(BigBuffer内の１レス)
	resnumber	レス本文である場合にレス番号(行番号＋１)、それ以外は0を渡す
	istagcut	<a href=...>と</a>をcutするか
	Return		次のpの先頭
	non-TYPE_TERIなdatには,"<>"は含まれないはずなので、#ifdef TYPE_TERI は略
*/
const char *ressplitter_split(ressplitter *This, const char *p, int resnumber)
{
	char *bufp = *This->buffers;
	int bufrest = This->rest;
	int istagcut = (LINKTAGCUT && isbusytime && resnumber > 1) || is_imode();
	/*	ループ中、*This->Buffersはバッファの先頭を保持している	*/
	while (--bufrest > 0) {
		int ch = *p;
		if (isCheck(ch)) {
			switch (ch) {
			case ' ':
				/* 無意味な空白は１つだけにする */
				while (*(p+1) == ' ')
					p++;
				if (*(p+1) != '<')
					break;
				if (*(p+2) == '>') {
					if (bufp == *This->buffers) /* 名前欄が半角空白の場合 */
						*bufp++ = ' ';
					p += 3;
					goto Teri_Break;
				}
				if (memcmp(p, " <br> ", 6) == 0) {
					if (bufp != *This->buffers && isSJIS1(*(bufp-1))) {
						*bufp++ = ' ';
					}
					memcpy(bufp, "<br>", 4);
					p += 6;
					bufp += 4;
					continue;
				}
				break;
			case '<': /*  醜いが */
				if (*(p+1) == '>') {
					/* This->isTeri = true; */
					p += 2;
					goto Teri_Break;
				}
#ifdef	RAWOUT
				/* rawmode != 0 の時は呼ばれていないように思うけど・・?? */
				if (rawmode)
					break;
#endif
				if (resnumber && p[1] == 'a' && isspace(p[2])) {
					char *tdp = bufp;
					char const *tsp = p;
					if (path_depth && !is_imode()) {
						if (!rewrite_href(&tdp, &tsp, istagcut))
							goto Break;
					} else {
						if (!rewrite_href2(&tdp, &tsp, istagcut))
							break;	/* そのままコピーを続ける */
					}
					bufrest -= tdp - bufp;
					bufp = tdp;
					p = tsp;
					continue;
				}
				break;
			case '&':
				if (memcmp(p+1, "amp", 3) == 0) {
					if (*(p + 4) != ';')
						p += 4 - 1;
				}
				/* &MAIL->&amp;MAILの変換は、dat直読みのかちゅ～しゃが対応しないと無理
				   もし変換するならbbs.cgi */
				break;
			case ':':
#if	1
				if (resnumber) {
#else
				if (resnumber && !istagcut) {
					/* urlのリンクを(時間帯によって)廃止するなら */
#endif
					if (*(p+1) == '/' && *(p+2) == '/' && isalnum(*(p+3))) {
						/*
							正常なdat(名前欄が少なくとも1文字)ならば、
							pの直前は最低、" ,,,"となっている(さらに投稿日の余裕がある)。
							なので、4文字("http")まではオーバーフローの危険はない
							ただし、これらは、resnumberが、正確に渡されているのが前提。
						*/
						int pdif = isurltop(p);	/*	http://の場合、4が返る	*/
						if (pdif) {
							int taillen = geturltaillen(p + 3);
							if (taillen && bufrest > taillen * 2 + 60) {
								bufp -= pdif, p -= pdif, bufrest += pdif, taillen += pdif + 3;
								pdif = urlcopy(bufp, p, taillen);
								bufp += pdif, bufrest -= pdif, p += taillen;
								continue;
							}
						}
					}
				}
				break;
			case '\n':
				goto Break;
				/*	break;	*/
#ifndef TYPE_TERI
			case COMMA_SUBSTITUTE_FIRSTCHAR: /*  *"＠"(8197)  "｀"(814d) */
				/* if (!This->isTeri) { */
				if (memcmp(p, COMMA_SUBSTITUTE, COMMA_SUBSTITUTE_LEN) == 0) {
					ch = ',';
					p += 4 - 1;
				}
				/* } */
				break;
			case ',':
				/* if (!This->isTeri) { */
				p++;
				goto Break;
				/* } */
				/* break; */
#endif
			case '\0':
				/* 読み飛ばしのほうが、動作としては適切かも */
				ch = '*';
				break;
			default:
				break;
			}
		}
		*bufp++ = ch;
		p++;
	}
Teri_Break:
	/* 名前欄に','が入っている時にsplitをミスるので、見誤る可能性があるので、 */
	/* This->isTeri = true; */
Break:
	*bufp++ = '\0';
	This->rest -= bufp - *This->buffers;
	*++This->buffers = bufp;
	
	/* 区切り末の空白を削除 */
	if (*p == ' ')
		++p;
	return p;
}

void splitting_copy(char **s, char *bufp, const char *p, int size, int linenum)
{
	ressplitter res;
	ressplitter_init(&res, s, bufp, size);
	
	p = ressplitter_split(&res, p, false); /* name */
	p = ressplitter_split(&res, p, false); /* mail */
	p = ressplitter_split(&res, p, false); /* date */
	p = ressplitter_split(&res, p, linenum+1); /* text */
	p = ressplitter_split(&res, p, false); /* title */
}

#endif

#ifdef COOKIE
#define FORM_MAXLEN 1024
char form_name[FORM_MAXLEN];
char form_mail[FORM_MAXLEN];

/*
 *  Unescape - %xx を元に戻して、HTML形式に変換
 *  %00はいろいろ問題あるから今のところは%20(' ')に置換
 */

void Unescape(char *dst, const char *src, int dstlen, int srclen)
{
	while(dstlen > 1 && srclen > 0 && *src) {
		int c = 0;
		if(*src == '%') {
			int i;
			const char *tmp = ++src;
			srclen--;
			for(i = 0; i < 2 && srclen > 0; i++, src++, srclen--) {
				c *= 16;
				if(*src >= '0' && *src <= '9')
					c += *src - '0';
				else if(*src >= 'A' && *src <= 'F')
					c += *src - 'A' + 10;
				else if(*src >= 'a' && *src <= 'f')
					c += *src - 'a' + 10;
				else
					break;
			}
			if(c == 0) {
				if(tmp == src) {
					*(dst++) = '%';
					dstlen--;
					continue;
				} else {
					*(dst++) = ' ';
					dstlen--;
					continue;
				}
			}
		} else {
			c = *(src++);
			srclen--;
		}
#define SAFECPY(str, slen)        		\
		if(dstlen <= slen + 1)		\
			break;			\
		memcpy(dst, str, slen);		\
		dstlen -= slen;
		if(c == '\"') {
			SAFECPY("&quot;", 6);
		} else if(c == '&') {
			SAFECPY("&amp;", 5);
		} else if(c == '<') {
			SAFECPY("&lt;", 4);
		} else if(c == '>') {
			SAFECPY("&gt;", 4);
#undef SAFECPY
		} else {
			*dst++ = c;
			dstlen--;
		}
	}
	if(dstlen)
		*(dst++) = '\0';
}

/*
 *  SetFormName
 */

void SetFormName(void)
{
	/* HTTP_COOKIE= FROM=......;MAIL=......といった形式
	 (&で区切ればzz_GetStringが使えるんだけどね...) */
	char const *p = zz_http_cookie;
	strcpy(form_name, KARA);
	strcpy(form_mail, KARA);
	while(p && *p) {
		char *q = strchr(p, '='), *r;
		char *target;
		if(!q)
			break;
		if(q - p == 4 && !strncmp(p, "MAIL", 4)) {
			target = form_mail;
		} else if (q - p == 4 && !strncmp(p, "NAME", 4)) {
			target = form_name;
		} else {
			p = strchr(q, ';');
			if(p)
				p++;
			continue;
		}
		r = strchr(q, ';');
		if(!r) {
			r = q + strlen(q);
			p = NULL;
		} else
			p = r + 1;
		Unescape(target, q + 1, FORM_MAXLEN, r - q - 1);
	}
}
#endif

/****************************************************************/
/*	BadAccess						*/
/****************************************************************/
#ifdef NEWBA
int BadAccess(void)
{
	char *agent_kick[] = {
#ifdef	Katjusha_Beta_kisei
		"Kathusha",
#endif
		"WebFetch",
		"origin",
		"Nozilla",
		"WWWD",
	};
	int i;

	if (!*zz_http_user_agent && !*zz_http_language)
		return 1;

	for (i = 0; i < sizeof(agent_kick) / sizeof(char *); i++) {
		if (strstr(zz_http_user_agent, agent_kick[i]))
			return 1;
	}

	return 0;
}
#else
int BadAccess(void)
{
	if (strstr(zz_http_user_agent, "DreamPassport"))
		return 0;
	if (strstr(zz_http_user_agent, "DoCoMo"))
		return 0;
	if (strstr(zz_http_user_agent, "J-PHONE"))
		return 0;
	if (strstr(zz_http_user_agent, "ASTEL"))
		return 0;
	if (strstr(zz_http_user_agent, "[ja]"))
		return 0;
	if (strstr(zz_http_user_agent, "iCab"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "iBOX"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "WannaBe"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "Macintosh; I;"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "Mozilla/3.0N"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "CBBoard"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "PersonaWare"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "Sharp"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "95"))
		return 0;	/* win95        */
	if (strstr(zz_http_user_agent, "NT 4.0"))
		return 0;	/* winNT        */
	if (strstr(zz_http_user_agent, "WinNT"))
		return 0;	/* winNT        */

	if (!*zz_http_user_agent && !*zz_http_language)
		return 1;
/*	if(!*zz_http_user_agent)			return 1;*/
/*	if(!*zz_http_language)				return 1;*/
#ifdef	Katjusha_Beta_kisei
	if (strstr(zz_http_user_agent, "Katjusha"))
		return 1;
#endif
	if (strstr(zz_http_user_agent, "WebFetch"))
		return 1;
	if (strstr(zz_http_user_agent, "origin"))
		return 1;
	if (strstr(zz_http_user_agent, "Nozilla"))
		return 1;
	if (strstr(zz_http_user_agent, "WWWD"))
		return 1;

	return 0;
}
#endif
/****************************************************************/
/*	Log Out							*/
/****************************************************************/
int logOut(char *txt)
{
#ifdef LOGLOGOUT
	FILE *fp;
#endif

#ifdef DEBUG
	return 1;
#endif

	if (!BadAccess())
		return 1;

#ifdef	LOGLOGOUT
/*
if(strcmp(zz_bs,"ascii"))	return 1;
*/
	fp = fopen("./logout.txt", "a+");
	if (fp == NULL)
		return;
	fprintf(fp, "%02d:%02d:%02d %8s:", tm_now.tm_hour, tm_now.tm_min,
		tm_now.tm_sec, zz_bs);
	fprintf(fp, "(%15s)", zz_remote_addr);
	fprintf(fp, "%s***%s\n", zz_http_language, zz_http_user_agent);

/*
	fprintf(fp,"%s\n",zz_query_string);
	if(strstr(zz_http_user_agent,"compatible"))
		fprintf(fp,"%s\n",zz_http_language);
	else
		fprintf(fp,"%s***%s\n",zz_http_language,zz_http_user_agent);
	if(*zz_http_referer)	fprintf(fp,"%s\n",zz_http_referer);
	else			fprintf(fp,"%s***%s\n",zz_http_language,zz_http_user_agent);
	else			fprintf(fp,"%s\n",zz_http_user_agent);
*/

	fclose(fp);
#endif
	html_error(ERROR_LOGOUT);
	return 1;
}
/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
void html_bannerNew(void)
{
	pPrintf(pStdout, R2CH_HTML_NEW_BANNER);
}

/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
#ifdef	CM_BBSPINK
void html_banner(void)
{
	pPrintf(pStdout, R2CH_HTML_BBSPINK_BANNER);
}

#else
void html_banner(void)
{
	pPrintf(pStdout, R2CH_HTML_BANNER);
}

#endif

/****************************************************************/
/*	Get file size(out_html1)				*/
/****************************************************************/
static int out_html1(int level)
{
	char *s[20];
#ifndef TYPE_TERI
	char r4[SIZE_BUF];
#endif
	char p[SIZE_BUF];

	if (out_resN)
		return 0;
#ifndef CUTRESLINK
	strncpy(p, BigLine[0], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
	if (!*p)
		return 1; 
#endif
#ifdef	TYPE_TERI
	/*someReplace(s[4],r4,COMMA_SUBSTITUTE,",")       ; */
	html_head(level, s[4], lineMax);
#else
	someReplace(s[4], r4, COMMA_SUBSTITUTE, ",");
	html_head(level, r4, lineMax);
#endif
#if 0
	if (!is_imode()) {	/* no imode       */
		pPrintf(pStdout, "<DL>");
	}
#endif
	out_resN++;
	return 0;
}
/****************************************************************/
/*	Get file size(out_html)					*/
/****************************************************************/
static int out_html(int level, int line, int lineNo)
{
	char *s[20];
#ifdef	TYPE_TERI
	char *r0, *r1, *r3, *r4;
#else
	char r0[SIZE_BUF], r1[SIZE_BUF], r3[SIZE_BUF],
	    r4[SIZE_BUF];
#endif
	char p[SIZE_BUF];

/*printf("line=%d[%s]<P>\n",line,BigLine[line]);return 0;*/

	if (!out_resN) {	/* Can I write header ?   */
#ifndef CUTRESLINK
		strncpy(p, BigLine[0], 1024);
		p[1024 - 1] = '\0';
		if (!*p)
			return 1;
		res_split(s, p);
#else
		splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
		if (!*p)
			return 1; 
#endif
#ifdef	TYPE_TERI
		r4 = s[4];
#else
		someReplace(s[4], r4, COMMA_SUBSTITUTE, ",");
#endif
		html_head(level, r4, lineMax);
#if 0
		if (!is_imode()) {	/* no imode       */
			pPrintf(pStdout, "<DL>");
		}
#endif
	}
	out_resN++;

#ifndef CUTRESLINK
	strncpy(p, BigLine[line], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[line], sizeof(p) - 20, line);
	if (!*p)
		return 1; 
#endif
	
#ifdef	TYPE_TERI
	r0 = s[0];
	r1 = s[1];
	r3 = s[3];
#else
	someReplace(s[0], r0, COMMA_SUBSTITUTE, ",");
	someReplace(s[1], r1, COMMA_SUBSTITUTE, ",");
	someReplace(s[3], r3, COMMA_SUBSTITUTE, ",");
	someReplace(r3, r3, "&amp;", "&");
#endif
#ifndef	CUTRESLINK
	hlinkReplace(r3);
#endif

	if (!is_imode()) {	/* no imode */
#ifndef	CUTRESLINK
		if (*r3 && strlen(r3) < 8192) {
#else
		if (*r3 && s[4]-r3 < 8192) {
#endif
			if (r1 && strcmp(r1, "sage") == 0) {
#ifdef SAGE_IS_PLAIN
				pPrintf(pStdout,
					R2CH_HTML_RES_SAGE("%d", "%d", "%s", "%s", "%s"),
					lineNo, lineNo, r0, s[2], r3);
#else
				pPrintf(pStdout,
					R2CH_HTML_RES_MAIL("%d", "%d", "%s", "%s", "%s", "%s"),
					lineNo, lineNo, r1, r0, s[2], r3);
#endif
			} else if (*r1) {
				pPrintf(pStdout,
					R2CH_HTML_RES_MAIL("%d", "%d", "%s", "%s", "%s", "%s"),
					lineNo, lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_RES_NOMAIL("%d", "%d", "%s", "%s", "%s"),
					lineNo, lineNo, r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_RES_BROKEN_HERE,
				lineNo);
		}
		if (isbusytime && out_resN > RES_NORMAL) {
			if (path_depth)
				pPrintf(pStdout,
					R2CH_HTML_PATH_TAIL,
					lineNo,
					lineNo + RES_NORMAL,
					RES_NORMAL,
					RES_NORMAL,
					RES_NORMAL,
					LIMIT_PM - 12, LIMIT_AM);
			else
				pPrintf(pStdout,
					R2CH_HTML_TAIL,
					CGINAME, zz_bs, zz_ky,
					lineNo,
					lineNo + RES_NORMAL,
					RES_NORMAL,
					CGINAME, zz_bs, zz_ky,
					RES_NORMAL,
					RES_NORMAL,
					LIMIT_PM - 12, LIMIT_AM);
			return 1;
		}
	} else {		/* imode  */

		if (*r3) {
			if (*s[1]) {
				pPrintf(pStdout, R2CH_HTML_IMODE_RES_MAIL,
					lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_IMODE_RES_NOMAIL, lineNo,
					r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_IMODE_RES_BROKEN_HERE,
				lineNo);
		}
		if (out_resN > RES_IMODE) {
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL,
				CGINAME, zz_bs, zz_ky, lineNo,
				lineNo + RES_IMODE, RES_IMODE, CGINAME,
				zz_bs, zz_ky, RES_IMODE, RES_IMODE);
			return 1;
		}
	}

	return 0;
}
/****************************************************************/
/*	Output raw data file					*/
/****************************************************************/
#ifdef RAWOUT
#ifdef	CUTRESLINK
#if	0
/* BigLineをnul-terminatedではなく'\n'-terminatedにする場合 */
int getlinelen(const char *line)
{
	const char *last = BigBuffer + zz_fileSize;
	const char *end = memchr(line, '\n', last - line);
	if (end)
		return end + 1 - line;
	return last - line;
}
#endif
#endif

int dat_out_raw(void)
{
	int i;

	/* もし報告された最終レス番号およびサイズが一致していなけ
	   れば、最初の行にその旨を示す */
	if(raw_lastnum > 0
	   && !(raw_lastnum <= lineMax
		&& (BigLine[raw_lastnum - 1]
#ifndef	CUTRESLINK
		    + strlen(BigLine[raw_lastnum - 1]) + 1
#else
			+ (BigLine[raw_lastnum] - BigLine[raw_lastnum - 1])
			/* + getlinelen(BigLine[raw_lastnum - 1]) */
#endif
		    - BigBuffer) == raw_lastsize)) {
		pPrintf(pStdout, "-INCR\n");
		/* 全部を送信するように変更 */
		raw_lastnum = 0;
	} else {
		pPrintf(pStdout, "+OK\n");
	}
	/* raw_lastnum から全部を送信する */
	for(i = raw_lastnum; i < lineMax; i++) {
#ifndef CUTRESLINK
		pPrintf(pStdout, "%s\n", BigLine[i]);
#else
		pPrintf(pStdout, "%.*s", BigLine[i+1] - BigLine[i], BigLine[i]);
#endif
	}
	return 1;
}
#endif

/****************************************************************/
/*	Get file size(dat_out)					*/
/*	Level=0のときは、外側の出力				*/
/*	Level=1のときは、内側の出力				*/
/****************************************************************/
int dat_out(int level)
{
	int line, lineNo;
#ifdef RELOADLINK
	int lineLast = lineMax;
#endif
	int threadStopped=0;
	char *s[20];
	char p[SIZE_BUF];
	
	for (line = 0; line < lineMax; line++) {
		lineNo = line + 1;

		if (lineNo == 1) {
			if (is_nofirst())
				continue;
		} else {
			if (nn_st && lineNo < nn_st)
				continue;
			if (nn_to && lineNo > nn_to)
				continue;
			if (nn_ls && line < lineMax - nn_ls)
				continue;
		}

		if (out_html(level, line, lineNo))
			break;
#ifdef RELOADLINK
		lineLast = lineNo;
#endif
	}
	out_html1(level);
#ifdef RELOADLINK
	if (!level && lineMax == lineLast) {
		html_reload(lineLast);	/*  Button: Reload */
	}
#endif
#ifndef CUTRESLINK
	strncpy(p, BigLine[lineMax-1], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[lineMax-1], sizeof(p) - 20, lineMax-1);
	if (!*p)
		return 1; 
#endif
	if( s[2]!=0 && strstr( s[2], "ストッパー" )) threadStopped=1;
	html_foot(level, lineMax, threadStopped);

	return 0;
}
/****************************************************************/
/*	Get file size(dat_read)					*/
/*	BigBuffer, BigLine, LineMaxが更新されるはず		*/
/****************************************************************/
int dat_read(char const *fname,
	     int st,
	     int to,
	     int ls)
{
	int in;

	zz_fileSize = getFileSize(fname);

	if (zz_fileSize > MAX_FILESIZE)
		html_error(ERROR_TOO_HUGE);
	if (zz_fileSize < 10)
		html_error(ERROR_NOT_FOUND); /* エラー種別は別にした方がいいかも */
	if (*zz_ky == '.')
		html_error(ERROR_NOT_FOUND);

#if 1
	nn_st = st;
	nn_to = to;
	nn_ls = ls;
#else
	nn_st = atoi(zz_st);
	nn_to = atoi(zz_to);
	nn_ls = atoi(zz_ls);
#endif
	if (nn_st < 0)
		nn_st = 0;
	if (nn_to < 0)
		nn_to = 0;
	if (nn_st == 1 && nn_to == 1)
		strcpy(zz_nf, KARA);
	if (is_imode()) {	/* imode */
		if (!nn_st && !nn_to && !nn_ls)
			nn_ls = RES_IMODE;
	}
	if (!is_nofirst() && nn_ls > 0) {
		nn_ls--;
		if(nn_ls == 0) {
			nn_ls = 1;
			strcpy(zz_nf, "true");
		}
	} else if (nn_ls < 0)
		nn_ls = 0;

	in = open(fname, O_RDONLY);
	if (in < 0)
	{
		html_error(ERROR_NOT_FOUND);
		return 0;
	}
#ifdef USE_MMAP
	BigBuffer = mmap(NULL,
			 zz_mmap_size = zz_fileSize + 0x10000,
			 PROT_READ | PROT_WRITE,
			 MAP_PRIVATE,
			 zz_mmap_fd = in,
			 0);
	if (BigBuffer == MAP_FAILED)
		html_error(ERROR_NO_MEMORY);
#else
	BigBuffer = malloc(zz_fileSize + 32);
	if (!BigBuffer)
		html_error(ERROR_NO_MEMORY);

	read(in, BigBuffer, zz_fileSize);
	close(in);
#endif
#ifndef	CUTRESLINK
	/* XXX ところどころに 0 が現れるの? */
	{
		char *end = BigBuffer + zz_fileSize;
		char *p = BigBuffer;
		while ((p = memchr(p, '\0', end - p)) != NULL) {
			*p = '*';
		}
	}
#endif

	lineMax = getLineMax();
/*
html_error(ERROR_MAINTENANCE);
*/
	return 0;
}
/****************************************************************/
/*	Get line number						*/
/****************************************************************/
int getLineMax(void)
{
	int line = 0;
	char *p = BigBuffer;
	char *p1;

	if (!p)
		return -8;

#ifndef	CUTRESLINK
	while ((p1 = strchr(p, '\n')) != NULL) {
		BigLine[line] = p;
		*p1 = '\0';
		p = p1;
		if (line > RES_RED)
			break;
		line++;
		p++;
	}
#else
	p1 = BigBuffer + zz_fileSize;	/* p1 = 最後の\nの次のポインタ */
	while (p < p1 && *(p1-1) != '\n')	/* 最後の行末を探す 正常なdatなら問題無し */
		p1--;
	if (p1 - p < 10)	/* 適当だけど、問題は出ないはず */
		return -8;
	do {
		BigLine[line] = p;
		if (line > RES_RED)
			break;
		++line;
		p = memchr(p, '\n', p1-p) + 1;
	} while(p != p1);
	
	/*
		最後のレスの次に、ファイル末へのポインタを入れておく。
		これにより、レスの長さはポインタの差ですむ。
		(dat_out_rawでstrlenしている部分への対応)
	*/
	BigLine[line] = BigBuffer + zz_fileSize;
#endif
	return line;
}
/****************************************************************/
/*	Get file size						*/
/****************************************************************/
int getFileSize(char const *file)
{
	struct stat CountStat;
	int ccc = 0;
	if (!stat(file, &CountStat))
		ccc = (int) CountStat.st_size;
	return ccc;
}
/****************************************************************/
/*	Get file last-modified(getFileLastmod)			*/
/****************************************************************/
#ifdef LASTMOD
time_t getFileLastmod(char *file)
{
	struct stat CountStat;
	time_t ccc;
	if (!stat(file, &CountStat)) {
		ccc = CountStat.st_mtime;
		zz_fileSize = CountStat.st_size;
		return ccc;
	} else
		return -1;
}
/****************************************************************/
/*	Get file last-modified(get_lastmod_str)			*/
/****************************************************************/
int get_lastmod_str(char *buf, time_t lastmod)
{
	strftime(buf, 1024, "%a, %d %b %Y %H:%M:%S GMT",
		 gmtime(&lastmod));
	return (1);
}
#endif
/****************************************************************/
/*	PATH_INFOを解析						*/
/*	/board/							*/
/*	/board/							*/
/*	/board/datnnnnnn/[range] であるとみなす			*/
/*	return: pathが有効だったら1を返す			*/
/*	副作用: zz_bs, zz_ky, zz_st, zz_to, zz_nf		*/
/*		などが更新される場合がある			*/
/****************************************************************/
#ifdef USE_PATH
static int get_path_info(char const *path_info)
{
	char const *s = path_info;
	int n;

	path_depth = 0;
	/* PATH_INFO は、'/' で始まってるような気がしたり */
	if (s[0] != '/')
		return 0;

	path_depth++;

	/* 長すぎるPATH_INFOは怖いので受け付けない */
	if (strlen(s) >= 256)
		return 0;

	/* PATH_INFOを走査。ボード名などを抜き出す。
	   さまざまな理由により、'/'の有無もチェックしておく */
	n = strcspn(++s, "/");
	if (n == 0)
		return 0;
	/* 板名 */
	strncpy(zz_bs, s, n);
	s += n;
	if (s[0] != '/') {
		/* XXX ここで何かがしたい。hehehe */
		return 0;
	}
	path_depth++;
	/* つぎ… */
	n = strcspn(++s, "/");
	if (n == 0)
		return 0;
	strncpy(zz_ky, s, n);	/* パラメータかもしれないので取り込む */
	if (n == 0 || s[n] != '/')
		return 0;
	path_depth++;
	/* スレ */
	s += n + 1;

	/* strtok()で切り出したバッファは総長制限が
	   かかっているので、buffer overrunはないはず */
	if (s[0]) {
		char *p;
		/* 範囲指定のフォーマットは以下のものがある

		   /4	(st=4&to=4)
		   /4-	(st=4)
		   /-6	(to=6)
		   /4-6	(st=4to=4)

		   カキコ1は特別扱いで、nofirstにより
		   動作が左右されるっぽいので、どうしよう */

		/* 特に指定がなかったら、st=1であるとみなす */
		strcpy(zz_st, "1");

		/* st を取り出す */
		if (isdigit(*s)) {
			for (p = zz_st; isdigit(*s); p++, s++)
				*p = *s;
			*p = 0;
		}

		if (*s == '-') {
			s++;
			/* toを取り出す */
			if (isdigit(*s)) {
				for (p = zz_to; isdigit(*s); p++, s++)
					*p = *s;
				*p = 0;
			}
		} else {
			/* 範囲指定はないので、
			   単点ポイントと見なす */
			strcpy(zz_to, zz_st);
		}

		/* nofirstの仕様をごまかすためのkludge XXX */
		if (!zz_nf[0])
			strcpy(zz_nf,
			       (atoi(zz_st) == 1
				? "false"
				: "true"));
	}

	/* 処理は完了したものとみなす */
	return 1;
}
#endif

/****************************************************************/
/*	SETTING_R.TXTの読みこみ					*/
/****************************************************************/
#ifdef	USE_SETTING_FILE
void readSettingFile(const char *bbsname)
{
	char fname[1024];
	int fd;
	
	sprintf(fname, "../%.256s/%s", bbsname, SETTING_FILE_NAME);
	fd = open(fname, O_RDONLY);
	if (fd >= 0) {
		/* SETTING_R.TXTを読む */
		char const *cptr;
		char const *endp;
		struct stat st;
#ifdef	USE_MMAP
		void *mmptr;
		fstat(fd, &st);
		mmptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
#else
		/* まあ設定ファイルが8k以上逝かなければいいということで */
		char mmptr[8192];
		st.st_size = read(fd, mmptr, 8192);
#endif
		for (cptr = mmptr, endp = cptr + st.st_size - 1;
		     cptr < endp && *endp != '\n'; endp--)
			;
		for ( ; cptr && cptr < endp;
		      cptr = memchr(cptr, '\n', endp - cptr), cptr?cptr++:0) {
			if (*cptr != '\n' && *cptr != '#' && *cptr != ';') {
				int i;
				for (i = 0; i < sizeof(SettingParam)
					     / sizeof(SettingParam[0]); i++) {
					int len = strlen(SettingParam[i].str);
					if (cptr + len < endp 
					    && cptr[len] == '='
					    && strncmp(cptr,
						       SettingParam[i].str,
						       len) == 0) {
						*SettingParam[i].val
							= atoi(cptr + len + 1);
						break;
					}
				}
			}
		}
#ifdef	USE_MMAP
#ifdef  EXPLICIT_RELEASE
		munmap(mmptr, st.st_size);
		close(fd);
#endif
#else
		/* mmptr == automatic variable... */
		/* free(mmptr); */
		close(fd);
#endif  /* USE_MMAP */
	}
}
#endif	/*	USE_SETTING_FILE	*/

/****************************************************************/
/*	GET Env							*/
/****************************************************************/
void zz_GetEnv(void)
{
	currentTime = (long) time(&t_now);
	putenv("TZ=JST-9");
	tzset();
	tm_now = *localtime(&t_now);

	zz_remote_addr = getenv("REMOTE_ADDR");
	zz_remote_host = getenv("REMOTE_HOST");
	zz_http_referer = getenv("HTTP_REFERER");
	zz_http_cookie = getenv("HTTP_COOKIE");
#ifdef USE_PATH
	zz_path_info = getenv("PATH_INFO");
#endif
	zz_query_string = getenv("QUERY_STRING");
	zz_temp = getenv("REMOTE_USER");
	zz_http_user_agent = getenv("HTTP_USER_AGENT");
	zz_http_language = getenv("HTTP_ACCEPT_LANGUAGE");
#ifdef GZIP
	zz_http_encoding = getenv("HTTP_ACCEPT_ENCODING");
#endif
#ifdef CHECK_MOD_GZIP
	zz_server_software = getenv("SERVER_SOFTWARE");
#endif
#ifdef LASTMOD
	zz_http_if_modified_since = getenv("HTTP_IF_MODIFIED_SINCE");
#endif

	if (!zz_remote_addr)
		zz_remote_addr = KARA;
	if (!zz_remote_host)
		zz_remote_host = KARA;
	if (!zz_http_referer)
		zz_http_referer = KARA;
#ifdef USE_PATH
	if (!zz_path_info)
		zz_path_info = "";	/* XXX KARAを使い回すのは怖い */
#endif
	if (!zz_query_string)
		zz_query_string = KARA;
	if (!zz_temp)
		zz_temp = KARA;
	if (!zz_http_user_agent)
		zz_http_user_agent = KARA;
	if (!zz_http_language)
		zz_http_language = KARA;

	zz_GetString(zz_bs, sizeof(zz_bs), "bbs");
	zz_GetString(zz_ky, sizeof(zz_ky), "key");
	zz_GetString(zz_ls, sizeof(zz_ls), "ls");
	zz_GetString(zz_st, sizeof(zz_st), "st");
	zz_GetString(zz_to, sizeof(zz_to), "to");
	zz_GetString(zz_nf, sizeof(zz_nf), "nofirst");
	zz_GetString(zz_im, sizeof(zz_im), "imode");
#ifdef RAWOUT
	zz_GetString(zz_rw, sizeof(zz_rw), "raw");
#endif
#ifdef USE_PATH
	if (!get_path_info(zz_path_info)) {
		/* これ以降、path が付与されているかどうかの
		   判定は zz_path_info のテストで行ってくれ */
		zz_path_info = NULL;
	}
#endif
#ifdef COOKIE
	SetFormName();
#endif
#ifdef RAWOUT
	rawmode = (*zz_rw != '\0');
	if(rawmode) {
		char *p = strchr(zz_rw, '.');
		if(p) {
			/* raw=(last_article_no).(local_dat_size) */
			raw_lastnum = atoi(zz_rw);
			raw_lastsize = atoi(p + 1);
		}
		if(!p || raw_lastnum < 1 || raw_lastsize < 1) {
			raw_lastnum = raw_lastsize = 0;
		}
	}
#endif
#ifdef	USE_SETTING_FILE
	readSettingFile(zz_bs);
#endif
	isbusytime = IsBusy2ch();
}

/*----------------------------------------------------------------------
	終了処理
----------------------------------------------------------------------*/
void atexitfunc(void)
{
	/* html_error()での二重呼び出し防止 */
	static int isCalled = 0;
	if (isCalled) return;
	isCalled = 1;

#ifdef EXPLICIT_RELEASE
#ifdef USE_MMAP
	if (BigBuffer && BigBuffer != MAP_FAILED) {
		munmap(BigBuffer, zz_mmap_size);
		close(zz_mmap_fd);
	}
#else
	/* あちこちに散らばってたのでまとめてみた */
	if (BigBuffer)
		free(BigBuffer);
#endif
#endif /* EXPLICIT_RELEASE */
#ifdef ZLIB
	if (gzip_flag) {
		int outlen;
		char *outbuf;

		gzclose(pStdout);
		outlen = gz_getdata(&outbuf);	/* 圧縮データを受け取る */

		if ( outlen != 0 && outbuf == NULL ) {
			/* 圧縮中にmallocエラー発生 */
			pPrintf = (zz_printf_t) fprintf;
			pStdout = (gzFile) stdout;
			pPrintf(pStdout,"\n");
			html_error(ERROR_NO_MEMORY);
		}
		if ( gzip_flag == 2 ) {
			fprintf(stdout,"Content-Encoding: x-gzip\n");
		} else {
			fprintf(stdout,"Content-Encoding: gzip\n");
		}
		fprintf(stdout,"Content-Length: %d\n\n",outlen);

		fwrite(outbuf,1,outlen,stdout);
		fflush(stdout);
		/* free(outbuf); outbufはfreeすべき物 exitなのでほっとく */
	}
#elif defined(GZIP)
	if(gzip_flag) {
		fflush(stdout);
		close(1);		/* gzipを終了させるためcloseする */
		waitpid(pid, NULL, 0);
	}
#endif
}

/****************************************************************/
/*	MAIN							*/
/****************************************************************/
int main(void)
{
#ifdef GZIP
#ifndef ZLIB
	int pipefds[2];
	int whitespace = 2048;
#endif
#endif
	char fname[1024];

#if	('\xFF' != 0xFF)
#error	-funsigned-char required.
	/* このシステムでは、-funsigned-charを要求する */
	if ((char)0xFF != (unsigned char)0xFF) {
		puts("Content-Type: text/html\n"
		     "\n"
		     "-funsigned-char required.");
		return 0;
	}
#endif

#ifdef ZLIB
	pStdout = (gzFile) stdout;
#endif
	zz_GetEnv();

#ifdef RAWOUT
	if(!rawmode)
#endif
		pPrintf(pStdout, "Content-Type: text/html\n");
#ifdef RAWOUT
	else
		/* pPrintf(pStdout, "Content-Type: application/octet-stream\n"); */
		/* 現在の.datの MIME type に合わせる．テキストデータだし... */
		pPrintf(pStdout, "Content-Type: text/plain\n");
#endif
#ifdef LASTMOD
	sprintf(fname, "../%.256s/dat/%.256s.dat", zz_bs, zz_ky);
#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif
	/* スレ一覧を取りに逝くモード */
	if (1 <= path_depth && path_depth < 3) {
		sprintf(fname, "../%.256s/subject.txt", zz_bs);
	}

	zz_fileLastmod = getFileLastmod(fname);
	get_lastmod_str(lastmod_str, zz_fileLastmod);
#if 1
	get_lastmod_str(expires_str, zz_fileLastmod + 5);
#else
	{
	  /* ためしに廃棄期限をちょっと30秒先に設定してみる */
	  time_t nw;
	  time(&nw);
	  get_lastmod_str(expires_str, nw + 30);
	}
#endif
#ifdef PREVENTRELOAD
	if (zz_http_if_modified_since) {
		time_t modtime = ap_parseHTTPdate(zz_http_if_modified_since);
		/* 最後に変更してから FORCE_304_TIME 秒間は強制 304
		 * (zz_http_if_modified_since基準にすると永久に記事が取れ
		 *  ないかも)
		 */
		if(modtime != BAD_DATE
		   && (modtime + FORCE_304_TIME >= t_now
		       || modtime >= zz_fileLastmod))
#else
	if (zz_http_if_modified_since
	    && strstr(zz_http_if_modified_since, lastmod_str))
#endif
	{
		pPrintf(pStdout, "Status: 304\n");
		pPrintf(pStdout, "\n");
		return (0);
	}
#ifdef PREVENTRELOAD
	}
#endif
#endif

#ifdef GZIP
	if (zz_http_encoding && strstr(zz_http_encoding, "x-gzip")) {
		gzip_flag = 2;
	} else if (zz_http_encoding && strstr(zz_http_encoding, "gzip")) {
		gzip_flag = 1;
	} else {
		gzip_flag = 0;
	}
#ifndef ZLIB
	if ( gzip_flag == 2 ) {
		pPrintf(pStdout, "Content-Encoding: x-gzip\n");
	} else if ( gzip_flag == 1 ) {
		pPrintf(pStdout, "Content-Encoding: gzip\n");
	}
#endif
#endif

/*  Get Last-Modified Date */
#ifdef LASTMOD
	pPrintf(pStdout, "Last-Modified: %.256s\n", lastmod_str);
#ifdef EXPIRES
	/* Expiresを吐いてみる */
	pPrintf(pStdout, "Expires: %.256s\n", expires_str);
#endif
#endif

#ifdef ZLIB
	if ( gzip_flag == 0 ) pPrintf(pStdout, "\n");
#else
	pPrintf(pStdout, "\n");
#endif
#ifdef DEBUG
	sleep(1);
#endif
	fflush(stdout);

	/*  終了処理登録 */
	atexit(atexitfunc);
#ifdef GZIP
	if (gzip_flag) {
#ifndef ZLIB
		pipe(pipefds);
		if ((pid = fork()) == 0) {
			/* child  */
			dup2(pipefds[0], 0);
			close(pipefds[0]);
			close(pipefds[1]);
			execl("/bin/gzip", "gzip", "-cf9", NULL);
			pPrintf(pStdout, "Error can't exec\n");
			exit(1);
		}
		/* parent */
		dup2(pipefds[1], 1);
		close(pipefds[0]);
		close(pipefds[1]);
		fflush(stdout);
		while (whitespace--)
			putchar(' ');
		fflush(stdout);
#else
		fflush(stdout);

		/*  prepare zlib */
		/* 引数(0,"w")はzlib/gzio.cで特別扱い
		   仮にstdoutを設定し、closeしない
		  （2chバージョンの独自仕様 通常stdinに"w"しないから） */
		pStdout = gzdopen(0, "wb9");

		pPrintf = gzprintf;
		/* gzdopen()で"wb9"を指定したので不要 */
		/* gzsetparams(pStdout, Z_BEST_COMPRESSION,
			Z_DEFAULT_STRATEGY); */
#if 0
		/* put 2048byte */
		/* もう要らないんじゃないかな、
		   XXX 追試求む */
		while (whitespace--)
			gzputc(pStdout, ' ');
#endif
#endif /* ZLIB */
	}
#endif /* GZIP */

	logOut("");

	dat_read(fname,
		 atoi(zz_st),
		 atoi(zz_to),
		 atoi(zz_ls));

#ifdef RAWOUT
	if (rawmode)
		dat_out_raw();
#ifdef USE_PATH
	else if (path_depth == 2) {
		if (zz_ky[0] == '-')
			dat_out_subback();	/* スレ一覧 */
		else
			dat_out_index();	/* 板ダイジェスト */
	}
#endif
	else
		dat_out(0);
#else
	dat_out(0);
#endif
	return 0;
}
/****************************************************************/
/*	ERROR END(html_error)					*/
/****************************************************************/
void html_error(enum html_error_t errorcode)
{
	char zz_soko[256];
	char tmp[256];
	char doko[256];
	struct stat CountStat;
	char comcom[256];
	const char * mes;
	switch ( errorcode ) {
	case ERROR_TOO_HUGE:
		mes = ERRORMES_TOO_HUGE;
		break;
	case ERROR_NOT_FOUND:
		mes = ERRORMES_NOT_FOUND;
		break;
	case ERROR_NO_MEMORY:
		mes = ERRORMES_NO_MEMORY;
		break;
	case ERROR_MAINTENANCE:
		mes = ERRORMES_MAINTENANCE;
		break;
	case ERROR_LOGOUT:
		mes = ERRORMES_LOGOUT;
		break;
	default:
		mes = "";
	}
#ifdef RAWOUT
	if(rawmode) {
		/* ?....はエラー。 */
		pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
	}
#endif
	
	*tmp = '\0';
	strcpy(tmp, LastChar(zz_ky, '/'));
	strncpy(zz_soko, tmp, 3);

	*comcom = '\0';
	if (errorcode == ERROR_LOGOUT) {
		sprintf(comcom, R2CH_HTML_ERROR_ADMIN);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_1, mes, mes, mes, comcom);

	if (strstr(zz_http_user_agent, "Katjusha")) {
		pPrintf(pStdout, R2CH_HTML_ERROR_2);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_3);
	html_bannerNew();
	pPrintf(pStdout, R2CH_HTML_ERROR_4);

	if (errorcode == ERROR_NOT_FOUND) {
		sprintf(doko, "../%.50s/kako/%.50s/%.50s.html", zz_bs,
			zz_soko, tmp);
		if (!stat(doko, &CountStat)) {
			pPrintf(pStdout, R2CH_HTML_ERROR_5_HTML, doko + 2,
				tmp);
		} else {
			sprintf(doko, "../%.50s/kako/%.50s/%.50s.dat",
				zz_bs, zz_soko, tmp);
			if (!stat(doko, &CountStat)) {
				pPrintf(pStdout, R2CH_HTML_ERROR_5_DAT,
					doko + 2, tmp);
			} else {
				pPrintf(pStdout, R2CH_HTML_ERROR_5_NONE,
					zz_bs);
			}
		}
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_6);

	exit(0);
}
/****************************************************************/
/*	ERROR END(html_error999)				*/
/****************************************************************/
int html_error999(char *mes)
{
	char zz_soko[256];	/* 倉庫番号(3桁数字) */
	char tmp[256];		/* ?スレ番号(9桁数字) */
	char tmp_time[16];	/* 時刻 "hh:mm:ss" */
	*tmp = '\0';
#ifdef RAWOUT
	if(rawmode) {
		/* ?....はエラー。 */
		pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
	}
#endif
	strcpy(tmp, LastChar(zz_ky, '/'));
	strncpy(zz_soko, tmp, 3);
	sprintf(tmp_time, "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min,
		tm_now.tm_sec);

	pPrintf(pStdout, R2CH_HTML_ERROR_999_1,
		mes, zz_bs, zz_ky, zz_ls, zz_st, zz_to, zz_nf, zz_fileSize,
		lineMax, tmp_time, zz_bs, zz_soko, tmp, tmp);
	html_banner();
	pPrintf(pStdout, R2CH_HTML_ERROR_999_2);

	exit(0);
}
/****************************************************************/
/*								*/
/****************************************************************/
#define GETSTRING_LINE_DELIM '&'
#define GETSTRING_VALUE_DELIM '='
#define MAX_QUERY_STRING 200
char *zz_GetString(char *dst, size_t dat_size, char const *tgt)
{
	int	i;

	char const *line = zz_query_string;
	char const *delim_ptr;
	char const *value_ptr;
#ifndef GSTR2
	int tgt_len = strlen(tgt);
#endif
	int line_len;
	int value_len;
	int value_start;

	for(i=0;i<MAX_QUERY_STRING;i++)
	{
		/* 行末を見つける */
		delim_ptr = strchr( line, GETSTRING_LINE_DELIM );
		if ( delim_ptr )
			line_len = delim_ptr - line;
		else
			line_len = strlen(line);

		/* '='を見つける */
		value_ptr = memchr( line, GETSTRING_VALUE_DELIM, line_len );
		if ( value_ptr ) {
			value_start = value_ptr + 1 - line;
#ifdef GSTR2
			/* 最初の一文字の一致 */
			if ( *line == *tgt )
#else
			/* 完全一致 */
			if ( tgt_len == (value_start-1) && !memcmp(line, tgt, tgt_len) )
#endif
			{
				/* 値部分の開始位置と長さ */
				value_len = line_len - value_start;

				/* 長さを丸める */
				if ( value_len >= dat_size )
					value_len = dat_size - 1;

				/* 値をコピー */
				memcpy( dst, line + value_start, value_len );
				dst[value_len] = '\0';

				return dst;
			}
		}

		if ( !delim_ptr )
			break;

		line += line_len + 1; /* skip delim */
	}
	*dst = '\0';
	return	dst;
}
/****************************************************************/
/*								*/
/****************************************************************/
int whatKanji(char *str)
{
	int val = 0;
	unsigned char b1, b2, b3;

	b1 = *str++;
	b2 = *str++;
	b3 = *str;
	if (b1 == 0x1b) {
		if (b2 == '$' && b3 == 'B')
			return 16;
		if (b2 == '$' && b3 == '@')
			return 32;
		if (b2 == '(' && b3 == 'J')
			return 64;
		if (b2 == '(' && b3 == 'B')
			return 128;
		return 0;
	}
	if (b1 >= 0xa0 && b1 <= 0xdf)
		val |= 1;
	if (((b1 >= 0x81 && b1 <= 0x9f) || (b1 >= 0xe0 && b1 <= 0xfc))
	    && (b2 >= 0x40 && b2 <= 0xfc && b2 != 0x7f))
		val |= 2;
	if (b1 == 0x8e && (b2 >= 0xa0 && b2 <= 0xdf))
		val |= 4;
	if ((b1 >= 0xa1 && b1 <= 0xfe) && (b2 >= 0xa1 && b1 <= 0xfe))
		val |= 8;

	return val;
}
/****************************************************************/
/*								*/
/****************************************************************/
#ifdef	TYPE_TERI
char *findSplitter(char *stt, int sp)
{
	char *p = stt;
	char ss = (unsigned char) (sp & 0x00ff);

	while (*p) {
/*
		if( *p >= 0xa0 && *p <= 0xdf)	{p++	;continue;}
		if((*p >= 0x81 && *p <= 0x9f || *p >= 0xe0 && *p <= 0xfc))	{p += 2	;continue;}
*/
		if (*p == ss && *(p + 1) == '>')
			return p;
		p++;
	}
	return NULL;
}
#else
char *findSplitter(char *stt, int sp)
{
	char *p = stt;
	char ss = (unsigned char) (sp & 0x00ff);

	while (*p) {
		if (*p >= 0xa0 && *p <= 0xdf) {
			p++;
			continue;
		}
		if (((*p >= 0x81 && *p <= 0x9f)
		     || (*p >= 0xe0 && *p <= 0xfc))) {
			p += 2;
			continue;
		}
/*		if((*p >= 0x40 && *p <= 0xfc && *p != 0x7f))	p += 2	;*/
		if (*p == ss)
			return p;
		p++;
	}
	return NULL;
}
#endif
/****************************************************************/
/*								*/
/****************************************************************/
int res_split(char **s, char *p)
{
	int i;
	char *p0;

	for (i = 0; i < 5; i++)
		s[i] = KARA;

	s[0] = p;
	for (i = 1; i < 5; i++) {
#ifdef	TYPE_TERI
		p0 = findSplitter(s[i - 1], '<');
		if (!p0) {
			return 0;
		}
		*p0 = '\0';
		p0++;
		*p0 = '\0';
		p0++;
		s[i] = p0;
#else
		p0 = findSplitter(s[i - 1], ',');
		if (!p0) {
			return 0;
		}
		*p0 = '\0';
		p0++;
		s[i] = p0;
#endif
	}
	return 1;
}
/****************************************************************/
/*								*/
/****************************************************************/
void dump_out16(char const *d)
{
	char const *p = d;
	pPrintf(pStdout, "\n\n<!-- ");
	while (*p) {
		pPrintf(pStdout, ",%02x", *p);
		p++;
	}
	pPrintf(pStdout, " -->\n\n");
}
/****************************************************************/
/*								*/
/****************************************************************/
int IsBusy2ch(void)
{
	if (tm_now.tm_hour < LIMIT_AM || LIMIT_PM <= tm_now.tm_hour)
		return 1;
	return 0;
}
/****************************************************************/
/*	えらー							*/
/****************************************************************/
 /* src中の一番末尾のデリミタ文字列 c の次の文字位置を得る
  */
char const *LastChar(char const *src, char c)
{
	char const *p;
	p = strrchr(src, c);

	if (p == NULL)
		return src;
	return (p + 1);
}
/****************************************************************/
/*	HTML HEADER						*/
/****************************************************************/
void html_head(int level, char const *title, int line)
{
#ifdef CHUNK_ANCHOR
	int i;
#endif
#ifdef USE_INDEX2CGI
	char fname[1024];
#endif

	if (level) {
		pPrintf(pStdout,
			R2CH_HTML_DIGEST_HEADER_2("%s"),
			title);
		/* これだけ出力してもどる */
		return;
	}

	if (!is_imode()) {	/* no imode       */
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_HEADER_1("%s", "../"),
				title);
		else {
#ifdef USE_INDEX2CGI
			sprintf(fname, "../%.256s/index2.cgi", zz_bs);
			if (access(fname, S_IXUSR) == -1)
#endif
#ifdef CHECK_MOD_GZIP
				if ( strstr(zz_server_software,"mod_gzip/") != NULL ) {
					pPrintf(pStdout,
						R2CH_HTML_HEADER_1("%s", "/%s/"),
						title, zz_bs);
				} else
#endif
#ifdef GZIP
				pPrintf(pStdout,
					R2CH_HTML_HEADER_1("%s", "/%s/index.htm%s"),
					title, zz_bs, gzip_flag ? "" : "l");
#else
				pPrintf(pStdout,
					R2CH_HTML_HEADER_1("%s", "/%s/index.htm"),
					title, zz_bs);
#endif
#ifdef USE_INDEX2CGI
			else
				pPrintf(pStdout,
					R2CH_HTML_HEADER_1("%s", "/%s/index2.cgi"),
					title, zz_bs);
#endif
		}
#ifdef ALL_ANCHOR
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_ALL_ANCHOR); 
		else
			pPrintf(pStdout,
				R2CH_HTML_ALL_ANCHOR,
				zz_bs, zz_ky); 
#endif
#ifdef CHUNK_ANCHOR
		for (i = 1; i <= line; i += CHUNK_NUM) {
			if (path_depth)
				pPrintf(pStdout,
					R2CH_HTML_PATH_CHUNK_ANCHOR,
					i,
					i + CHUNK_NUM - 1, 
					i);
			else
				pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR,
					zz_bs, zz_ky,
					i,
					i + CHUNK_NUM - 1, 
					(i == 1 ? "" : "&n=t"),
					i);
		}
#endif /* CHUNK_ANCHOR */
#ifdef LATEST_ANCHOR
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_LATEST_ANCHOR,
				LATEST_NUM, LATEST_NUM);
		else
			pPrintf(pStdout,
				R2CH_HTML_LATEST_ANCHOR,
				zz_bs, zz_ky,
				LATEST_NUM, LATEST_NUM);
#endif
	} else {
		pPrintf(pStdout, R2CH_HTML_IMODE_HEADER_1,
			title, zz_bs, zz_bs, zz_ky, RES_IMODE, zz_bs,
			zz_ky, RES_IMODE, RES_IMODE);
	}

	if (line > RES_RED) {
		pPrintf(pStdout, R2CH_HTML_HEADER_RED, RES_RED);
	} else if (line > RES_REDZONE) {
		pPrintf(pStdout, R2CH_HTML_HEADER_REDZONE, RES_REDZONE,
			RES_RED);
	} else if (line > RES_YELLOW) {
		pPrintf(pStdout, R2CH_HTML_HEADER_YELLOW, RES_YELLOW,
			RES_RED);
	}

	if (is_imode())
		pPrintf(pStdout, R2CH_HTML_HEADER_2_I, title);
	else
		pPrintf(pStdout, R2CH_HTML_HEADER_2, title);
}

/****************************************************************/
/*	RELOAD						        */
/****************************************************************/
#ifdef RELOADLINK
void html_reload(int startline)
{
	if (is_imode())	/*  imode */
		pPrintf(pStdout, R2CH_HTML_RELOAD_I, zz_bs, zz_ky,
			startline);
	else {
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_RELOAD,
				startline);
		else
			pPrintf(pStdout,
				R2CH_HTML_RELOAD,
				zz_bs, zz_ky,
				startline);
	}
}
#endif
/****************************************************************/
/*	HTML FOOTER						*/
/****************************************************************/
static void html_foot(int level, int line, int stopped)
{
	out_resN = 0;	/* ここで初期化するといいらしい? */

	if (is_imode())
		return html_foot_im(line,stopped);
	if (line <= RES_RED && !stopped) {
#ifndef COOKIE
		if (path_depth == 3)
			pPrintf(pStdout,
				R2CH_HTML_FORM("../../../", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
		else if (path_depth == 2)
			pPrintf(pStdout,
				R2CH_HTML_FORM("../../", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
		else
			pPrintf(pStdout,
				R2CH_HTML_FORM("", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
#else
		pPrintf(pStdout, R2CH_HTML_FORM, form_name,
			form_mail, zz_bs, zz_ky,
			currentTime);
#endif
		}

	if (level)
		pPrintf(pStdout, R2CH_HTML_DIGEST_FOOTER);
	else
		pPrintf(pStdout, R2CH_HTML_FOOTER);
}
/****************************************************************/
/*	HTML FOOTER(i-MODE)					*/
/****************************************************************/
void html_foot_im(int line, int stopped)
{
	if (line <= RES_RED && !stopped ) {
		pPrintf(pStdout, R2CH_HTML_FORM_IMODE, zz_bs, zz_ky, currentTime);
	}
	pPrintf(pStdout, R2CH_HTML_FOOTER_IMODE);
}

/****************************************************************/
/*	Replace(do)						*/
/****************************************************************/
char *doReplace(char *des, char const *str0, char const *str1)
{
	char *p; 

	int str0_length; 
	int str1_length; 

	/* 置き換えるべき文字列の位置を取得 */ 
	p = strstr(des, str0); 
	if (p == NULL) { 
		return NULL; 
	} 

	str0_length = strlen(str0); 
	str1_length = strlen(str1); 

	/* 後ろの部分を目的の位置まで移動 */
	memmove( p + str1_length, p + str0_length, strlen(p + str0_length)+1 );

	/* str1をはめ込む */
	memcpy( p, str1, str1_length );

	/* 部分文字列以後の文字列の位置を返す */ 
	return p + str1_length; 
}
/****************************************************************/
/*	Replace(some)						*/
/****************************************************************/
void someReplace(char const *src,
		 char *des,
		 char const *str0, char const *str1)
{
	char *last;
	strcpy(des, src);

	last = des;
	while (last) {
		last = doReplace(last, str0, str1);
	}
}
/****************************************************************/
/*	Replace(hrefStop)					*/
/****************************************************************/
#ifndef CUTRESLINK
int hrefStop(char x)
{
	if ('A' <= x && x <= 'Z')
		return 0;
	if ('a' <= x && x <= 'z')
		return 0;
	if ('0' <= x && x <= '9')
		return 0;
	if (x == '#')
		return 0;
	if (x == '/')
		return 0;
	if (x == '~')
		return 0;
	if (x == '_')
		return 0;
	if (x == '.')
		return 0;
	if (x == ',')
		return 0;
	if (x == '$')
		return 0;
	if (x == '%')
		return 0;
	if (x == '&')
		return 0;
	if (x == '@')
		return 0;
	if (x == '?')
		return 0;
	if (x == '=')
		return 0;
	if (x == '-')
		return 0;
	if (x == '+')
		return 0;
	if (x == '*')
		return 0;
	if (x == ';')
		return 0;
	if (x == ':')
		return 0;
	if (x == '!')
		return 0;
	if (x == '^')
		return 0;
	if (x == '`')
		return 0;
	if (x == '|')
		return 0;
	if (x == '[')
		return 0;
	if (x == ']')
		return 0;
	if (x == '{')
		return 0;
	if (x == '}')
		return 0;
	if (x == '\\')
		return 0;
	return 1;
}
#endif
/****************************************************************/
/*	Replace(cutWordOff)					*/
/****************************************************************/
int cutWordOff(char *src, char *word)
{
	char *p = strstr(src, word);
	if (!p)
		return 0;
	*p = '\0';
	return 1;
}
/****************************************************************/
/*	Replace(ExistHlinkX)					*/
/****************************************************************/
int ExistHlinkX(char *tgt, char *src, char *url, char *hrefStr)
{
	char *p;
	char *u;
	p = strstr(src, tgt);
	if (!p)
		return 0;
	/*NASHI*/ for (u = url; *p; p++) {
		if (hrefStop(*p))
			break;
		*u = *p;
		u++;
	}
	*u = '\0';
	/*-- &quot; 等の削除 --*/
	cutWordOff(url, "&quot;");
/*	cutWordOff(url,";")		;*/
	/*---------------------*/
	sprintf(hrefStr,
		"<A HREF=\"%.1024s\" TARGET=\"_blank\">%.1024s</A>", url,
		url);
	return 1;
/*ARI*/}
/****************************************************************/
/*	Replace(ExistHlink)					*/
/****************************************************************/
int ExistHlink(char *src, char *h0, char *h1)
{
	if (ExistHlinkX("http://", src, h0, h1))
		return 1;
	if (ExistHlinkX("ftp://", src, h0, h1))
		return 1;
	return 0;
}
/****************************************************************/
/*	Replace(hlinkReplace)					*/
/****************************************************************/
void hlinkReplace(char *src)
{
	char *last = src;
	char hlink0[SIZE_BUF];
	char hlink1[SIZE_BUF];

	while (last) {
		if (!ExistHlink(last, hlink0, hlink1))
			break;
		last = doReplace(last, hlink0, hlink1);
	}
}


/****************************************************************/
/*	END OF THIS FILE					*/
/****************************************************************/
