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
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=1 WIDTH=100%%><TR><TD>" \
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 WIDTH=100%%>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR BGCOLOR=#ccc><TD HEIGHT=20 NOWRAP><FONT SIZE=2>&nbsp;" \
	"| <A HREF=\"http://www.maido3.com/\" CLASS=BigLine> おすすめ </A>" \
	"| <A HREF=\"http://news.kakiko.com/2chwatch/\" CLASS=BigLine> 2chサーバ監視所 </A>" \
	"| <A HREF=\"http://www.binboserver.com/\" CLASS=BigLine> BinboServer </A>" \
	"| <A HREF=\"http://www.maido3.com/server/subdomain.html\" CLASS=BigLine> Hikky.Zansu.com </A>" \
	"| <A HREF=\"http://news.kakiko.com/\" CLASS=BigLine> 無料サービス </A>" \
	"|</FONT></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"</TABLE>" \
	"<CENTER>" \
	"<BR>" \
	"さぶドメインシリーズ登場<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>syo-ten</FONT></B>.com<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>gasuki</FONT></B>.com<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>zansu</FONT></B>.com<BR>" \
	"おすきな名前を無料で使えます。早い者勝ち。<BR>" \
	"<FONT COLOR=#FF1493><B>工夫しだいで、楽しさ100倍。</B></FONT>" \
	"<P><A HREF=\"http://www.maido3.com/server/\">" \
	"深夜でも快適なレンタルサーバサービス(n)" \
	"</A>" \
	"<BR>あの巨大なサイトも使っています。" \
	"</CENTER><P>" \
	"</TD></TR></TABLE><P>"

/* 使われてない...はず。 */
#define	R2CH_HTML_BBSPINK_BANNER \
	"<CENTER>" \
	"<A TARGET=\"_blank\" HREF=\"http://www.pinknavi.com/?pinkid=PB0001&prg=3&act=0&txt=おいでませ&url=http://www.seiai.com/\">おいでませアダルトサイト</A><br>" \
	"<br><a href=\"http://www.maido3.com/mentai/\"><b><font size=4>深夜でも快適なレンタルサーバサービス</font></b></a><br>" \
	"</CENTER>" \
	"<P>"

/* 使われてない...はず。 */
#define	R2CH_HTML_BANNER \
	"<CENTER>" \
	"<A HREF=\"http://www.maido3.com/server/\" TARGET=\"_blank\">" \
	"深夜でも快適なレンタルサーバサービス" \
	"</A>" \
	"<BR>あの巨大なサイトも使っています。" \
	"</CENTER><P>"

/*
 * BROWSERでレスを表示。
 */
/* レス(mailtoあり): %d=レス番号  %s=mailto %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_RES_MAIL \
	"<dt>%d 名前：<a href=\"mailto:%s \"><b>%s </b></a> 投稿日：%s<dd>%s<br><br>"

/* レス(mailto無し): %d=レス番号 %s=名前 %s=投稿日 %s=本文*/
#define R2CH_HTML_RES_NOMAIL \
	"<dt>%d 名前：<font color=green><b>%s </b></font> 投稿日：%s<dd>%s<br><br>"

/* レス(ここ壊れています): %d=レス番号 */
#define R2CH_HTML_RES_BROKEN_HERE \
	"<dt>%d 名前： 投稿日：[ここ壊れています]<DD>[ここ壊れています]<br><br>"

/* tail: %s=cgi %s=board %s=key %d=開始 %d=終了 %d=レス数
         %s=cgi %s=board %s=key %d=レス数 %d=レス数
         %02d=制限開始時刻 %02d=制限終了時刻 */
#define R2CH_HTML_TAIL \
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d\">次の %d レスを見る</a>" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d\">最新レス %d</a>\n" \
	" (%02d:00PM - %02d:00AM の間一気に全部は読めません)<BR>\n"


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
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d&imode=true\">次の %d レスを見る</a>\n" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d&imode=true\">最新レス %d</a><BR>\n"

/*
 * ERROR
 * ERROR1, ERROR2?, ERROR3, BANNER, ERROR4, (ERROR5_HTML|ERROR5_DAT|ERROR5_NONE)?, ERROR_6
 */

/* エラー(問い合わせ先): */
#define R2CH_HTML_ERROR_ADMIN \
	"<A HREF=\"http://green.jbbs.net/computer/20/bigserver.html\">お問い合わせ先</A>"

/* エラー(1/6): %s=エラーメッセージ %s=エラーメッセージ %s=エラーメッセージ %s=コメント */
#define R2CH_HTML_ERROR_1 \
	"<HTML><HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s</TITLE>" \
	"<style type=\"text/css\">" \
	"<!--\n" \
	"TD.Type1 {" \
	"color: #fff;" \
	"text-align: left;" \
	"}" \
	"A.BigLine {" \
	"color: #000;" \
	"text-decoration: none;" \
	"}\n" \
	"-->" \
	"</style>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=\"#00F\" alink=\"#F00\" vlink=\"#609\">" \
	"<P><FONT SIZE=+1 COLOR=\"#F00\">%s</FONT></B><BR>" \
	"<DL>" \
	"<dt>1 名前：<font color=green><b>" \
	CGIVER \
	"</b></font>投稿日：2001/04/12(木) 15:11<dd>%s<br><br>%s<br>\n"

/* エラー(2/6): かちゅーしゃのみ。 */
#define R2CH_HTML_ERROR_2 \
	"<dt>2 名前：<font color=green><b>" CGIVER "</b></font>投稿日：2001/04/12(木) 15:11<dd>"\
	"<BR>" \
	"<P><A target=\"_blank\" HREF=\"http://www.maido3.com/server/\">" \
	"http://www.maido3.com/server/" \
	"</A><BR>" \
	"さぶドメインシリーズ登場<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>syo-ten</FONT></B>.com<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>gasuki</FONT></B>.com<BR>" \
	"　　　*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>zansu</FONT></B>.com<BR>" \
	"おすきな名前を無料で使えます。早い者勝ち。<BR>" \
	"<FONT COLOR=DEEPPINK><B>工夫しだいで、楽しさ100倍。</B></FONT>" \
	"<P><A target=\"_blank\" HREF=\"http://www.maido3.com/server/\">" \
	"深夜でも快適なレンタルサーバサービス" \
	"</A>" \
	"<BR>あの巨大なサイトも使っています。<BR><BR>" \
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 WIDTH=100%%>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR BGCOLOR=#ccc><TD HEIGHT=20 NOWRAP><FONT SIZE=2>&nbsp;" \
	"| <A target=\"_blank\" HREF=\"http://www.maido3.com/\" CLASS=\"BigLine\"> おすすめ </A>" \
	"| <A target=\"_blank\" HREF=\"http://news.kakiko.com/2chwatch/\" CLASS=\"BigLine\"> 2chサーバ監視所 </A>" \
	"| <A target=\"_blank\" HREF=\"http://www.binboserver.com/\" CLASS=\"BigLine\"> BinboServer </A>" \
	"| <A target=\"_blank\" HREF=\"http://www.maido3.com/server/subdomain.html\" CLASS=\"BigLine\"> Hikky.Zansu.com </A>" \
	"| <A target=\"_blank\" HREF=\"http://news.kakiko.com/\" CLASS=\"BigLine\"> 無料サービス </A>" \
	"|</FONT></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"</TABLE>" \
	"<br><br>"

/* エラー(3/6): */
#define R2CH_HTML_ERROR_3 \
	"</DL><CENTER>\n<BR>"

/* エラー(4/6): */
#define R2CH_HTML_ERROR_4 \
	"<BR></CENTER><HR><FONT SIZE=-2>" CGIVER "</FONT><HR><P>"

/* エラー(5a/6,HTML): %s=場所 %s=スレ番号 */
#define R2CH_HTML_ERROR_5_HTML \
	"隊長! 過去ログ倉庫で、<A HREF=\"%s\">スレッド %s.html</A>　を発見しました。\n"

/* エラー(5b/6,DAT): %s=場所 %s=スレ番号 */
#define R2CH_HTML_ERROR_5_DAT \
	"隊長! 過去ログ倉庫で、<A HREF=\"%s\">スレッド %s.dat</A>　を発見しました。<BR><B><FONT SIZE=+1 COLOR=RED>が</FONT></B>、しかしまだ html化されていません。残念、待つしかない。\n"

/* エラー(5c/6,NONE): %s=板 */
#define R2CH_HTML_ERROR_5_NONE \
	"<A HREF=\"/%s/kako/\">過去ログ倉庫</A>にもありませんでした。<BR>問い合わせても見つかる可能\性はほとんどありません。\n"

/* エラー(6/6): */
#define R2CH_HTML_ERROR_6 \
	"</BODY></HTML>"

/* エラー999: %s=エラーメッセージ %s=board %s=key %s=ls %s=st %s=to %s=nofirst %d=file-size %d=max-line
              %s=time(hh:mm:ss) %s=board %s=soko %s=key %s=key */
/* 使われてない...はず。 */
#define R2CH_HTML_ERROR_999_1 \
	"<HTML><HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>ERROR</TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=#00F alink=#F00 vlink=#609>" \
	"<TABLE border=1 cellspacing=11 cellpadding=2 width=95%% bgcolor=#efefef align=center><TR><TD>" \
	"えらー、%s<P>" \
	"bbs = [%s]　" \
	"key = [%s]　" \
	"ls = [%s]　" \
	"st = [%s]　" \
	"to = [%s]　" \
	"nofirst = [%s]<BR>" \
	"fileSize = [%d bytes] %dlines<BR>" \
	"time=[%s]<BR>" \
	"<BR>「えらー、そんな板orスレッドないです」の場合はここ倉庫へ Go <BR>" \
	"<A HREF=\"/%s/kako/%s/%s.html\">過去ログ倉庫 %s.html</A>\n" \
	"<BR>まだ、dat -> html 変換が行われていない場合が良くあります。" \
	"<BR><A HREF=\"http://teri.2ch.net/test/read.cgi?bbs=accuse&key=980507715&ls=30\">ここ</A>で訴えると、過去ログ整理をしてくれる場合があるかもです。" \
	"</TD></TR></TABLE>" \
	"<BR><BR><BR>"

/* 使われてない...はず。 */
#define R2CH_HTML_ERROR_999_2 \
	"<BR><BR><BR>" \
	"<FONT SIZE=-2>" CGIVER "</FONT>" \
	"</BODY>" \
	"</HTML>"

/*
 * HEADER
 * (HEADER|IMODE_HEADER), (HEADER_REDZONE|HEADER_REDZONE|HEADER_REDZONE)? , HEADER_2
 */
/* ブラウザで見たとき: %s=スレ名 %s=板 %s=板 %s=スレ番号 %s=板 %s=スレ番号 */
#ifndef COOKIE
#define R2CH_HTML_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"<SCRIPT LANGUAGE=JavaScript>" \
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
	"</SCRIPT>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/index2.html\">■掲示板に戻る■</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s\">レスを全部読む</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=100\">最新レス100</A>"
#else
#define R2CH_HTML_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/index2.html\">■掲示板に戻る■</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s\">レスを全部読む</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=100\">最新レス100</A>"
#endif
	
/* i-Modeで見たとき: %s=スレ名 %s=板 %s=板 %s=スレ番号 %d=一度に表示するレス数
                     %s=板 %s=スレ番号 %d=一度に表示するレス数 %d=一度に表示するレス数 */
#define R2CH_HTML_IMODE_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/i/\">■掲示板に戻る■</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&st=1&to=%d&imode=true\">レスを最初から読む</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=%d&imode=true\">最新レス%d</A>"

/* レス数オーバー: %d=最大レス数 */
#define R2CH_HTML_HEADER_RED \
	"<P><TABLE><TR><TD BGCOLOR=RED>" \
	"<FONT COLOR=#fff><BR><BR>レス数が %d を超えています。残念ながら全部は表\示しません。<BR><BR></FONT>" \
	"</TD></TR></TABLE>"

/* レス数やばい: %d=やばくなるレス数 */
#define R2CH_HTML_HEADER_REDZONE \
	"<P><TABLE><TR><TD BGCOLOR=RED>\n" \
	"<FONT COLOR=#fff>レス数が %d を超えています。%dを超えると表\示できなくなるよ。</FONT>" \
	"</TD></TR></TABLE>"

/* レス数そろそろ?: %d=警告の出るレス数 */
#define R2CH_HTML_HEADER_YELLOW \
	"<P><TABLE><TR><TD BGCOLOR=#ff0>" \
	"レス数が %d を超えています。%dを超えると表\示できなくなるよ。" \
	"</TD></TR></TABLE>"

/* スレ名: %s=スレ名 */
#define R2CH_HTML_HEADER_2 \
	"<P><FONT SIZE=+1 COLOR=red>%s </FONT>"

/*
 * RELOAD
 */
#define R2CH_RELOAD \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d\">新レスの表\示</a></center><hr>"

#define R2CH_RELOAD_I \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d&imode=true\">新レスの表\示</a></center><hr>"

/*
 * FOOTER
 * (FORM? , FOOTER) | FORM_IMODE
 */
#ifndef COOKIE
/* %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FORM \
	"<form method=POST action=\"bbs.cgi\">" \
	"<input type=submit value=\"書き込む\" name=submit> 名前： " \
	"<SCRIPT Language=JavaScript>" \
	"<!--\n" \
	"document.write(\"<INPUT TYPE=text NAME=FROM SIZE=19 VALUE=\",N,\"> E-mail<font size=1> (省略可) </font>: <INPUT TYPE=text NAME=mail SIZE=19 VALUE=\",M,\">\");\n" \
	"// -->" \
	"</SCRIPT>" \
	"<NOSCRIPT>" \
	"<INPUT TYPE=text NAME=FROM SIZE=19>" \
	" E-mail<font size=1> (省略可) </font>: " \
	"<INPUT TYPE=text NAME=mail SIZE=19>" \
	"</NOSCRIPT>" \
	"<BR><textarea rows=5 cols=70 wrap=OFF name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#else
/* ブラウザのときのフッタ: %s=投稿者名 %s=mail %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FORM \
	"<form method=POST action=\"bbs.cgi\">" \
	"<input type=submit value=\"書き込む\" name=submit> 名前： " \
	"<INPUT TYPE=text NAME=FROM SIZE=19 VALUE=\"%s\">" \
	" E-mail<font size=1> (省略可) </font>: " \
	"<INPUT TYPE=text NAME=mail SIZE=19 value=\"%s\">" \
	"<BR><textarea rows=5 cols=70 wrap=OFF name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#endif
	
#define R2CH_HTML_FOOTER \
	"</DL><P>" CGIVER "</BODY></HTML>"

/* i-MODEのときのフッタ: %s=板 %s=スレ %ld=現在時刻 */
#define R2CH_HTML_FORM_IMODE \
	"<FORM METHOD=POST ACTION=\"./bbs.cgi\">\n" \
	"<INPUT TYPE=submit VALUE=\"かきこむ\" name=submit> " \
	"NAME：<INPUT TYPE=text NAME=FROM SIZE=20 VALUE=\"\">" \
	"MAIL：<INPUT TYPE=text NAME=mail SIZE=20 VALUE=\"\">" \
	"<br>" \
	"<INPUT TYPE=hidden name=bbs VALUE=%s>" \
	"<INPUT TYPE=hidden name=key VALUE=%s>" \
	"<INPUT TYPE=hidden name=time VALUE=%ld>" \
	"<TEXTAREA ROWS=5 COLS=60 wrap=off NAME=MESSAGE></TEXTAREA>" \
	"</FORM><br>" \
	"<P>" CGIVER "</BODY></HTML>"
