/*
 * read.cgiからHTML部分を抜き出したもの。
 *
 * 編集上の注意。
 * ・%sなどの数と順番を変えてはいけません(%s,%d,%ld)
 * ・次の行へ続けるときは \<改行> です。
 * ・" は \"、改行は\n、%は%%です。
 * ・時々怪しげな\が有りますがそれはおまじないです。消さないでね。
 * ・CGIVERは ""でくくらないこと。
 * ・コメントは ／＊～＊／ (半角で)。
 * ・ころんでも、なかない。
 */

/*
 * BANNERS: スレが無い等のエラー時に出る広告。
 */
#define	R2CH_HTML_NEW_BANNER \
	"<table cellspacing=0 cellpadding=0 border=1 width=100%%><tr><td>" \
	"<table cellspacing=0 cellpadding=0 border=0 width=100%%>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr bgcolor=#cccccc><td height=20 nowrap><font size=2>&nbsp;" \
	"| <a href=\"http://www.maido3.com/\" class=BigLine>おすすめ</a>" \
	"| <a href=\"http://news.kakiko.com/2chwatch/\" class=BigLine>2chサーバ監視所</a>" \
	"| <a href=\"http://www.binboserver.com/\" class=BigLine>BinboServer</a>" \
	"| <a href=\"http://www.maido3.com/server/subdomain.html\" class=BigLine>Hikky.Zansu.com</a>" \
	"| <a href=\"http://news.kakiko.com/\" class=BigLine>無料サービス</a>" \
	"|</font></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"</table>" \
	"<br>" \
	"<center>" \
	"さぶドメインシリーズ登場<br>" \
	"　　　*****.<b><font size=+1 face=Arial color=#ff1493>syo-ten</font></b>.com<br>" \
	"　　　*****.<b><font size=+1 face=Arial color=#ff1493>gasuki</font></b>.com<br>" \
	"　　　*****.<b><font size=+1 face=Arial color=#ff1493>zansu</font></b>.com<br>" \
	"おすきな名前を無料で使えます。早い者勝ち。<br>" \
	"<b><font color=#ff1493>工夫しだいで、楽しさ100倍。</font></b>" \
	"<p><a href=\"http://www.maido3.com/server/\">" \
	"深夜でも快適なレンタルサーバサービス(n)" \
	"</a>" \
	"<br>あの巨大なサイトも使っています。" \
	"</center><p>" \
	"</td></tr></table><p>"

/* 使われてない...はず。 */
#define	R2CH_HTML_BBSPINK_BANNER \
	"<center>" \
	"<a href=\"http://www.pinknavi.com/?pinkid=PB0001&prg=3&act=0&txt=おいでませ&url=http://www.seiai.com/\" target=\"_blank\">おいでませアダルトサイト</a><br>" \
	"<br><a href=\"http://www.maido3.com/mentai/\"><b><font size=4>深夜でも快適なレンタルサーバサービス</font></b></a><br>" \
	"</center>" \
	"<p>"

/* 使われてない...はず。 */
#define	R2CH_HTML_BANNER \
	"<center>" \
	"<a href=\"http://www.maido3.com/server/\" target=\"_blank\">" \
	"深夜でも快適なレンタルサーバサービス" \
	"</a>" \
	"<br>あの巨大なサイトも使っています。" \
	"</center><p>"

/*
 * BROWSERでレスを表示。
 */
/* レス(mailtoあり): %d=レス番号  %s=mailto %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_RES_MAIL(n, l, m, nm, d, t) \
	"<dt>" n " 名前：<a name=" l " href=\"mailto:" m " \"><b>" nm " </b></a> 投稿日：" d "<dd>" t "<br><br>"

/* レス(mailto無し): %d=レス番号 %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_RES_NOMAIL(n, l, nm, d, t) \
	"<dt>" n " <a name=" l ">名前：<font color=green><b>" nm " </b></font></a> 投稿日：" d "<dd>" t "<br><br>"

/* レス(sage): %d=レス番号 %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_RES_SAGE(n, l, nm, d, t) \
	"<dt>" n " <a name=" l ">名前：<font color=#0000c0><b>" nm " </b></font></a> 投稿日：" d "<dd>" t "<br><br>"

/* レス(ここ壊れています): %d=レス番号 */
#define R2CH_HTML_RES_BROKEN_HERE \
	"<dt>%d 名前： 投稿日：[ここ壊れています]<dd>[ここ壊れています]<br><br>"

/* tail: npath=生成するURLその1 nst=次のxxxレス
	 lpath=生成するURLその2 ls=最新レスxxx
         %02d=制限開始時刻 %02d=制限終了時刻 */
#define R2CH_HTML_T_TAIL(npath, nst, lpath, ls) \
	" <a href=\"" npath "\">次の" nst "レス</a>" \
	" <a href=\"" lpath "\">最新レス" ls "</a>\n" \
	" (%02d:00PM - %02d:00AM の間一気に全部は読めません)<br>\n"

/* tail: PATHナシ
	%s=cgi %s=board %s=key %d=開始 %d=終了 %d=レス数
	%s=cgi %s=board %s=key %d=レス数 %d=レス数
	%02d=制限開始時刻 %02d=制限終了時刻 */
#define R2CH_HTML_TAIL \
	R2CH_HTML_T_TAIL("./%s?bbs=%s&key=%s&st=%d&to=%d&n=t", "%d", \
			 "./%s?bbs=%s&key=%s&ls=%d&n=t", "%d")
/* tail: PATH仕様
	%d=開始 %d=終了 %d=レス数
	%d=レス数 %d=レス数
	%02d=制限開始時刻 %02d=制限終了時刻 */
#define R2CH_HTML_PATH_TAIL \
	R2CH_HTML_T_TAIL("%d-%d", "%d", \
			 "./?ls=%d", "%d")

/*
 * i-MODEでレスを表示。
 */
/* レス(mailtoあり): %d=レス番号  %s=mailto %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_IMODE_RES_MAIL \
	"<p>[%d:<a href=\"mailto:%s \"><b>%s </b></a>(%s)]<br>%s</p><hr>"

/* レス(mailto無し): %d=レス番号 %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_IMODE_RES_NOMAIL \
	"<p>[%d:<b>%s </b>(%s)]<br>%s</p><hr>"

/* レス(ここ壊れています): %d=レス番号 */
#define R2CH_HTML_IMODE_RES_BROKEN_HERE "<p>[%d:ここ壊れています]</p><hr>"

/* tail: %s=cgi %s=board %s=key %d=開始 %d=終了 %d=レス数 %s=cgi %s=board %s=key %d=レス数 %d=レス数 */
#define R2CH_HTML_IMODE_TAIL \
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d&imode=true\">次の%dレス</a>\n" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d&imode=true&n=t\">最新レス%d</a><br>\n"

/*
 * ERROR
 * ERROR1, ERROR2?, ERROR3, BANNER, ERROR4, (ERROR5_HTML|ERROR5_DAT|ERROR5_NONE)?, ERROR_6
 */

/* エラー(問い合わせ先): */
#define R2CH_HTML_ERROR_ADMIN \
	"<a href=\"http://green.jbbs.net/computer/20/bigserver.html\">お問い合わせ先</a>"

/* エラー(1/6): %s=エラーメッセージ %s=エラーメッセージ %s=エラーメッセージ %s=コメント */
#define R2CH_HTML_ERROR_1 \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>%s</title>" \
	"<style type=\"text/css\">" \
	"<!--\n" \
	"td.Type1 {" \
	"color: #fff;" \
	"text-align: left;" \
	"}" \
	"a.BigLine {" \
	"color: #000;" \
	"text-decoration: none;" \
	"}\n" \
	"-->" \
	"</style>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<p><font size=+1 color=red>%s</font></b><br>" \
	"<dl>" \
	"<dt>1 名前：<font color=green><b>" \
	CGIVER \
	"</b></font>投稿日：2001/04/12(木) 15:11<dd>%s<br><br>%s<br>\n"

/* エラー(2/6): かちゅーしゃのみ。 */
#define R2CH_HTML_ERROR_2 \
	"<dt>2 名前：<font color=green><b>" CGIVER "</b></font>投稿日：2001/04/12(木) 15:11<dd>"\
	"<br>" \
	"<p><a href=\"http://www.maido3.com/server/\" target=\"_blank\">" \
	"http://www.maido3.com/server/" \
	"</a><br>" \
	"さぶドメインシリーズ登場<br>" \
	"　　　*****.<b><font size=+1 face=\"Arial\" color=deeppink>syo-ten</font></b>.com<br>" \
	"　　　*****.<b><font size=+1 face=\"Arial\" color=deeppink>gasuki</font></b>.com<br>" \
	"　　　*****.<b><font size=+1 face=\"Arial\" color=deeppink>zansu</font></b>.com<br>" \
	"おすきな名前を無料で使えます。早い者勝ち。<br>" \
	"<font color=deeppink><b>工夫しだいで、楽しさ100倍。</b></font>" \
	"<p><a target=\"_blank\" href=\"http://www.maido3.com/server/\">" \
	"深夜でも快適なレンタルサーバサービス" \
	"</a>" \
	"<br>あの巨大なサイトも使っています。<br><br>" \
	"<table cellspacing=0 cellpadding=0 border=0 width=100%%>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr bgcolor=#cccccc><td height=20 nowrap><font size=2>&nbsp;" \
	"| <a href=\"http://www.maido3.com/\" class=\"BigLine\" target=\"_blank\">おすすめ</a>" \
	"| <a href=\"http://news.kakiko.com/2chwatch/\" class=\"BigLine\" target=\"_blank\">2chサーバ監視所</a>" \
	"| <a href=\"http://www.binboserver.com/\" class=\"BigLine\" target=\"_blank\">BinboServer</a>" \
	"| <a href=\"http://www.maido3.com/server/subdomain.html\" class=\"BigLine\" target=\"_blank\">Hikky.Zansu.com</a>" \
	"| <a href=\"http://news.kakiko.com/\" class=\"BigLine\" target=\"_blank\">無料サービス</a>" \
	"|</font></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"</table>" \
	"<br><br>"

/* エラー(3/6): */
#define R2CH_HTML_ERROR_3 \
	"</dl><center>\n<br>"

/* エラー(4/6): */
#define R2CH_HTML_ERROR_4 \
	"<br></center><HR><font size=-2>" CGIVER "</font><HR><p>"

/* エラー(5a/6,HTML): %s=場所 %s=スレ番号 */
#define R2CH_HTML_ERROR_5_HTML \
	"隊長! 過去ログ倉庫で、<a href=\"%s\">スレッド %s.html</a> を発見しました。\n"

/* エラー(5b/6,DAT): %s=場所 %s=スレ番号 */
#define R2CH_HTML_ERROR_5_DAT \
	"隊長! 過去ログ倉庫で、<a href=\"%s\">スレッド %s.dat</a> を発見しました。<br><b><font size=+1 color=red>が</font></b>、しかしまだ html化されていません。残念、待つしかない。\n"

/* エラー(5c/6,NONE): %s=板 */
#define R2CH_HTML_ERROR_5_NONE \
	"<a href=\"/%s/kako/\">過去ログ倉庫</a>にもありませんでした。<br>問い合わせても見つかる可能\性はほとんどありません。\n"

/* エラー(6/6): */
#define R2CH_HTML_ERROR_6 \
	"</body></html>"

/* エラー999: %s=エラーメッセージ %s=board %s=key %s=ls %s=st %s=to %s=nofirst %d=file-size %d=max-line
              %s=time(hh:mm:ss) %s=board %s=soko %s=key %s=key */
/* 使われてない...はず。 */
#define R2CH_HTML_ERROR_999_1 \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>ERROR</title>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<table border=1 cellspacing=11 cellpadding=2 width=95%% bgcolor=#efefef align=center><tr><td>" \
	"えらー、%s<p>" \
	"bbs = [%s]　" \
	"key = [%s]　" \
	"ls = [%s]　" \
	"st = [%s]　" \
	"to = [%s]　" \
	"nofirst = [%s]<br>" \
	"fileSize = [%d bytes] %dlines<br>" \
	"time=[%s]<br>" \
	"<br>「えらー、そんな板orスレッドないです」の場合はここ倉庫へ Go <br>" \
	"<a href=\"/%s/kako/%s/%s.html\">過去ログ倉庫 %s.html</a>\n" \
	"<br>まだ、dat -> html 変換が行われていない場合が良くあります。" \
	"<br><a href=\"http://teri.2ch.net/test/read.cgi?bbs=accuse&key=980507715&ls=30\">ここ</a>で訴えると、過去ログ整理をしてくれる場合があるかもです。" \
	"</td></tr></table>" \
	"<br><br><br>"

/* 使われてない...はず。 */
#define R2CH_HTML_ERROR_999_2 \
	"<br><br><br>" \
	"<font size=-2>" CGIVER "</font>" \
	"</body>" \
	"</html>"

/*
 * HEADER:
 * ((HEADER ALL_ANCHOR CHUNK_ANCHOR LATEST_ANCHOR)|IMODE_HEADER)
 * (HEADER_REDZONE|HEADER_REDZONE|HEADER_REDZONE)
 * HEADER_2
 */
#ifndef COOKIE
#define R2CH_HTML_COOKIE_SCRIPT \
	"<script language=JavaScript>" \
	"<!--\n" \
	"N=g(\"NAME\");" \
	"M=g(\"MAIL\");" \
	"function g(key,tmp1,tmp2,xx1,xx2,xx3){" \
	"tmp1=\" \"+document.cookie+\";\";" \
	"xx1=xx2=0;" \
	"len=tmp1.length;" \
	"while(xx1<len){" \
	"xx2=tmp1.indexOf(\";\",xx1);" \
	"tmp2=tmp1.substring(xx1+1,xx2);" \
	"xx3=tmp2.indexOf(\"=\");" \
	"if(tmp2.substring(0,xx3)==key){" \
	"return(unescape(tmp2.substring(xx3+1,xx2-xx1-1)));" \
	"}" \
	"xx1=xx2+1;" \
	"}return(\"\");" \
	"}\n" \
	"// -->" \
	"</script>" 
#else
#define R2CH_HTML_COOKIE_SCRIPT ""
#endif

/* ブラウザで見たとき: %s=スレ名 %s=板 */
#define R2CH_HTML_HEADER_1(title, board) \
	"<html>" \
	"<head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>" title "</title>" \
	R2CH_HTML_COOKIE_SCRIPT \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<a href=\"" board "\">■掲示板に戻る■</a>"

/* path=生成するURL */
#define R2CH_HTML_T_ALL_ANCHOR(path) \
	" <a href=\"" path "\">レスを全部読む</a>"
/* path=生成するURL st=開始位置 */
#define R2CH_HTML_T_CHUNK_ANCHOR(path, st) \
	" <a href=\"" path "\">" st "-</a>"
/* path=生成するURL ls=記事数 */
#define R2CH_HTML_T_LATEST_ANCHOR(path, ls) \
	" <a href=\"" path "\">最新レス"ls"</a>"

/* 以下のものは、PATHナシ仕様で用いられる */

/* %s=板 %s=スレ番号 */
#define R2CH_HTML_ALL_ANCHOR \
	R2CH_HTML_T_ALL_ANCHOR(CGINAME "?bbs=%s&key=%s")
/* %s=板 %s=スレ番号 %d=開始レス %d=終了レス %s="&n=f" %d=開始レス */
#define R2CH_HTML_CHUNK_ANCHOR \
	R2CH_HTML_T_CHUNK_ANCHOR(CGINAME "?bbs=%s&key=%s&st=%d&to=%d%s", "%d")
/* %s=板 %s=スレ番号 %d=レス個数 %d=レス個数 */
#define R2CH_HTML_LATEST_ANCHOR \
	R2CH_HTML_T_LATEST_ANCHOR(CGINAME "?bbs=%s&key=%s&ls=%d&n=t", "%d")

/* 以下のものは、PATH仕様で用いられる
   板・スレ番はすでに決定しているので、埋め込まれない */

/* 外部パラメータナシ */
#define R2CH_HTML_PATH_ALL_ANCHOR \
	R2CH_HTML_T_ALL_ANCHOR("1-")
/* %d=開始レス %d=終了レス %d=開始レス */
#define R2CH_HTML_PATH_CHUNK_ANCHOR \
	R2CH_HTML_T_CHUNK_ANCHOR("%d-%d", "%d")
/* %d=レス個数 %d=レス個数 */
#define R2CH_HTML_PATH_LATEST_ANCHOR \
	R2CH_HTML_T_LATEST_ANCHOR("./?ls=%d", "%d")


/* i-Modeで見たとき: %s=スレ名 %s=板 %s=板 %s=スレ番号 %d=一度に表示するレス数
                     %s=板 %s=スレ番号 %d=一度に表示するレス数 %d=一度に表示するレス数 */
#define R2CH_HTML_IMODE_HEADER_1 \
	"<html>" \
	"<head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>%s</title>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<a href=\"/%s/i/\">■掲示板に戻る■</a>" \
	" <a href=\"" CGINAME "?bbs=%s&key=%s&st=1&to=%d&imode=true\">レスを最初から読む</a>" \
	" <a href=\"" CGINAME "?bbs=%s&key=%s&ls=%d&imode=true&n=t\">最新レス%d</a>"

/* レス数オーバー: %d=最大レス数 */
#define R2CH_HTML_HEADER_RED \
	"<p><table><tr><td bgcolor=red>" \
	"<br><br><font color=white>レス数が%dを超えています。残念ながら全部は表\示しません。</font>" \
	"</td></tr></table>"

/* レス数やばい: %d=やばくなるレス数 */
#define R2CH_HTML_HEADER_REDZONE \
	"<p><table><tr><td bgcolor=red>\n" \
	"<font color=white>レス数が%dを超えています。%dを超えると表\示できなくなるよ。</font>" \
	"</td></tr></table>"

/* レス数そろそろ?: %d=警告の出るレス数 */
#define R2CH_HTML_HEADER_YELLOW \
	"<p><table><tr><td bgcolor=yellow>" \
	"レス数が%dを超えています。%dを超えると表\示できなくなるよ。" \
	"</td></tr></table>"

/* スレ名: %s=スレ名 */
#define R2CH_HTML_HEADER_2 \
	"<p><font size=+1 color=red>%s</font>" \
	"<DL>"

#define R2CH_HTML_HEADER_2_I \
	"<p><font size=+1 color=red>%s</font>"

/*
 * RELOAD
 */
#define R2CH_HTML_T_RELOAD(path) \
	"<hr><center><a href=\"" path "\">新レスの表\示</a></center><hr>"

#define R2CH_HTML_RELOAD \
	R2CH_HTML_T_RELOAD("read.cgi?bbs=%s&key=%s&st=%d&n=t")

#define R2CH_HTML_PATH_RELOAD \
	R2CH_HTML_T_RELOAD("%d-")

#define R2CH_HTML_RELOAD_I \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d&i=t&n=t\">新レスの表\示</a></center><hr>"

/*
 * FOOTER
 * (FORM? , FOOTER) | FORM_IMODE
 */
#ifndef COOKIE
/* %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FORM(depth, bs, ky, tm) \
	"<form method=post action=\"" depth "bbs.cgi\">" \
	"<input type=submit value=\"書き込む\" name=submit> 名前： " \
	"<script language=JavaScript>" \
	"<!--\n" \
	"document.write(\"<input type=text name=FROM size=19 value=\",N,\"> E-mail<font size=1> (省略可) </font>: <input type=text name=mail size=19 value=\",M,\">\");\n" \
	"// -->" \
	"</script>" \
	"<noscript>" \
	"<input type=text name=FROM size=19>" \
	" E-mail<font size=1> (省略可) </font>: " \
	"<input type=text name=mail size=19>" \
	"</noscript>" \
	"<br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=" bs ">" \
	"<input type=hidden name=key value=" ky ">" \
	"<input type=hidden name=time value=" tm ">" \
	"</form>"
#else
#define R2CH_HTML_FORM \
	"<form method=post action=\"bbs.cgi\">" \
	"<input type=submit value=\"書き込む\" name=submit> 名前： " \
	"<input type=text name=FROM size=19 value=\"%s\">" \
	" E-mail<font size=1> (省略可) </font>: " \
	"<input type=text name=mail size=19 value=\"%s\">" \
	"<br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#endif
	
/* ブラウザのときのフッタ: %s=投稿者名 %s=mail %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FOOTER \
	"</dl><p>" CGIVER "</body></html>"

/* i-MODEのときのFORM: %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FORM_IMODE \
	"<form method=post action=\"./bbs.cgi\">\n" \
	"<input type=submit balue=\"かきこむ\" name=submit> " \
	"NAME：<input type=text name=FROM size=20 value=\"\">" \
	"MAIL：<input type=text name=mail size=20 value=\"\">" \
	"<br>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"<textarea rows=5 cols=60 wrap=off name=MESSAGE></textarea>" \
	"</form><br>"

/* i-MODEのときのフッタ: %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FOOTER_IMODE \
	"<p>" CGIVER "</body></html>"

/* html_error()用エラーメッセージ */
#define ERRORMES_TOO_HUGE "このスレッド大きすぎます。"
#define ERRORMES_NOT_FOUND "そんな板orスレッドないです。"
#define ERRORMES_NO_MEMORY "メモリの確保に失敗しました。"
#define ERRORMES_MAINTENANCE "調整中。。。"
#define ERRORMES_LOGOUT "なんか不調です。"


/* スレダイジェスト用HTML */

#define R2CH_HTML_INDEX_HEADER(title1, title2) \
"<head><META http-equiv=Content-Type content=\"text/html; charset=shift_jis\">

<style type=\"text/css\">
body{margin: 2%%;} 
h3{color:red} 
img{border:solid 0} 
div{border:.5em ridge;margin:1em;padding:1em} 
.m{border-color:#cfc;background:#cfc} 
.r{border-color:#eee;background:#eee} 
dt{margin:1em 0 0} 
b{color:#080} 
h1, .c{text-align:center} 
</style>
<title>" title1 "</title></head><body bgcolor=white>

<h1><a href=\"http://www.2ch.net/info.html\"><img src=\"http://www.geocities.co.jp/SiliconValley-Cupertino/5657/img20010514210907.gif\" border=0></a></h1>
<div class=m>
<form method=POST action=\"../test/bbs.cgi\">
<h2>" title2 "</h2>
<dl>
<dt>■扱う話題■<dd><ul>
<li>HTML、CSS、FLASHなどの<em>サイト制作の技術</em><li>JavaScript、VBScriptなどの<em>クライアントサイドプログラム</em><li>Webサイトの運営および管理についての情報交換・雑談<li>2chでは荒らし依頼はできません</ul>
<dt>■板違いな話題■<dd><ul>
<li>HP観察、ネットアイドル、ネット上の事件→『<a href=\"http://kaba.2ch.net/net/index2.html\">ネット事件板</a>』<li>レンタルサーバ、ドメイン→『<a href=\"/perl/index2.html\">レンタル鯖板</a>』<li>HPの宣伝、ネットするだけで儲かる話し等→『<a href=\"http://cocoa.2ch.net/ad/index2.html\">宣伝掲示板</a>』<li>CGI、Perl、PHPなどのサーバサイドプログラミング→『<a href=\"http://corn.2ch.net/php/index2.html\">WebProg</a>』</ul>
<dt>■各種総合スレ■<dd><ul><li>自分のHPの評価を求める場合→『<a href=\"/test/read.cgi?bbs=hp&key=993635902\">■■HP評価スレッド■■</a>』<li>スタイルシートについての質問→『<a href=\"/test/read.cgi?bbs=hp&key=996828258\">CSS、スタイルシート質問用スレッド</a>』<li>FLASHについての質問→『<a href=\"/test/read.cgi?bbs=hp&key=994898823\">FLASHの製作に関する質問はここで♪</a>』<li>JavaScriptについての質問→『<a href=\"/test/read.cgi?bbs=hp&key=997445548\">JavaScriptの質問用スレッド</a>』<li>それ以外の質問・板案内→『<a href=\"/test/read.cgi?bbs=hp&key=995867650\">Webサイト制作初心者用スレッド</a>』<li>『<a href=\"/test/read.cgi?bbs=hp&key=991370397\">Dreamweaver</a>』、『<a href=\"/test/read.cgi?bbs=hp&key=997848771\">ホームページビルダー</a>』、『<a href=\"/test/read.cgi?bbs=hp&key=987875928\">Golive</a>』、『<a href=\"/test/read.cgi?bbs=hp&key=991367944\">Photoshop・illustrater</a>』等は専用スレがあります</ul>
<dt>■スレッドをたてる前に■<dd><ul>
<li>板違いではないか、既出ではないか、もう一度確認しましょう。<li>単発質問は禁止です。総合スレに書き込んでください。</ul>
<dt>■住人の心得■<dd><ul>
<li>板違い・スレ違いを発見したら優しく誘導してあげましょう。<li>勝手にスレッドを終了させるような行為は慎みましょう。<li>駄スレ糞スレにはレスせず完全放置。「放置の美学」です。<li>「空気を読む」のも忘れずに。<li>ﾏﾀｰﾘとね。</ul>
</dl>

<input type=submit value=\"スレッド作成\" name=submit><input type=hidden name=bbs value=hp><input type=hidden name=time value=998848311>
<a href=http://www.2ch.net/before.html>書き込む前に読んでね</a> ｜ <a href=http://www.2ch.net/guide/>2chガイド</a> ｜ <a href=\"http://www.2ch.net/guide/faq.html\">FAQ</a></form>
<p><a href=http://210.150.210.150/sage/bbstable.html Target=_blank>■掲示板一覧■</a></p>
</div>

<div class=m>
<small>"

#define R2CH_HTML_INDEX_LABEL(anchor, num, label, title) \
	"<a href=\"" anchor "/?ls=50\">" num ":</a> " \
	"<a href=\"" label "\">" title "</a>\n"

#define R2CH_HTML_INDEX_ANCHOR(anchor, num, title) \
	"<a href=\"" anchor "/?ls=50\" target=\"body\">" num ": " title "</a>\n"

#define R2CH_HTML_INDEX_AD \
"<p><a href=\"-\">スレッド一覧</a></p>
</small>
</div>

<div class=m>
<a href=\"http://www.gekiura.com/index.shtml\"><img src=\"http://www.ceres.dti.ne.jp/~gekiura/banner/bana_m3.jpg\"></a>
<a href=\"http://software.wakusei.ne.jp/free/\"><img src=\"http://www2.kitanet.ne.jp/~mail/2ch/fplnklgo.gif\"></a>
<a href=\"http://www.uranews.com/articles/tools/\"><img src=\"http://www.uranews.com/articles/tools/utoolb.gif\"></a>
<a href=\"http://www.jyukujyo.com/\"><img src=\"http://www.jyukujyo.com/ad/jyukujyo.gif\"></a>
<a href=\"http://www.tanteifile.com\"><img src=\"http://homepage2.nifty.com/kirik/2ch/game.gif\"></a>
<a href=\"http://homepage2.nifty.com/tbks/\"><img src=\"http://homepage2.nifty.com/tbks/chiee0ffff.jpg\"></a>
<p><a href=\"http://www.2ch.net/ad.html\">広告出稿のお問い合わせ</a></p>
</div>


<TABLE BORDER=1 cellspacing=0 cellpadding=0 WIDTH=95%%><TR bgcolor=black><TD colspan=2>
<TABLE WIDTH=100%%><TR bgcolor=#FFCC44><TD>
■ 2chは、<A HREF=\"http://www.maido3.com/server/usagi/news.html\" TARGET=\"_blank\"><FONT COLOR=NAVY SIZE=+1><B>ここのサーバ</B></FONT></A>を使ってるです。。。
</TD></TR></TABLE>
</TD></TR><TR><TD bgcolor=black>
<font color=#CCFFCC>
　<FONT SIZE=+1 FACE=\"Arial\"><B>.jp</B></FONT> ドメインお持ちのお客様大歓迎。<A HREF=\"http://welcome.maido3.jp/\" TARGET=\"_blank\"><FONT SIZE=+1 FACE=\"Arial\" COLOR=RED><B>maido3.jp</B></FONT></A><BR>
　<FONT SIZE=+1 FACE=\"Arial\"><B>.com .net .org</B></FONT> 取得代行します。(US$10) <A HREF=\"http://www.maido3.com/server/\" TARGET=\"_blank\"><FONT SIZE=+1 FACE=\"Arial\" color=cyan><B>maido3.com</B></FONT></A><BR>
　<FONT SIZE=+1 FACE=\"Arial\" COLOR=WHITE><B>.cc .to .tv .fm .ca</B></FONT> 取得代行はじめました。<font color=white face=\"Arial\" size=-1><b>NEW</b></font><BR>
</td><td align=center>
無料ホームページスペース<P>
<A HREF=\"http://space.kakiko.com/\" TARGET=\"_blank\"><FONT SIZE=+1 FACE=\"Arial\" color=BLUE><B>Space.Kakiko.com</B></FONT></A><BR>
</font>
</TD></TR><TR bgcolor=black><TD colspan=2>
<TABLE WIDTH=100%%><TR bgcolor=black><TD>
<font color=#ccffcc>月々1,000円からの</font> <A HREF=\"http://www.binboserver.com/\" target=\"_blank\"><font color=cyan face=\"Arial\"><B>BinboServer.com</B></font></A>
</TD><TD>
<font color=#ccffcc>２ちゃんねるも使っている</font> <A HREF=\"http://www.maido3.com/server/\" target=\"_blank\"><font color=cyan face=\"Arial\"><B>Big-Server.com</B></font></A>
</TD></TR></TABLE>
</TD></TR><TR bgcolor=black><TD colspan=2>
<TABLE WIDTH=100%%><TR bgcolor=#FFCC44><TD align=right>
&gt;&gt; ２ちゃんねる、<A HREF=\"http://news.kakiko.com/mentai/\"><FONT COLOR=NAVY>サーバ監視所</FONT></A>
</TD></TR></TABLE>
</TD></TR></TABLE>"

#define R2CH_HTML_DIGEST_HEADER_1(label) \
"<div class=r><a name=\"#" label "\">"

#define R2CH_HTML_DIGEST_HEADER_2(title) \
"<h3>" title "</h3><dl>"

#define R2CH_HTML_DIGEST_FOOTER \
"<form method=POST action=\"../test/bbs.cgi\">
<input type=hidden name=bbs value=hp><input type=hidden name=key value=998774537><input type=hidden name=time value=998848311>
<input type=submit value=\"投稿\" name=submit> 名前：
<INPUT TYPE=text NAME=FROM SIZE=19> E-mail：<INPUT TYPE=text NAME=mail SIZE=19>
<ul>
<textarea rows=5 cols=60 wrap=OFF name=MESSAGE></textarea>
</ul>
<a href=\"../test/read.cgi?bbs=hp&key=998774537\">全レス</a> <a href=\"../test/read.cgi?bbs=hp&key=998774537&ls=50\">最新50</a>
</form></dl>
</div>"

#define R2CH_HTML_INDEX_FOOTER \
"<p class=c>
<a href=\"http://www.honeyline.co.jp/\"><img src=\"http://honeyline.co.jp/link/img/honey8.gif\"></a> 
<a href=http://FRIEND.P-CLUB.COM/WEB/link.cgi/DDN2785/HREF/hp><img src=http://FRIEND.P-CLUB.COM/WEB/link.cgi/DDN2785/IMG/hp></a><br>
どのような形の削除依頼であれ公開させていただきます
</p>

</body>"


/* スレインデクス用HTML */

#define R2CH_HTML_SUBBACK_HEADER \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=x-sjis\">" \
	"</head>" \
	"<body target=\"body\">" \
	"<font size=-1>\n"

#define R2CH_HTML_SUBBACK_ITEM(anchor, num, title) \
	"<a href=\"" anchor "/?ls=50\">" num ": " title "</a>\n"

#define R2CH_HTML_SUBBACK_FOOTER \
	"<div align=right><a href=\"/tech/kako/\"><b>過去ログ倉庫はこちら</b></a></font></body></html>"
