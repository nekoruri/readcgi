#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<time.h>
#include	<unistd.h>
#ifdef HAVE_READ2CH_H
# include	"read2ch.h"
#endif

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

char const *zz_remote_addr;
char const *zz_remote_host;
char const *zz_http_referer;
char const *zz_http_cookie;
#ifdef USE_PATH
char const *zz_path_info;
#endif
char const *zz_query_string;
char *zz_temp;
char const *zz_http_user_agent;
char const *zz_http_language;
#ifdef GZIP
char const *zz_http_encoding;
int gzip_flag;
#endif

#ifdef LASTMOD
char const *zz_http_if_modified_since;
time_t zz_fileLastmod;
char lastmod_str[1024];
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
char *BigLine[RES_RED + 16];

#define is_imode() (*zz_im == 't')

char *KARA = "";
int zz_fileSize = 0;
int lineMax = -1;
int out_resN = 0;

time_t t_now;
struct tm tm_now;
long currentTime;
int isbusytime = 0;

char const *LastChar(char const *src, char c);
char *zz_GetString(char *dst, char const *tgt);
char *doReplace(char *des, char const *str0, char const *str1);
void html_error(char *mes);
void html_foot_im(int);
void html_head(char *title, int line);
int res_split(char **s, char *p);
void someReplace(char const *src, char *des, char const *str0, char const *str1);
void hlinkReplace(char *src);
void html_foot(int line);
int getLineMax(void);
int IsBusy2ch(void);
int getFileSize(char *file);
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
typedef int (*zz_printf_t) (gzFile, const char *, ...);
static gzFile pStdout; /*  = (gzFile) stdout; */
static zz_printf_t pPrintf = (zz_printf_t) fprintf;
#else
#define pPrintf fprintf
#define pStdout stdout
int pid;
#endif

#ifdef CUTRESLINK
/* <ctype.h>等とかぶってたら、要置換 */
#define false (0)
#define true (!false)
#define _C_ (1<<0) /* datチェック用区切り文字等 */
#define _U_ (1<<1) /* URLに使う文字 */
#define _S_ (1<<2) /* SJIS1バイト目＝<br>タグ直前の空白が削除可かを適当に判定 */
#define LINKTAGCUT (1)

/* #define isCheck(c) (flagtable[(unsigned char)(c)] & _C_) */
#define isCheck(c) (flagtable[/*(unsigned char)*/(c)] & _C_)
#define isSJIS1(c) (flagtable[(unsigned char)(c)] & _S_)
#define hrefStop(c) (!(flagtable[(unsigned char)(c)] & _U_))

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


char flagtable[256] = {
	_0____,______,______,______,______,______,______,______, /*  00-07 */
	______,______,______,______,______,______,______,______, /*  08-0F */
	______,______,______,______,______,______,______,______, /*  10-17 */
	______,______,______,______,______,______,______,______, /*  18-1F */
	_0____,__1___,______,__1___,__1___,__1___,_01___,______, /*  20-27 !"#$%&' */
	______,______,__1___,__1___,_01___,__1___,__1___,__1___, /*  28-2F ()*+,-./ */
	__1___,__1___,__1___,__1___,__1___,__1___,__1___,__1___, /*  30-37 01234567 */
	__1___,__1___,__1___,__1___,_0____,__1___,______,__1___, /*  38-3F 89:;<=>? */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  40-47 @ABCDEFG */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  48-4F HIJKLMNO */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  50-57 PQRSTUVW */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  58-5F XYZ[\]^_ */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  60-67 `abcdefg */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  68-6F hijklmno */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  70-77 pqrstuvw */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,______, /*  78-7F xyz{|} */
	____3_,_0_23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  80-87 */
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

/*
findSplitterの代わり
レスを全走査するが、コピーと変換(と削除)を同時に行う
p コピー前のレス(BigBuffer内の１レス)
istagcut <a href=...>と</a>をcutするか
Return 次のpの先頭
non-TYPE_TERIなdatには,"<>"は含まれないはずなので、#ifdef TYPE_TERI は略
*/
const char *ressplitter_split(ressplitter *This, const char *p, int istagcut)
{
	char *bufp = *This->buffers;
	int bufrest = This->rest;
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
				if (istagcut) {
					/* if (*(p+1) != 'b' || *(p+2) != 'r') { */
					if ((*(p+1) == 'a' && *(p+2) == ' ')
					    || (*(p+1) == '/' && *(p+2) == 'a')) {
						while (*p != '>') { /* strchr(p, '>') */
							if (*p == '\0')
								goto Break;
							++p;
						}
						++p;
						continue;
					}
				}
				break;
			case '&':
				if (memcmp(p, "&amp", 4) == 0) {
					if (*(p + 4) != ';')
						p += 4 - 1;
				}
				break;
#ifndef TYPE_TERI
			case 0x81: /*  *"＠" */
				/* if (!This->isTeri) { */
				if (memcmp(p, "＠｀", 4) == 0) {
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
				goto Break;
				/* break; */
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

void splitting_copy(char **s, char *bufp, const char *p, int size)
{
	ressplitter res;
	ressplitter_init(&res, s, bufp, size);
	
	p = ressplitter_split(&res, p, false); /* name */
	p = ressplitter_split(&res, p, false); /* mail */
	p = ressplitter_split(&res, p, false); /* date */
	p = ressplitter_split(&res, p, (LINKTAGCUT && isbusytime)
			      || rawmode); /* text */
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
int BadAccess()
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
int BadAccess()
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
	html_error("なんか不調です。");
	return 1;
}
/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
void html_bannerNew()
{
	pPrintf(pStdout, R2CH_HTML_NEW_BANNER);
}

/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
#ifdef	CM_BBSPINK
void html_banner()
{
	pPrintf(pStdout, R2CH_HTML_BBSPINK_BANNER);
}

#else
void html_banner()
{
	pPrintf(pStdout, R2CH_HTML_BANNER);
}

#endif

/****************************************************************/
/*	Get file size(out_html1)				*/
/****************************************************************/
int out_html1()
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
	splitting_copy(s, p, BigLine[0], sizeof(p) - 20);
	if (!*p)
		return 1; 
#endif
#ifdef	TYPE_TERI
	/*someReplace(s[4],r4,"＠｀",",")       ; */
	html_head(s[4], lineMax);
#else
	someReplace(s[4], r4, "＠｀", ",");
	html_head(r4, lineMax);
#endif
	if (!is_imode()) {	/* no imode       */
		pPrintf(pStdout, "<DL>");
	}
	out_resN++;
	return 0;
}
/****************************************************************/
/*	Get file size(out_html)					*/
/****************************************************************/
int out_html(int line, int lineNo)
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
		splitting_copy(s, p, BigLine[0], sizeof(p) - 20);
		if (!*p)
			return 1; 
#endif
#ifdef	TYPE_TERI
		r4 = s[4];
#else
		someReplace(s[4], r4, "＠｀", ",");
#endif
		html_head(r4, lineMax);
		if (!is_imode()) {	/* no imode       */
			pPrintf(pStdout, "<DL>");
		}
	}
	out_resN++;

#ifndef CUTRESLINK
	strncpy(p, BigLine[line], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[line], sizeof(p) - 20);
	if (!*p)
		return 1; 
#endif
	
#ifdef	TYPE_TERI
	r0 = s[0];
	r1 = s[1];
	r3 = s[3];
#else
	someReplace(s[0], r0, "＠｀", ",");
	someReplace(s[1], r1, "＠｀", ",");
	someReplace(s[3], r3, "＠｀", ",");
	someReplace(r3, r3, "&amp;", "&");
#endif
	hlinkReplace(r3);

	if (!is_imode()) {	/* no imode */
		if (*r3 && strlen(r3) < 8192) {
			if (r1 && strcmp(r1, "sage") == 0) {
#ifdef SAGE_IS_PLAIN
				pPrintf(pStdout, R2CH_HTML_RES_SAGE,
					lineNo, r0, s[2], r3);
#else
				pPrintf(pStdout, R2CH_HTML_RES_MAIL,
					lineNo, r1, r0, s[2], r3);
#endif
			} else if (*r1) {
				pPrintf(pStdout, R2CH_HTML_RES_MAIL,
					lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout, R2CH_HTML_RES_NOMAIL,
					lineNo, r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_RES_BROKEN_HERE,
				lineNo);
		}
		if (isbusytime && out_resN > RES_NORMAL) {
			pPrintf(pStdout, R2CH_HTML_TAIL,
				CGINAME, zz_bs, zz_ky, lineNo,
				lineNo + RES_NORMAL, RES_NORMAL, CGINAME,
				zz_bs, zz_ky, RES_NORMAL, RES_NORMAL,
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
int dat_out_raw()
{
	int i;

	/* もし報告された最終レス番号およびサイズが一致していなけ
	   れば、最初の行にその旨を示す */
	if(raw_lastnum > 0
	   && !(raw_lastnum <= lineMax
		&& (BigLine[raw_lastnum - 1]
		    + strlen(BigLine[raw_lastnum - 1]) + 1
		    - BigBuffer) == raw_lastsize)) {
		pPrintf(pStdout, "-INCR\n");
		/* 全部を送信するように変更 */
		raw_lastnum = 0;
	} else {
		pPrintf(pStdout, "+OK\n");
	}
	/* raw_lastnum から全部を送信する */
	for(i = raw_lastnum; i < lineMax; i++) {
		pPrintf(pStdout, "%s\n", BigLine[i]);
	}
	return 1;
}
#endif

/****************************************************************/
/*	Get file size(dat_out)					*/
/****************************************************************/
int dat_out()
{
	int line, lineNo;
#ifdef RELOADLINK
	int lineLast = lineMax;
#endif
	
	for (line = 0; line < lineMax; line++) {
		lineNo = line + 1;

		if (lineNo == 1) {
			if (!strncmp(zz_nf, "t", 1))
				continue;
		} else {
			if (nn_st && lineNo < nn_st)
				continue;
			if (nn_to && lineNo > nn_to)
				continue;
			if (nn_ls && line < lineMax - nn_ls)
				continue;
		}

		if (out_html(line, lineNo))
			break;
#ifdef RELOADLINK
		lineLast = lineNo;
#endif
	}
	out_html1();
#ifdef RELOADLINK
	if (lineMax == lineLast) {
		html_reload(lineLast);	/*  Button: Reload */
	}
#endif
	html_foot(lineMax);

	return 0;
}
/****************************************************************/
/*	Get file size(dat_read)					*/
/****************************************************************/
int dat_read()
{
	int i;
	int in;
	char fname[1024];

	sprintf(fname, "../%.256s/dat/%.256s.dat", zz_bs, zz_ky);
	/* sprintf(fname,"../%s/dat/%s.dat",zz_bs,zz_ky) ; */
#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif
	zz_fileSize = getFileSize(fname);

	if (zz_fileSize > MAX_FILESIZE)
		html_error("このスレッド大きすぎます。");
	if (*zz_ky == '.')
		html_error("そんな板orスレッドないです。");

	nn_st = atoi(zz_st);
	nn_to = atoi(zz_to);
	nn_ls = atoi(zz_ls);
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
	if (strncmp(zz_nf, "t", 1))
		nn_ls--;
	if (nn_ls < 0)
		nn_ls = 0;

	BigBuffer = malloc(zz_fileSize + 32);
	if (!BigBuffer)
		html_error("メモリの確保に失敗しました。");

	in = open(fname, O_RDONLY);
	if (in < 0)
	{
		html_error("そんな板orスレッドないです。");
		return 0;
	}
	read(in, BigBuffer, zz_fileSize);
	close(in);
	i = strlen(BigBuffer);
	while(i < zz_fileSize) {
		BigBuffer[i] = '*';
		i += strlen(BigBuffer + i);
	}

	lineMax = getLineMax();
/*
html_error("調整中。。。");
*/
	return 0;
}
/****************************************************************/
/*	Get line number						*/
/****************************************************************/
int getLineMax()
{
	int line = 0;
	char *p = BigBuffer;
	char *p1;

	if (!p)
		return -8;

	while ((p1 = strchr(p, '\n')) != NULL) {
		BigLine[line] = p;
		*p1 = '\0';
		p = p1;
		if (line > RES_RED)
			break;
		line++;
		p++;
	}
	return line;
}
/****************************************************************/
/*	Get file size						*/
/****************************************************************/
int getFileSize(char *file)
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
		return ccc;
	} else
		return -1;
}
/****************************************************************/
/*	Get file last-modified(get_lastmod_str)			*/
/****************************************************************/
int get_lastmod_str(time_t lastmod)
{
	strftime(lastmod_str, 1024, "%a, %d %b %Y %H:%M:%S GMT",
		 gmtime(&lastmod));
	return (1);
}
#endif
/****************************************************************/
/*	PATH_INFOを解析						*/
/*	/board/datnnnnnn/[range] であるとみなす			*/
/*	return:pathが有効だったら1を返す			*/
/*	副作用: zz_bs, zz_ky, zz_st, zz_to, zz_nf		*/
/*		などが更新される場合がある			*/
/****************************************************************/
#ifdef USE_PATH
static int get_path_info(char const *path_info)
{
	char buf[48];
	char const *b, *k, *r;

	/* PATH_INFO は、'/' で始まってるような気がしたり */
	if (path_info[0] != '/')
		return 0;

	/* PATH_INFOから、トークンを2個以上抜き出す */
	strncpy(buf, &path_info[1], sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	b = strtok(buf, "/");	/* board */
	k = strtok(NULL, "/");	/* key */
	r = strtok(NULL, "/");	/* range */
	if (!(b && k))
		return 0;

	/* さしあたって bs, ky を更新しる */
	strncpy(zz_bs, b, 1024 - 1);
	strncpy(zz_ky, k, 1024 - 1);

	/* strtok()で切り出したバッファは総長制限が
	   かかっているので、buffer overrunはないはず */
	if (r) {
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
		if (isdigit(*r)) {
			for (p = zz_st; isdigit(*r); p++, r++)
				*p = *r;
			*p = 0;
		}

		if (*r == '-') {
			r++;
			/* toを取り出す */
			if (isdigit(*r)) {
				for (p = zz_to; isdigit(*r); p++, r++)
					*p = *r;
				*p = 0;
			}
		} else {
			/* 範囲指定はないので、
			   単点ポイントと見なす */
			strcpy(zz_to, zz_st);
		}

		/* nofirstの仕様をごまかすためのkludge XXX */
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
/*	GET Env							*/
/****************************************************************/
void zz_GetEnv(void)
{
	currentTime = (long) time(&t_now);
	putenv("TZ=JST-9");
	tzset();
	localtime_r(&t_now, &tm_now);

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

#ifdef GSTR2
	zz_GetString(zz_bs, "b");
	zz_GetString(zz_ky, "k");
	zz_GetString(zz_ls, "l");
	zz_GetString(zz_st, "s");
	zz_GetString(zz_to, "t");
	zz_GetString(zz_nf, "n");
	zz_GetString(zz_im, "i");
#ifdef RAWOUT
	zz_GetString(zz_rw, "r");
#endif
#else
	zz_GetString(zz_bs, "bbs");
	zz_GetString(zz_ky, "key");
	zz_GetString(zz_ls, "ls");
	zz_GetString(zz_st, "st");
	zz_GetString(zz_to, "to");
	zz_GetString(zz_nf, "nofirst");
	zz_GetString(zz_im, "imode");
#ifdef RAWOUT
	zz_GetString(zz_rw, "raw");
#endif
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
	isbusytime = IsBusy2ch();
}

/*----------------------------------------------------------------------
	終了処理
----------------------------------------------------------------------*/
#ifdef GZIP
void atexitfunc(void)
{
#ifdef ZLIB
	if (gzip_flag) {
		gzflush(pStdout, Z_FINISH);
		fflush(stdout);
		gzclose(pStdout);
	}
#else
	fflush(stdout);
	close(1);
	waitpid(pid, NULL, 0);
#endif
}
#endif

/****************************************************************/
/*	MAIN							*/
/****************************************************************/
int main()
{
#ifdef GZIP
#ifndef ZLIB
	int pipefds[2];
#endif
	int whitespace = 2048;
#endif
	char fname[1024];

	/* このシステムでは、-funsigned-charを要求する */
	if ((char)0xFF != (unsigned char)0xFF) {
		puts("Content-Type: text/html\n"
		     "\n"
		     "-funsigned-char required.");
		return 0;
	}

#ifdef ZLIB
	pStdout = (gzFile) stdout;
#endif
	zz_GetEnv();

#ifdef RAWOUT
	if(!rawmode)
#endif
		pPrintf(pStdout, "Content-type: text/html\n");
#ifdef RAWOUT
	else
		pPrintf(pStdout, "Content-type: application/octet-stream\n");
#endif
#ifdef LASTMOD
	sprintf(fname, "../%.256s/dat/%.256s.dat", zz_bs, zz_ky);
#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif
	zz_fileLastmod = getFileLastmod(fname);
	get_lastmod_str(zz_fileLastmod);
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
		gzip_flag = 1;
		pPrintf(pStdout, "Content-Encoding: x-gzip\n");
	} else if (zz_http_encoding && strstr(zz_http_encoding, "gzip")) {
		gzip_flag = 1;
		pPrintf(pStdout, "Content-Encoding: gzip\n");
	} else {
		gzip_flag = 0;
	}
#endif

/*  Get Last-Modified Date */
#ifdef LASTMOD
	pPrintf(pStdout, "Last-Modified: %.256s\n", lastmod_str);
#endif

	pPrintf(pStdout, "\n");
#ifdef DEBUG
	sleep(1);
#endif
	fflush(stdout);

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
		pStdout = gzdopen(1, "wb");

		/*  終了処理登録 */
		atexit(atexitfunc);
		pPrintf = gzprintf;
		gzsetparams(pStdout, Z_BEST_COMPRESSION,
			    Z_DEFAULT_STRATEGY);

		/* put 2048byte */
		while (whitespace--)
			gzputc(pStdout, ' ');
#endif
	}
#endif

	logOut("");

	dat_read();
#ifdef RAWOUT
	rawmode ? dat_out_raw() : dat_out();
#else
	dat_out();
#endif
	
	if (BigBuffer)
		free(BigBuffer);
	BigBuffer = NULL;
	return 0;
}
/****************************************************************/
/*	ERROR END(html_error)					*/
/****************************************************************/
void html_error(char *mes)
{
	char zz_soko[256];
	char tmp[256];
	char doko[256];
	struct stat CountStat;
	char comcom[256];
#ifdef RAWOUT
	if(rawmode) {
		/* ?....はエラー。 */
		pPrintf(pStdout, "-ERR %s\n", mes);
		if (BigBuffer)
			free(BigBuffer);
		BigBuffer = NULL;
		exit(0);
	}
#endif
	
	*tmp = '\0';
	strcpy(tmp, LastChar(zz_ky, '/'));
	strncpy(zz_soko, tmp, 3);

	*comcom = '\0';
	if (strstr(mes, "不調")) {
		sprintf(comcom, R2CH_HTML_ERROR_ADMIN);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_1, mes, mes, mes, comcom);

	if (strstr(zz_http_user_agent, "Katjusha")) {
		pPrintf(pStdout, R2CH_HTML_ERROR_2);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_3);
	html_bannerNew();
	pPrintf(pStdout, R2CH_HTML_ERROR_4);

	if (strstr(mes, "そんな")) {
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

	if (BigBuffer)
		free(BigBuffer);
	BigBuffer = NULL;

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
		if (BigBuffer)
			free(BigBuffer);
		BigBuffer = NULL;
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

	if (BigBuffer)
		free(BigBuffer);
	BigBuffer = NULL;

	exit(0);
}
/****************************************************************/
/*								*/
/****************************************************************/
#ifdef GSTR2
char *zz_GetString(char *dst, char const *tgt)
{
	int i;
	int len;
	int ch = (int) '&';
	int ch2 = (int) '=';
	char const *kk = zz_query_string;
	char *kk0;
	char *kk1;
	for (i = 0; i < 200; i++) {
		if (kk[0] == tgt[0]) {
			kk0 = strchr(kk, ch);	/* & */
			if (!kk0)
				kk0 = strchr(kk, (int) '\0');
			kk1 = strchr(kk, ch2);	/* = */
			len = kk0 - kk1 - 1;
			if (len > 0) {
				if (kk0)
					*kk0 = '\0';
				strncpy(dst, kk1 + 1, 20);
				dst[20 - 1] = '\0';
				if (kk0)
					*kk0 = '&';
				return dst;
			}
		}
		kk = strchr(kk, ch);
		if (!kk)
			break;
		kk++;
	}
	*dst = '\0';
	return dst;
}
#else
char *zz_GetString(char *dst, char *tgt)
{
	int i;
	int ch = (int) '&';
	char *kk0;
	char *kk1;
	char *kk = zz_query_string;
	for (i = 0; i < 200; i++) {
		if (!strncmp(kk, tgt, strlen(tgt))) {
			kk0 = kk;
			kk1 = strchr(kk, ch);
			if (kk1)
				*kk1 = '\0';
			strncpy(dst, kk0 + strlen(tgt) + 1, 1024);
			dst[1024 - 1] = '\0';
			if (kk1)
				*kk1 = ch;
			return dst;
		}
		kk = strchr(kk, ch);
		if (!kk)
			break;
		kk++;
	}
	*dst = '\0';
	return dst;
}
#endif
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
int IsBusy2ch()
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
void html_head(char *title, int line)
{
#ifdef CHUNK_ANCHOR
	int i;
#endif

	if (!is_imode()) {	/* no imode       */
		pPrintf(pStdout, R2CH_HTML_HEADER_1, title, zz_bs);
#ifdef ALL_ANCHOR
		pPrintf(pStdout, R2CH_HTML_ALL_ANCHOR, zz_bs, zz_ky); 
#endif
#ifdef CHUNK_ANCHOR
		for (i = 1; i <= line; i += CHUNK_NUM) {
			pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR,
					zz_bs, zz_ky, i, i + CHUNK_NUM - 1, 
					(i == 1 ? "" : "&n=t"), i);
		}
#endif /* CHUNK_ANCHOR */
#ifdef LATEST_ANCHOR
		pPrintf(pStdout, R2CH_HTML_LATEST_ANCHOR,
				zz_bs, zz_ky, LATEST_NUM, LATEST_NUM);
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

	pPrintf(pStdout, R2CH_HTML_HEADER_2, title);
}
#if (defined(CHUNK_ANCHOR) && CHUNK_NUM > RES_NORMAL) 
# error "Too large CHUNK_NUM!!"
#endif
/****************************************************************/
/*	RELOAD						        */
/****************************************************************/
#ifdef RELOADLINK
void html_reload(int startline)
{
	if (is_imode())	/*  imode */
		pPrintf(pStdout, R2CH_RELOAD_I, zz_bs, zz_ky,
			startline);
	else
		pPrintf(pStdout, R2CH_RELOAD, zz_bs, zz_ky,
			startline);
}
#endif
/****************************************************************/
/*	HTML FOOTER						*/
/****************************************************************/
void html_foot(int line)
{
	if (is_imode())
		return html_foot_im(line);
	if (line <= RES_RED) {
#ifndef COOKIE		
		pPrintf(pStdout, R2CH_HTML_FORM, zz_bs, zz_ky,
			currentTime);
#else
		pPrintf(pStdout, R2CH_HTML_FORM, form_name,
			form_mail, zz_bs, zz_ky,
			currentTime);
#endif
		}

	pPrintf(pStdout, R2CH_HTML_FOOTER);
}
/****************************************************************/
/*	HTML FOOTER(i-MODE)					*/
/****************************************************************/
void html_foot_im(int line)
{
	if (line <= RES_RED) {
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
