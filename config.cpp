// config.cpp: Config クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include <math.h>
#include <commctrl.h>
#include <commdlg.h>
#include "common.h"
#include "resource.h"
#include "main.h"
#include "config.h"
#include "screen.h"
#include "draw.h"
#include "draw8.h"
#include "draw16.h"
#include "pic8.h"
#include "pic16.h"

void Config::
init()
{
  file[0]= 0;
  host[0]= 0;
};

int Config::
gammacolor(int col) const
{
  int range= gammahigh-gammalow;
  if(range<=0) {
    return(gammalow<<16|gammalow<<8|gammalow);
  };
  int mid= gammamid;
  if(mid>=gammahigh) mid= gammahigh-1;
  if(mid<=gammalow) mid= gammalow+1;
  double gamma= -log((double)(mid-gammalow)/range)/log(2);

  int a, out;
  a= (int)(pow((double)(col>>16&255)/255,gamma)*range+gammalow+0.5);
  if(a<0) a= 0; else if(a>255) a= 255;
  out= a<<16;
  a= (int)(pow((double)(col>>8&255)/255,gamma)*range+gammalow+0.5);
  if(a<0) a= 0; else if(a>255) a= 255;
  out|= a<<8;
  a= (int)(pow((double)(col&255)/255,gamma)*range+gammalow+0.5);
  if(a<0) a= 0; else if(a>255) a= 255;
  out|= a;
  return(out);
};

int Config::
parsecolor(const char*p)
{
  int i;
  char c;
  int col= -1;
  if(*p=='#') {
    col= 0;
    for(i= 0; i<6; i++) {
      c= *++p;
      if(c>='a'&&c<='f') c= c-'a'+10;
      else if(c>='A'&&c<='F') c= c-'A'+10;
      else if(c>='0'&&c<='9') c-= '0';
      else break;
      col= col*16+c;
    };
    if(i<6) col= -1;
  };
  return(col);
};

void Config::
setrandom(void)
{
  int a1, a2, a, c;
  a1= colback1>>16&255; a2= colback2>>16&255; if(a1>a2) a= a1, a1= a2, a2= a;
  c= (rand()%(a2-a1+1)+a1)<<16;
  a1= colback1>>8&255; a2= colback2>>8&255; if(a1>a2) a= a1, a1= a2, a2= a;
  c|= (rand()%(a2-a1+1)+a1)<<8;
  a1= colback1&255; a2= colback2&255; if(a1>a2) a= a1, a1= a2, a2= a;
  colback= c|(rand()%(a2-a1+1)+a1);
};

void Config::
writem(HANDLE fdw, WCHAR const *wbuf)
{
  char buf[_MAX_PATH];
  DWORD d;
  d= WideCharToMultiByte(0, 0, wbuf, -1, buf, numof(buf), 0, 0)-1;
  WriteFile(fdw, buf, d, &d, 0);
};

PCWSTR Config::
save()
{
  int rt;
  if(file[0]!=0) {
    WCHAR wfile[_MAX_PATH];
    wfile[0]= 0;
    HANDLE fdr, fdw;
    fdr= CreateFile(file, GENERIC_READ, FILE_SHARE_READ, 0
    , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(fdr==INVALID_HANDLE_VALUE) { //新規書き込み
      fdw= CreateFile(file, GENERIC_WRITE, 0, 0
      , CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    } else {
      wsprintf(wfile, L"%s.new", file);
      fdw= CreateFile(wfile, GENERIC_WRITE, 0, 0
      , CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    };
    if(fdw==INVALID_HANDLE_VALUE) return L"cFailed to create file.";
    int inblock= 0;
    int lastc= '\n';
    //書き戻しが必要かどうか
    int qhost= 0;
    if(fdr==INVALID_HANDLE_VALUE) qhost= (host[0]!=0);
    int qname= (name[0]!=0);
    int qsjis= (sjis!=0);
    int qcolback= (colback1!=0xffffff||colback2!=0xffffff);
    int qcoluline= (coluline!=0xff7777);
    int qcolor[8];
    qcolor[0]= (color[0]!=0x000000);
    qcolor[1]= (color[1]!=0xcc0000);
    qcolor[2]= (color[2]!=0x00aa00);
    qcolor[3]= (color[3]!=0x888800);
    qcolor[4]= (color[4]!=0x0000ff);
    qcolor[5]= (color[5]!=0xcc00cc);
    qcolor[6]= (color[6]!=0x008888);
    qcolor[7]= (color[7]!=0x666666);
    int qpic= (pic[0]!=0);
    int qgamma= (pic[0]!=0&&!(gammalow==0&&gammamid==128&&gammahigh==255));
    int qyen= (yen!=1);
    int qnobold= (nobold!=0);
    int qstay= (stay!=0);
    int qymax= (ymax!=24);
    int qenter= (enter!=1);
    int qnewline= (newline!=0);
    int qecho= (echo!=0);
    int qbsdel= (bsdel!=0);

    for(;;) {
      WCHAR wbuf[_MAX_PATH];
      char lc;
      char rbuf[_MAX_PATH], *p;
      DWORD d, bytes= 0;
      p= rbuf; lc= 0;
      if(fdr!=INVALID_HANDLE_VALUE) {
        while(bytes<_MAX_PATH-1) {
	  rt= ReadFile(fdr, &lc, 1, &d, 0);
	  if(rt==0||d==0) break;
	  *p++= (char)lc; bytes++;
	  if(lc=='\n') break;
        };
      };
      *p= 0;
      p= rbuf;
      while(*p>0&&*p<=' ') p++;
      char key[_MAX_PATH], *q;
      q= key;
      while(*p&&*p!='=') {
	if(*p>='A'&&*p<='Z') *q++= *p++|0x20; else *q++= *p++;
      };
      while(q!=key&&q[-1]>0&&q[-1]<=' ') q--;
      *q= 0;
      if(bytes==0||key[0]=='['&&q[-1]==']') { //block start/end
	if(strcmp(key, "[24term]")==0) {
	  inblock= 1;
	} else if(inblock==1||bytes==0&&inblock==0) { //block end
	  if(inblock==0) {
	    if(lastc!='\n') writem(fdw, L"\r\n");
	    writem(fdw, L"[24term]\r\n");
	    lastc= '\n'; inblock= 2;
	  };
	  //書いてないのを書く
	  for(int ix= 0; ix<8; ix++) if(qcolor[ix]) break;
	  if(qhost||qname||qsjis||qcolback||qcoluline||ix<8||qpic||qgamma
	  ||qyen||qnobold||qstay||qymax||qenter||qnewline||qbsdel||qecho) {
	    if(lastc!='\n') writem(fdw, L"\r\n");
	    if(qhost) {
	       writem(fdw, L"host="); writem(fdw, host); writem(fdw, L"\r\n");
	    };
	    if(qname) {
               writem(fdw, L"title="); writem(fdw, name); writem(fdw, L"\r\n");
	    };
	    if(qsjis) {
              writem(fdw, L"kanji=");
              if(sjis==1) writem(fdw, L"sjis\r\n");
              else if(sjis==2) writem(fdw, L"jis\r\n");
	      else writem(fdw, L"euc\r\n");
	    };
	    if(qcolback) {
	      wsprintf(wbuf, L"back=#%06x\r\n", colback1);
	      if(colback1!=colback2) wsprintf(wbuf+12, L"-%06x\r\n", colback2);
              writem(fdw, wbuf);
	    };
	    if(qcoluline) {
	      wsprintf(wbuf, L"under=#%06x\r\n", coluline);
	      writem(fdw, wbuf);
	    };
	    for(ix= 0; ix<8; ix++) {
	      if(qcolor[ix]) {
	        wsprintf(wbuf, L"fore%d=#%06x\r\n", ix, color[ix]);
	        writem(fdw, wbuf);
	      };
	      qcolor[ix]= 0;
	    };
	    if(qpic) {
	      WCHAR *s= pic;
	      if(file[0]=='\\'&&pic[0]=='\\') {
	        WCHAR* s0= wcsrchr(file, '\\');
	        WCHAR* s1= wcsrchr(pic, '\\');
		if(s0-file==s1-pic&&_wcsnicmp(file, pic, s0-file)==0) s= s1+1;
	      };
              writem(fdw, L"picture="); writem(fdw, s); writem(fdw, L"\r\n");
	    };
	    if(qgamma&&pic[0]) {
	      wsprintf(wbuf, L"gamma=%d,%d,%d\r\n", gammalow, gammamid, gammahigh);
	      writem(fdw, wbuf);
	    };
	    if(qyen) {
              writem(fdw, L"code5c=");
	      if(yen==1) writem(fdw, L"yen\r\n");
	      else writem(fdw, L"backslash\r\n");
	    };
	    if(qnobold) {
              writem(fdw, L"bold=");
	      if(nobold==1) writem(fdw, L"disable\r\n");
	      else writem(fdw, L"enable\r\n");
	    };
	    if(qstay) {
              writem(fdw, L"autoclose=");
	      if(stay==1) writem(fdw, L"no\r\n");
	      else writem(fdw, L"yes\r\n");
	    };
	    if(qymax) {
              wsprintf(wbuf, L"scroll=%d\r\n", ymax-24);
	      writem(fdw, wbuf);
	    };
	    if(qenter) {
	      writem(fdw, L"enter=");
	      if(enter==0) writem(fdw, L"crlf\r\n");
	      else if(enter==2) writem(fdw, L"lf\r\n");
	      else writem(fdw, L"cr\r\n");
	    };
	    if(qnewline) {
	      writem(fdw, L"newline=");
	      if(newline==1) writem(fdw, L"cr\r\n");
	      else if(newline==2) writem(fdw, L"lf\r\n");
	      else writem(fdw, L"crlf\r\n");
	    };
	    if(qbsdel) {
	      writem(fdw, L"bsdel=");
	      if(bsdel==1) writem(fdw, L"swap\r\n");
	      else writem(fdw, L"normal\r\n");
	    };
	    if(qecho) {
	      writem(fdw, L"echo=");
	      if(echo==1) writem(fdw, L"yes\r\n");
	      else writem(fdw, L"no\r\n");
	    };
	    qhost= 0; qname= 0; qsjis= 0; qcolback= 0; qcoluline= 0; qpic= 0; qgamma= 0;
	    qyen= 0; qnobold= 0; qstay= 0; qymax= 0; qenter= 0; qnewline= 0; qbsdel= 0; qecho= 0;
	  };
	};
	if(bytes) WriteFile(fdw, rbuf, bytes, &d, 0);
	lastc= lc;
      } else if(inblock!=1) {
	WriteFile(fdw, rbuf, bytes, &d, 0);
	lastc= lc;
      } else if(strcmp(key, "host")==0) {
        //ホスト名がある場合のみ書き換え
	if(*p=='=') {
	  p++;
	  while(*p>0&&*p<=' ') p++;
	};
	if(*p!=0&&*p!='-') {
          writem(fdw, L"host="); writem(fdw, host); writem(fdw, L"\r\n");
	  lastc= '\n';
	} else {
	  WriteFile(fdw, rbuf, bytes, &d, 0);
	  lastc= lc;
	};
	qhost= 0; //maybe 0
      } else if(strcmp(key, "title")==0) {
        writem(fdw, L"title="); writem(fdw, name); writem(fdw, L"\r\n");
	lastc= '\n'; qname= 0;
      } else if(strcmp(key, "kanji")==0) {
        writem(fdw, L"kanji=");
        if(sjis==1) writem(fdw, L"sjis\r\n");
        else if(sjis==2) writem(fdw, L"jis\r\n");
	else writem(fdw, L"euc\r\n");
	lastc= '\n'; qsjis= 0;
      } else if(strcmp(key, "color")==0) {
        //delete this line -- backward compatibility
      } else if(strcmp(key, "back")==0) {
	wsprintf(wbuf, L"back=#%06x\r\n", colback1);
	if(colback1!=colback2) wsprintf(wbuf+12, L"-%06x\r\n", colback2);
        writem(fdw, wbuf);
	lastc= '\n'; qcolback= 0;
      } else if(strcmp(key, "under")==0) {
	wsprintf(wbuf, L"under=#%06x\r\n", coluline);
	writem(fdw, wbuf);
	lastc= '\n'; qcoluline= 0;
      } else if(strncmp(key, "fore", 4)==0) {
        int ix= atol(key+4);
	if(ix>=0&&ix<=7) {
	  wsprintf(wbuf, L"fore%d=#%06x\r\n", ix, color[ix]);
	  writem(fdw, wbuf);
	  lastc= '\n'; qcolor[ix]= 0;
	};
      } else if(strcmp(key, "picture")==0) {
	WCHAR *s= pic;
	if(file[0]=='\\'&&pic[0]=='\\') {
	  WCHAR* s0= wcsrchr(file, '\\');
	  WCHAR* s1= wcsrchr(pic, '\\');
	  if(s0-file==s1-pic&&_wcsnicmp(file, pic, s0-file)==0) s= s1+1;
	};
	writem(fdw, L"picture="); writem(fdw, s); writem(fdw, L"\r\n");
	lastc= '\n'; qpic= 0;
      } else if(strcmp(key, "gamma")==0) {
        if(pic[0]) {
	  wsprintf(wbuf, L"gamma=%d,%d,%d\r\n", gammalow, gammamid, gammahigh);
	  writem(fdw, wbuf);
	  lastc= '\n'; qgamma= 0;
	};
      } else if(strcmp(key, "code5c")==0) {
        writem(fdw, L"code5c=");
	if(yen==1) writem(fdw, L"yen\r\n");
	else writem(fdw, L"backslash\r\n");
	lastc= '\n'; qyen= 0;
      } else if(strcmp(key, "bold")==0) {
        writem(fdw, L"bold=");
	if(nobold==1) writem(fdw, L"disable\r\n");
	else writem(fdw, L"enable\r\n");
	lastc= '\n'; qnobold= 0;
      } else if(strcmp(key, "autoclose")==0) {
        writem(fdw, L"autoclose=");
	if(stay==1) writem(fdw, L"no\r\n");
	else writem(fdw, L"yes\r\n");
	lastc= '\n'; qstay= 0;
      } else if(strcmp(key, "scroll")==0) {
        wsprintf(wbuf, L"scroll=%d\r\n", ymax-24);
	writem(fdw, wbuf);
	lastc= '\n'; qymax= 0;
      } else if(strcmp(key, "enter")==0) {
	writem(fdw, L"enter=");
	if(enter==0) writem(fdw, L"crlf\r\n");
	else if(enter==2) writem(fdw, L"lf\r\n");
	else writem(fdw, L"cr\r\n");
	lastc= '\n'; qenter= 0;
      } else if(strcmp(key, "newline")==0) {
	writem(fdw, L"newline=");
	if(newline==1) writem(fdw, L"cr\r\n");
	else if(newline==2) writem(fdw, L"lf\r\n");
	else writem(fdw, L"crlf\r\n");
	lastc= '\n'; qnewline= 0;
      } else if(strcmp(key, "bsdel")==0) {
        writem(fdw, L"bsdel=");
        if(bsdel==1) writem(fdw, L"swap\r\n");
        else writem(fdw, L"normal\r\n");
	lastc= '\n'; qbsdel= 0;
      } else if(strcmp(key, "echo")==0) {
	writem(fdw, L"echo=");
        if(echo==1) writem(fdw, L"yes\r\n");
        else writem(fdw, L"no\r\n");
	lastc= '\n'; qecho= 0;
      } else {
	WriteFile(fdw, rbuf, bytes, &d, 0);
	lastc= lc;
      };
      if(bytes==0) break;
    };
    if(fdr!=INVALID_HANDLE_VALUE) CloseHandle(fdr);
    CloseHandle(fdw);
    if(fdr!=INVALID_HANDLE_VALUE) {
      DeleteFile(file);
      MoveFile(wfile, file);
      return L"1"; //overwrite
    };
    return L"0"; //normal
  };
  return L"";
};

PCWSTR Config::
load()
{
  int rt;

  name[0]= 0;
  pic[0]= 0;
  sjis= 0;
  yen= 1;
  nobold= 0;
  stay= 0;
  ymax= 24;
  enter= 1;
  newline= 0;
  bsdel= 0;
  echo= 0;
  gammalow= 0; gammamid=128; gammahigh= 255;
  cert= 0; certalloc= 0; certlen= 0;

  //for backward compatibility
  int baseblack= 0;
  colback1= -1; colback2= -1;
  coluline= -1;
  for(int i= 0; i<8; i++) color[i]= -1;

  if(file[0]!=0) {
    HANDLE fdr;
    fdr= CreateFile(file, GENERIC_READ, FILE_SHARE_READ, 0
    , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(fdr!=INVALID_HANDLE_VALUE) {
      int inblock= 0;
      for(;;) {
        char lc;
        char rbuf[_MAX_PATH], *p;
        DWORD d, bytes= 0;
        p= rbuf; lc= 0;
        while(bytes<_MAX_PATH-1) {
	  rt= ReadFile(fdr, &lc, 1, &d, 0);
	  if(rt==0||d==0) break;
	  *p++= (char)lc; bytes++;
	  if(lc=='\n') break;
        };
	if(bytes==0) break;
	//後ろ取る
	while(p!=rbuf&&p[-1]>0&&p[-1]<=' ') p--;
        *p= 0;
        p= rbuf;
        while(*p>0&&*p<=' ') p++;
        char key[_MAX_PATH], *q;
        q= key;
        while(*p&&*p!='=') {
	  if(*p>='A'&&*p<='Z') *q++= *p++|0x20; else *q++= *p++;
        };
        while(q!=key&&q[-1]>0&&q[-1]<=' ') q--;
        *q= 0;
        if(key[0]=='['&&q[-1]==']') { //block start
	  if(strcmp(key, "[24term]")==0) {
	    inblock= 1;
	  } else if(strcmp(key, "[cert]")==0) {
	    inblock= 2;
	    if(certalloc==0) {
	      certalloc= 2000;
	      cert= (BYTE*)malloc(certalloc);
	    };
	    certlen= 0;
	  };
	} else if(inblock==1) { //24term
	  if(*p=='=') p++;
	  while(*p>0&&*p<=' ') p++;
          if(*p) {
	    if(strcmp(key, "host")==0) {
	      if(host[0]==0) {
	        MultiByteToWideChar(0, 0, p, -1, host, _MAX_PATH);
	      };
	    } else if(strcmp(key, "title")==0) {
	      MultiByteToWideChar(0, 0, p, -1, name, _MAX_PATH);
	    } else if(strcmp(key, "kanji")==0) {
	      if((*p|0x20)=='s') sjis= 1;
	      if((*p|0x20)=='j') sjis= 2;
	    } else if(strcmp(key, "scroll")==0) {
	      ymax= atol(p)+24;
	      if(ymax<24) ymax= 24;
	      if(ymax>1024) ymax= 1024;
	    } else if(strcmp(key, "color")==0) {
	      if((*p|0x20)=='b') baseblack= 1;
	    } else if(strcmp(key, "back")==0) {
              if(*p=='#') {
                int col= 0;
                for(i= 0; i<6; i++) {
                  lc= *++p;
                  if(lc>='a'&&lc<='f') lc= lc-'a'+10;
                  else if(lc>='A'&&lc<='F') lc= lc-'A'+10;
                  else if(lc>='0'&&lc<='9') lc-= '0';
                  else break;
                  col= col*16+lc;
                };
                if(i==6) {
		  colback1= colback2= col;
		  if(*++p=='-') { //range
		    col= 0;
                    for(i= 0; i<6; i++) {
                      lc= *++p;
                      if(lc>='a'&&lc<='f') lc= lc-'a'+10;
                      else if(lc>='A'&&lc<='F') lc= lc-'A'+10;
                      else if(lc>='0'&&lc<='9') lc-= '0';
                      else break;
                      col= col*16+lc;
                    };
                    if(i==6) colback2= col;
		  };
		};
              };
	    } else if(strcmp(key, "under")==0) {
	      coluline= parsecolor(p);
	    } else if(strncmp(key, "fore", 4)==0) {
	      int ix= atol(key+4);
	      if(ix>=0&&ix<=7) color[ix]= parsecolor(p);
	    } else if(strcmp(key, "picture")==0) {
	      MultiByteToWideChar(0, 0, p, -1, pic, _MAX_PATH);
	    } else if(strcmp(key, "gamma")==0) {
	      gammalow= atol(p);
	      p= strchr(p, ',');
	      if(p!=0) {
		gammamid= atol(p+1);
		p= strchr(p+1, ',');
	      };
	      if(p!=0) {
		gammahigh= atol(p+1);
	      };
	      if(p==0
	      ||gammalow>gammamid
	      ||gammamid>gammahigh) {
		gammalow= 0; gammamid=128; gammahigh= 255;
	      };
	    } else if(strcmp(key, "code5c")==0) {
	      if((*p|0x20)=='b') yen= 0;
	    } else if(strcmp(key, "bold")==0) {
	      if((*p|0x20)=='d'||(*p|0x20)=='n') nobold= 1;
	    } else if(strcmp(key, "autoclose")==0) {
	      if((*p|0x20)=='d'||(*p|0x20)=='n') stay= 1;
	    } else if(strcmp(key, "enter")==0) {
	      for(char *p0= p; *p0; p0++) if(*p0>='A'&&*p0<='Z') *p0|= 0x20;
	      if(strcmp(p, "crlf")==0) enter= 0;
	      else if(strcmp(p, "lf")==0) enter= 2;
	    } else if(strcmp(key, "newline")==0) {
	      for(char *p0= p; *p0; p0++) if(*p0>='A'&&*p0<='Z') *p0|= 0x20;
	      if(strcmp(p, "cr")==0) newline= 1;
	      else if(strcmp(p, "lf")==0) newline= 2;
	    } else if(strcmp(key, "bsdel")==0) {
	      if((*p|0x20)=='s'||(*p|0x20)=='e'||(*p|0x20)=='y') bsdel= 1;
	    } else if(strcmp(key, "echo")==0) {
	      if((*p|0x20)=='e'||(*p|0x20)=='y') echo= 1;
	    };
	  };	  
	} else if(inblock==2) { //cert
	  int ph= 0;
	  int b= 0;
	  for(p= rbuf; lc= *p; p++) {
	    if(lc>='A'&&lc<='Z') lc-= 'A';
	    else if(lc>='a'&&lc<='z') lc= lc-'a'+26;
	    else if(lc>='0'&&lc<='9') lc= lc-'0'+52;
	    else if(lc=='+') lc= 62;
	    else if(lc=='/') lc= 63;
	    else if(lc>0&&lc<=' ') lc= -1;
	    else break; //unknown char occur
	    if(lc>=0) {
	      if(ph==0) {
	        b= lc;
	      } else {
	        b= (b<<ph|lc>>(6-ph))&255;
		if(certlen>=certalloc) {
		  certalloc+= certalloc/2;
		  cert= (BYTE*)realloc(cert, certalloc);
		};
                cert[certlen++]= b;
                b= lc;
	      };
	      ph= (ph+2)&6;
	    };
          };
        };
      };
      CloseHandle(fdr);
    };
  };

  //baseblack for backward compatibility
  if(color[0]<0) color[0]= baseblack ? 0xffffff : 0x000000;
  if(color[1]<0) color[1]= baseblack ? 0xff0000 : 0xcc0000;
  if(color[2]<0) color[2]= baseblack ? 0x00ff00 : 0x00aa00;
  if(color[3]<0) color[3]= baseblack ? 0xffff00 : 0x888800;
  if(color[4]<0) color[4]= baseblack ? 0x4444ff : 0x0000ff;
  if(color[5]<0) color[5]= baseblack ? 0xff00ff : 0xcc00cc;
  if(color[6]<0) color[6]= baseblack ? 0x00ffff : 0x008888;
  if(color[7]<0) color[7]= baseblack ? 0x999999 : 0x666666;
  if(coluline<0) coluline= baseblack ? 0xff7777 : 0xff7777;
  if(colback1<0) colback1= colback2= baseblack ? 0x000000 : 0xffffff;

  black= 0;
  if((colback1>>16&255)*30+(colback1>>8&255)*59+(colback1&255)*11<12800) black= 1;

  setrandom();
  return L"";
};

PCWSTR Config::
checkpic() const
{
  if(pic[0]==0) return L"nNo picture file specified.";

  WCHAR picp[_MAX_PATH];
  if(pic[0]!='\\'&&file[0]=='\\') { //add path
    int a= wcsrchr(file, '\\')-file+1;
    wcsncpy(picp, file, a);
    wcscpy(picp+a, pic);
  } else {
    wcscpy(picp, pic);
  };
  DWORD dd;
  HANDLE pich= CreateFile(picp, GENERIC_READ
  , FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if(pich==INVALID_HANDLE_VALUE) return L"pPicture File not found.";

  BITMAPFILEHEADER bfh;
  BITMAPINFOHEADER bih;
  ReadFile(pich, &bfh, sizeof(bfh), &dd, 0);
  ReadFile(pich, &bih, sizeof(bih), &dd, 0);
  if(!(bfh.bfType==0x4D42&&bih.biWidth>=480&&(bih.biHeight<=-240||bih.biHeight>=240)
  &&bih.biBitCount==8&&bih.biCompression==BI_RGB)) {
    CloseHandle(pich); return L"pNot a picture file.";
  };
  CloseHandle(pich);
  return L"";
};

void Config::
setcomboheight(HWND w)
{
//  int h= SendMessage(w, CB_GETCOUNT, 0, 0);
  RECT r;
  GetWindowRect(w, &r);
  SetWindowPos(w, 0, 0, 0, r.right-r.left, 240, SWP_NOZORDER|SWP_NOMOVE);
};

void Config::
hostget(HWND w)
{
  if(w==0) return;
  WCHAR buf[_MAX_PATH];
  GetDlgItemText(w, IDC_NAME, name, numof(name));
  GetDlgItemText(w, IDC_HOST, buf, numof(buf));
  //両側空白カット
  int n= wcslen(buf);
  while(n>0&&buf[n-1]==' ') n--;
  buf[n]= 0;
  n= 0;
  while(buf[n]==' ') n++;
  wcscpy(host, buf+n);
};

void Config::
hostset(HWND w)
{
  if(w==0) return;
  
  SetDlgItemText(w, IDC_HOST, host);
  int option= 0;
  WCHAR *p= host;
  while(*p==' ') p++;
  if(*p!='#') {
    while(*p&&*p!=':') p++;
    if(*p==':') {
      p++;
      while(*p>='0'&&*p<='9') p++;
      for(;option==0&&*p&&*p!=':';p++) {
        if(*p=='z'||*p=='Z') option= 1;
        if(*p=='s'||*p=='S') option= 2;
        if(*p=='p'||*p=='P') option= 4;
      };
    };
  };
  SendDlgItemMessage(w, IDC_OPTION, CB_SETCURSEL, option, 0); 
  p= host;
  while(*p==' ') p++;
  if(*p=='#'&&p[1]!='#') { //comport
    p++;
    WCHAR buf[_MAX_PATH], *np;
    np= buf;
    if(*p>='0'&&*p<='9') wcscpy(buf, L"COM"), np= buf+3;
    while(*p&&*p!=':') {
      *np++= *p++;
    };
    *np++= ':'; *np= 0;
    int a= SendDlgItemMessage(w, IDC_PORT, CB_FINDSTRING, 0, (LPARAM)buf); 
    SendDlgItemMessage(w, IDC_PORT, CB_SETCURSEL, a, 0); //CB_ERR=-1
    int irmode= 0;
    int baud= 9600;
    int bits= 0;
    int flow= 0;
    if(*p==':') {
      p++;
      if(*p>='0'&&*p<='9') {
	baud= 0;
	while(*p>='0'&&*p<='9') baud= baud*10+(*p++ - '0');
      };
      for(;*p&&*p!=':';p++) ;
      if(*p==':') {
	WCHAR c;
	while((c= *++p)&&c!=':') {
	  switch(c) {
	  case '7':
	    bits|= 8; break;
	  case 'e': case 'E':
	    bits|= 2; break;
	  case 'o': case 'O':
	    bits|= 4; break;
	  case '2':
	    bits|= 1; break;
	  case 'h': case 'H': case 'r': case 'R':
	    flow= 1; break;
	  case 's': case 'S': case 'x': case 'X':
	    flow= 2; break;
	  case 'i': case 'I':
	    irmode= 1;
	  };
	};
      };
    };
    wsprintf(buf, L"%d", baud);
    a= SendDlgItemMessage(w, IDC_BAUD, CB_FINDSTRINGEXACT, 0, (LPARAM)buf);
    SendDlgItemMessage(w, IDC_BAUD, CB_SETCURSEL, a, 0); //CB_ERR=-1
    SendDlgItemMessage(w, IDC_BITS, CB_SETCURSEL, (bits>>3^1)*6+(bits&7), 0);
    SendDlgItemMessage(w, IDC_FLOW, CB_SETCURSEL, flow, 0);
    SendDlgItemMessage(w, IDC_IRMODE, BM_SETCHECK, irmode==1?BST_CHECKED:BST_UNCHECKED, 0);
  } else {
    SendDlgItemMessage(w, IDC_PORT, CB_SETCURSEL, -1, 0);
    SendDlgItemMessage(w, IDC_BAUD, CB_SETCURSEL, 3, 0);
    SendDlgItemMessage(w, IDC_BITS, CB_SETCURSEL, 6, 0);
    SendDlgItemMessage(w, IDC_FLOW, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(w, IDC_IRMODE, BM_SETCHECK, BST_UNCHECKED, 0);
  };
  SetDlgItemText(w, IDC_NAME, name);
};

BOOL CALLBACK Config::
hostwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.hostproc(w, msg, wp, lp);
};

BOOL Config::
hostproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    {
      //SendDlgItemMessage(w, IDC_HOST, WM_SETFONT, 0/*systemfont*/, TRUE);
      //コントロール項目セット
      SendDlgItemMessage(w, IDC_OPTION, CB_ADDSTRING, 0, (LPARAM)L"None");
      SendDlgItemMessage(w, IDC_OPTION, CB_ADDSTRING, 0, (LPARAM)L"SSLtelnet");
      SendDlgItemMessage(w, IDC_OPTION, CB_ADDSTRING, 0, (LPARAM)L"SSL");
      SendDlgItemMessage(w, IDC_OPTION, CB_ADDSTRING, 0, (LPARAM)L"CheckCert");
      SendDlgItemMessage(w, IDC_OPTION, CB_ADDSTRING, 0, (LPARAM)L"Passive");
      setcomboheight(GetDlgItem(w, IDC_OPTION));
      int rt, i;
      HKEY regx;
      RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Drivers\\Active", 0, 0, &regx);
      for(i= 0; ; i++) {
        WCHAR value[_MAX_PATH];
	DWORD len, type;
	len= numof(value);
        rt= RegEnumKeyEx(regx, i, value, &len, 0, 0, 0, 0);
	if(rt) break;
	HKEY reg;
	RegOpenKeyEx(regx, value, 0, 0, &reg);
	len= sizeof(value);
	rt= RegQueryValueEx(reg, L"Name", 0, &type, (BYTE*)value, &len);
	if(rt==0&&wcsnicmp(value, L"COM", 3)==0) {
	  int port= _wtol(value+3);
	  len= sizeof(value);
	  rt= RegQueryValueEx(reg, L"Key", 0, &type, (BYTE*)value, &len);
	  RegCloseKey(reg);
	  if(rt==0) {
	    RegOpenKeyEx(HKEY_LOCAL_MACHINE, value, 0, 0, &reg);
	    len= sizeof(value);
	    rt= RegQueryValueEx(reg, L"FriendlyName", 0, &type, (BYTE*)value, &len);
	    RegCloseKey(reg);
	  };
          WCHAR buf[_MAX_PATH];
	  if(rt==0) {
            wsprintf(buf, L"COM%d: %s", port, value);
	  } else {
	    wsprintf(buf, L"COM%d:", port);
	  };
	  SendDlgItemMessage(w, IDC_PORT, CB_ADDSTRING, 0, (LPARAM)buf);
	} else {
	  RegCloseKey(reg);
	};
      };
      RegCloseKey(regx);
      setcomboheight(GetDlgItem(w, IDC_PORT));

      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"1200");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"2400");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"4800");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"9600");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"19200");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"38400");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"57600");
      SendDlgItemMessage(w, IDC_BAUD, CB_ADDSTRING, 0, (LPARAM)L"115200");
      setcomboheight(GetDlgItem(w, IDC_BAUD));
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7n1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7n2");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7e1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7e2");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7o1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"7o2");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8n1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8n2");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8e1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8e2");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8o1");
      SendDlgItemMessage(w, IDC_BITS, CB_ADDSTRING, 0, (LPARAM)L"8o2");
      setcomboheight(GetDlgItem(w, IDC_BITS));
      SendDlgItemMessage(w, IDC_FLOW, CB_ADDSTRING, 0, (LPARAM)L"None");
      SendDlgItemMessage(w, IDC_FLOW, CB_ADDSTRING, 0, (LPARAM)L"Hard");
      SendDlgItemMessage(w, IDC_FLOW, CB_ADDSTRING, 0, (LPARAM)L"Soft");
      setcomboheight(GetDlgItem(w, IDC_FLOW));

      hostset(w);
    };
    return(TRUE);
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_COMMAND:
    {
      if(wp==MAKELONG(IDC_OPTION, CBN_SELENDOK)) {
        WCHAR host[_MAX_PATH];
        GetDlgItemText(w, IDC_HOST, host, numof(host));
	WCHAR *p= host;
	while(*p==' ') p++;
	if(*p!='#') { //not comport
	  WCHAR buf[_MAX_PATH];
	  WCHAR *q= buf;
	  while(*p&&*p!=':') p++;
	  if(*p==':') {
	    p++; while(*p>='0'&&*p<='9') p++;
	    wcsncpy(q, host, p-host); q+= p-host;
	  } else {
	    wcsncpy(q, host, p-host); q+= p-host;
	    *q++= ':';
	  };
          int sel= SendDlgItemMessage(w, IDC_OPTION, CB_GETCURSEL, 0, 0);
	  if(sel==1) { //ssl telnet
	    *q++= 'z';
	    for(;*p&&*p!=':';p++) {
	      if(*p!='z'&&*p!='Z'&&*p!='s'&&*p!='S') *q++= *p;
	    };
	  } else if(sel==2) { //ssl
	    *q++= 's';
	    for(;*p&&*p!=':';p++) {
	      if(*p!='z'&&*p!='Z'&&*p!='s'&&*p!='S') *q++= *p;
	    };
	  } else if(sel==3) { //cert
	    *q++= 'c';
	    for(;*p&&*p!=':';p++) {
	      if(*p!='c'&&*p!='C') *q++= *p;
	    };
	  } else if(sel==4) { //passive
	    *q++= 'p';
	    for(;*p&&*p!=':';p++) {
	      if(*p!='p'&&*p!='P') *q++= *p;
	    };
	  } else { //none- delete all options
	    for(;*p&&*p!=':';p++) ;
	  };
	  if(*p==0&&q[-1]==':') --q;
	  wcscpy(q, p);
	  SetDlgItemText(w, IDC_HOST, buf);
	};
      } else if(wp==MAKELONG(IDC_PORT, CBN_SELENDOK)) {
        WCHAR host[_MAX_PATH];
        GetDlgItemText(w, IDC_HOST, host, numof(host));
	WCHAR *p= host;
	while(*p==' ') p++;
	if(*p=='#'&&p[1]!='#') {
	  WCHAR buf[_MAX_PATH];
          GetDlgItemText(w, IDC_PORT, buf, numof(buf));
          if(wcsnicmp(buf, L"COM", 3)==0) {
	    int port= _wtol(buf+3);
	    WCHAR *q= buf;
	    wcsncpy(q, host, p-host); q+= p-host;
	    q+= wsprintf(q, L"#%d", port);
	    p++; while(*p&&*p!=':') p++;
            wcscpy(q, p);
            SetDlgItemText(w, IDC_HOST, buf);
	  };
	} else {
          //接続文字列新規作成
          WCHAR buf[_MAX_PATH];
          GetDlgItemText(w, IDC_PORT, buf, numof(buf));
          if(wcsnicmp(buf, L"COM", 3)==0) {
            int port= _wtol(buf+3);
            GetDlgItemText(w, IDC_BAUD, buf, numof(buf));
            int baud= _wtol(buf);
            if(baud==0) baud= 9600;
            int bits= SendDlgItemMessage(w, IDC_BITS, CB_GETCURSEL, 0, 0);
            int flow= SendDlgItemMessage(w, IDC_FLOW, CB_GETCURSEL, 0, 0);
            WCHAR *q= buf;
            q+= wsprintf(q, L"#%d", port);
            q+= wsprintf(q, L":%d", baud);
	    int ir= SendDlgItemMessage(w, IDC_IRMODE, BM_GETCHECK, 0, 0);
            if(bits!=6||flow!=0||ir==BST_CHECKED) {
              *q++= ':';
              if(bits<6) *q++= '7';
              if((bits/2)%3==1) *q++= 'e';
              if((bits/2)%3==2) *q++= 'o';
              if(bits&1) *q++= '2';
              if(flow==1) *q++= 'h';
              if(flow==2) *q++= 's';
	      if(ir==BST_CHECKED) *q++= 'i';
            };
            *q= 0;
            SetDlgItemText(w, IDC_HOST, buf);
	  };
	};
      } else if(wp==MAKELONG(IDC_BAUD, CBN_SELENDOK)) {
        WCHAR host[_MAX_PATH];
        GetDlgItemText(w, IDC_HOST, host, _MAX_PATH);
	WCHAR *p= host;
	while(*p==' ') p++;
	if(*p=='#'&&p[1]!='#') {
	  WCHAR buf[_MAX_PATH];
	  GetDlgItemText(w, IDC_BAUD, buf, numof(buf));
	  int baud= _wtol(buf);
	  WCHAR *q= buf;
	  while(*p&&*p!=':') p++;
	  wcsncpy(q, host, p-host); q+= p-host;
          q+= wsprintf(q, L":%d", baud);
	  if(*p==':') {
	    p++; while(*p>='0'&&*p<='9') p++;
	    wcscpy(q, p);
	  };
          SetDlgItemText(w, IDC_HOST, buf);
	};
      } else if(wp==MAKELONG(IDC_BITS, CBN_SELENDOK)||wp==MAKELONG(IDC_FLOW, CBN_SELENDOK)
      ||wp==MAKELONG(IDC_IRMODE, BN_CLICKED)) {
        WCHAR host[_MAX_PATH];
        GetDlgItemText(w, IDC_HOST, host, _MAX_PATH);
	WCHAR *p= host;
        int bits= SendDlgItemMessage(w, IDC_BITS, CB_GETCURSEL, 0, 0);
        int flow= SendDlgItemMessage(w, IDC_FLOW, CB_GETCURSEL, 0, 0);
	int irmode= SendDlgItemMessage(w, IDC_IRMODE, BM_GETCHECK, 0, 0);
	while(*p==' ') p++;
	if(*p=='#'&&p[1]!='#') {
	  WCHAR buf[_MAX_PATH];
	  WCHAR *q= buf;
	  while(*p&&*p!=':') p++;
	  if(*p==':') {
	    p++;
	    while(*p&&*p!=':') p++;
            wcsncpy(q, host, p-host); q+= p-host;
	    if(*p==':') p++;
	  } else { //no baud specifier
	    wcsncpy(q, host, p-host); q+= p-host;
	    if(bits!=6||flow!=0||irmode) { //追加あり
	      WCHAR buf2[10];
	      GetDlgItemText(w, IDC_BAUD, buf2, _MAX_PATH);
	      int baud= _wtol(buf2);
	      if(baud==0) baud= 9600;
	      q+= wsprintf(q, L":%d", baud);
	    };
	  };
	  int cf= 0;
	  if(bits!=6||flow!=0||irmode) { //追加
	    *q++= ':'; cf= 1;
            if(bits<6) *q++= '7';
            if((bits/2)%3==1) *q++= 'e';
            if((bits/2)%3==2) *q++= 'o';
            if(bits&1) *q++= '2';
            if(flow==1) *q++= 'h';
            if(flow==2) *q++= 's';
	    if(irmode) *q++= 'i';
	  };
          for(;*p&&*p!=':';p++) {
	    switch(*p) {
	    case '7': case '8': case '2': case '1':
	    case 'n': case 'N': case 'e': case 'E': case 'o': case 'O':
	    case 'h': case 'H': case 'r': case 'R': case 's': case 'S': case 'x': case 'X':
	    case 'i': case 'I':
	      break;
	    default:
	      if(cf==0) { *q++= ':'; cf= 1; };
	      *q++= *p;
	    };
	  };
	  *q= 0;
	  if(*p) {
	    if(cf==0) *q++= ':';
	    wcscpy(q, p);
	  };
          SetDlgItemText(w, IDC_HOST, buf);
	};
      };
    };
    break;
  };
  return(FALSE);
};

void Config::
arrowdraw(HDC dc, int mode)
{
  HBRUSH br;
  RECT r;
  if(mode==0) {
    br= GetSysColorBrush(COLOR_3DSHADOW);
    r.left= 0; r.top= 0; r.right= 19; r.bottom= 1; FillRect(dc, &r, br);
    r.left= 0; r.top= 1; r.right= 1; r.bottom= 18; FillRect(dc, &r, br);
    r.left= 18; r.top= 1; r.right= 19; r.bottom= 18; FillRect(dc, &r, br);
    r.left= 0; r.top= 18; r.right= 19; r.bottom= 19; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= 19; r.top= 0; r.right= 20; r.bottom= 19; FillRect(dc, &r, br);
    r.left= 0; r.top= 19; r.right= 20; r.bottom= 20; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= 1; r.top= 1; r.right= 18; r.bottom= 2; FillRect(dc, &r, br);
    r.left= 1; r.top= 2; r.right= 2; r.bottom= 18; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= 2; r.top= 2; r.right= 18; r.bottom= 18; FillRect(dc, &r, br);
  } else {
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= 0; r.top= 0; r.right= 20; r.bottom= 2; FillRect(dc, &r, br);
    r.left= 0; r.top= 2; r.right= 2; r.bottom= 20; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= 19; r.top= 1; r.right= 20; r.bottom= 19; FillRect(dc, &r, br);
    r.left= 1; r.top= 19; r.right= 20; r.bottom= 20; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= 2; r.top= 2; r.right= 19; r.bottom= 19; FillRect(dc, &r, br);
  };
  HICON mi= (HICON)LoadImage(theInst, MAKEINTRESOURCE(IDI_ARROW), IMAGE_ICON, 16, 16, 0);
  DrawIcon(dc, 2+mode, 2+mode, mi);
  DestroyIcon(mi);
};

void Config::
arrowdrop(HWND w)
{
  HDC dc= GetDC(w);
  arrowdraw(dc, 1);
  ReleaseDC(w, dc);
  HMENU mm= LoadMenu(theInst, MAKEINTRESOURCE(IDR_COLOR_S));
  RECT r;
  GetWindowRect(w, &r);
  int act= TrackPopupMenu(GetSubMenu(mm,0), 0, r.right, r.top, 0, GetParent(w), 0);
  DestroyMenu(mm);
  dc= GetDC(w);
  arrowdraw(dc, 0);
  ReleaseDC(w, dc);
  MSG umsg;
  while(PeekMessage(&umsg, w, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE)) ;
};

LRESULT CALLBACK Config::
arrowwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.arrowproc(w, msg, wp, lp);
};

LRESULT Config::
arrowproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc= BeginPaint(w, &ps);
      arrowdraw(dc, 0);
      EndPaint(w, &ps);      
    };
    return(0);
  case WM_LBUTTONDOWN:
    arrowdrop(w);
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

void Config::
colordraw(HDC dc)
{
  RECT r;
  HBRUSH br;
  long col;
  HBRUSH bbr= (HBRUSH)GetStockObject(BLACK_BRUSH);
  HBRUSH cbr;
  int x, y;
  for(int i= 0; i<11; i++) {
    if(i==8) { //uline
      y= 93; x= 4;
      col= coluline;
      if(Draw::devcolor==8) br= CreateSolidBrush(0x0100001a);	  
    } else if(i==9) { //back
      y= 8; x= 4;
      col= colback;
      if(Draw::devcolor==8) br= CreateSolidBrush(0x0100001b);	  
    } else if(i==10) { //back2
      y= 25; x= 4;
      br= GetSysColorBrush(COLOR_STATIC);
    } else {
      y= i*17+8; x= 56;
      col= color[i];
      if(Draw::devcolor==8) br= CreateSolidBrush(0x01000000|0x0a+i);
    };
    if(Draw::devcolor!=8&&i!=10) {
      if(Draw::devcolor==2) {
	int colb= (colback>>16&255)*30+(colback>>8&255)*59+(colback&255)*11+50;
	int d= colb/6400;
        if(col!=colback) {
	  col= (col>>16&255)*30+(col>>8&255)*59+(col&255)*11+50;
	  if(col/6400!=d) d= col/6400;
	  else if(col>colb) d++; else d--;
	};
	if(d<0) d= 0; if(d>3) d= 3;
	br= CreateSolidBrush(0x01000000|d);
      } else {
        br= CreateSolidBrush(RGB(col>>16&255, col>>8&255, col));
      };
    };
    if(i==curcolor) {
      cbr= (HBRUSH)GetStockObject(BLACK_BRUSH);
    } else {
      cbr= GetSysColorBrush(COLOR_STATIC);
    };

    r.top= y+1; r.bottom= y+17;
    if(i!=8&&i!=9&&i-1==curcolor) r.top= y+3;
    if(i!=7&&i!=8&&i+1==curcolor) r.bottom= y+15;
    r.left= x+1; r.right= x+19; FillRect(dc, &r, br);
    r.left= x; r.right= x+1; if(i==10&&currand==0) FillRect(dc, &r, cbr); else FillRect(dc, &r, bbr);
    r.left= x+19; r.right= x+20; if(i==10&&currand==0) FillRect(dc, &r, cbr); else FillRect(dc, &r, bbr);
    if(i==10&&currand==1) {
      r.top= y+8; r.bottom= y+9; r.left= x+7; r.right= x+10; FillRect(dc, &r, bbr);
      r.left= x+13; r.right= x+14; FillRect(dc, &r, bbr);
      r.top= y+9; r.bottom= y+10; r.left= x+6; r.right= x+7; FillRect(dc, &r, bbr);
      r.left= x+10; r.right= x+13; FillRect(dc, &r, bbr);
    };
    r.left= x; r.right= x+20;
    r.top= y; r.bottom= y+1; FillRect(dc, &r, bbr);
    r.top= y+17; r.bottom= y+18; if(i==10&&currand==0) FillRect(dc, &r, cbr); else FillRect(dc, &r, bbr);

    r.top= y; r.bottom= y+18;
    if(i!=8&&i!=9&&i-1==curcolor) r.top= y+3;
    if(i!=7&&i!=8&&i+1==curcolor) r.bottom= y+15;
    r.left= x-2; r.right= x; FillRect(dc, &r, cbr);
    r.left= x+20; r.right= x+22; FillRect(dc, &r, cbr);
    r.left= x-2; r.right= x+22;
    if(i==curcolor||i==0||i==8||i==9) { r.top= y-2; r.bottom= y; FillRect(dc, &r, cbr); };
    if(i==curcolor||i==7||i==8||i==10) { r.top= y+18; r.bottom= y+20; FillRect(dc, &r, cbr); };
    if(i!=10) DeleteObject(br);
  };
};

void Config::
colorget(HWND w)
{
  if(w==0) return;
  if(SendDlgItemMessage(w, IDC_CHECKRND, BM_GETCHECK, 0, 0)==BST_UNCHECKED) {
    colback2= colback1;
  };
};

void Config::
colorset(HWND w)
{
  if(w==0) return;
  colorcapt= 0;
  if(colback1==colback2) {
    currand= 0;
    ShowWindow(GetDlgItem(w, IDC_STATICB2), SW_HIDE);
    SendDlgItemMessage(w, IDC_CHECKRND, BM_SETCHECK, BST_UNCHECKED, 0);
    if(curcolor==10) curcolor= 9;
  } else {
    currand= 1;
    ShowWindow(GetDlgItem(w, IDC_STATICB2), SW_SHOW);
    SendDlgItemMessage(w, IDC_CHECKRND, BM_SETCHECK, BST_CHECKED, 0);
  };
  int col;
  if(curcolor==8) col= coluline;
  else if(curcolor==9) col= colback1;
  else if(curcolor==10) col= colback2;
  else col= color[curcolor];
  WCHAR buf[10];
  wsprintf(buf, L"%06X", col);
  colorcapt= 1;
  SetDlgItemText(w, IDC_EDITRGB, buf);
  colorcapt= 0;
  // InvalidateRect(w, 0, 0);
  HDC dc= GetDC(w);
  colordraw(dc);
  ReleaseDC(w, dc);
};

void Config::
colorpalette()
{
  if(Draw::devcolor==8) {
    for(int i= 0; i<8; i++) ((Draw8*)theDraw)->palset(i+0x0a, color[i]);
    ((Draw8*)theDraw)->palset(0x1a, coluline);
    ((Draw8*)theDraw)->palset(0x1b, colback);
    SetPaletteEntries(((Draw8*)theDraw)->palette, 0x0a, 18
    , &((Draw8*)theDraw)->logpal->palPalEntry[0x0a]);
    ((Draw8*)theDraw)->palset(0x1c+screencur*2, colback);
    ((Draw8*)theDraw)->palset(0x1d+screencur*2, color[0]);
    SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1c+screencur*2, 2
    , &((Draw8*)theDraw)->logpal->palPalEntry[0x1c+screencur*2]);
    disppalette();
  };
};

BOOL CALLBACK Config::
colorwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.colorproc(w, msg, wp, lp);
};

BOOL Config::
colorproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    SCROLLINFO si;
    si.cbSize= sizeof(SCROLLINFO);
    si.fMask= SIF_POS|SIF_PAGE|SIF_RANGE;
    si.nMin= 0;
    si.nMax= 255+47;
    si.nPage= 48;
    si.nPos= 128;
    SendDlgItemMessage(w, IDC_SLIDERR, TBM_SETRANGE, 0, MAKELONG(0, 255));
    SendDlgItemMessage(w, IDC_SLIDERR, TBM_SETPAGESIZE, 0, 17);
    SendDlgItemMessage(w, IDC_SLIDERR, TBM_SETPOS, 1, 128);
    SendDlgItemMessage(w, IDC_SLIDERG, TBM_SETRANGE, 0, MAKELONG(0, 255));
    SendDlgItemMessage(w, IDC_SLIDERG, TBM_SETPAGESIZE, 0, 17);
    SendDlgItemMessage(w, IDC_SLIDERG, TBM_SETPOS, 1, 128);
    SendDlgItemMessage(w, IDC_SLIDERB, TBM_SETRANGE, 0, MAKELONG(0, 255));
    SendDlgItemMessage(w, IDC_SLIDERB, TBM_SETPAGESIZE, 0, 17);
    SendDlgItemMessage(w, IDC_SLIDERB, TBM_SETPOS, 1, 128);
    SendDlgItemMessage(w, IDC_EDITRGB, EM_SETLIMITTEXT, 6, 0);
    warrow= CreateWindowEx(WS_EX_NOACTIVATE
    , L"24arrow", L"arrow"
    , WS_CHILD|WS_VISIBLE
    , 27, 125, 20, 20, w, 0, theInst, 0);
    curcolor= 9;
    colorset(w);
    return(TRUE);
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_VSCROLL:
    {
      int pos= 255-SendMessage((HWND)lp, TBM_GETPOS, 0, 0);
      WCHAR buf[10];
      GetDlgItemText(w, IDC_EDITRGB, buf, 10);
      int v= wcstol(buf, 0, 16);
      if(v<0) v= 0;
      if(v>0xffffff) v= 0xffffff;
      int sv= v;
      switch(GetDlgCtrlID((HWND)lp)) {
      case IDC_SLIDERR:
        sv= sv&0xffff|pos<<16;
	break;
      case IDC_SLIDERG:
        sv= sv&0xff00ff|pos<<8;
	break;
      case IDC_SLIDERB:
        sv= sv&0xffff00|pos;
	break;
      };
      wsprintf(buf, L"%06X", sv);
      if(sv!=v) {
        colorcapt= 2;
	SetDlgItemText(w, IDC_EDITRGB, buf);
	colorcapt= 0;
      };
      if(LOWORD(wp)==SB_ENDSCROLL&&Draw::devcolor!=8) {
        HDC dc= GetDC(wtitle);
	titledraw(dc);
        ReleaseDC(wtitle, dc);
        theDraw->init(this, wdisp);
	theDraw->redrawwin(theScreen, wdisp);
      };
    };
    return(0);
  case WM_COMMAND:
    {
      if(LOWORD(wp)==IDC_EDITRGB) {
        if(HIWORD(wp)==EN_CHANGE) {
	  WCHAR buf[10];
	  GetWindowText((HWND)lp, buf, 10);
	  int v= wcstol(buf, 0, 16);
	  if(v<0) v= 0;
	  if(v>0xffffff) v= 0xffffff;
	  if(colorcapt!=2) {
	    int sv=
	    ( SendDlgItemMessage(w, IDC_SLIDERR, TBM_GETPOS, 0, 0)<<16
	    | SendDlgItemMessage(w, IDC_SLIDERG, TBM_GETPOS, 0, 0)<<8
	    | SendDlgItemMessage(w, IDC_SLIDERB, TBM_GETPOS, 0, 0) )^0xffffff;
	    if(v!=sv) {
	      SendDlgItemMessage(w, IDC_SLIDERR, TBM_SETPOS, 1, v>>16^255);
	      SendDlgItemMessage(w, IDC_SLIDERG, TBM_SETPOS, 1, v>>8&255^255);
	      SendDlgItemMessage(w, IDC_SLIDERB, TBM_SETPOS, 1, v&255^255);
	    };
	  };
	  int *colp= 0;
	  int pal;
	  if(curcolor==8) colp= &coluline, pal= 0x1a;
	  else if(curcolor==9) colp= &colback1, pal= 0x1b;
	  else if(curcolor==10) colp= &colback2, pal= 0x1b;
	  else if(curcolor<8) colp= &color[curcolor], pal= 0x0a+curcolor;
	  if(colorcapt!=1 && colp && *colp!=v) {
	    *colp= v;
	    int rtitle= 0;
	    int rdisp= 0;
	    if(pal==0x1b) {
	      if(SendDlgItemMessage(w, IDC_CHECKRND, BM_GETCHECK, 0, 0)==BST_CHECKED) {
	        setrandom();
	      } else colback= colback1;
	      v= colback;
	      if(Draw::devcolor==8) { //タブ色セット
	        ((Draw8*)theDraw)->palset(0x1c+screencur*2, v);
	        SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1c+screencur*2, 1
	        , &((Draw8*)theDraw)->logpal->palPalEntry[0x1c+screencur*2]);
	      } else if(colorcapt==0) rtitle= 1;
	    };
	    if(curcolor==0) {
	      if(Draw::devcolor==8) { //タブ色セット
	        ((Draw8*)theDraw)->palset(0x1d+screencur*2, v);
	        SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1d+screencur*2, 1
	        , &((Draw8*)theDraw)->logpal->palPalEntry[0x1d+screencur*2]);
	      } else if(colorcapt==0) rtitle= 1;
	    };
	    if(curcolor==9) { //colback1 change for black
	      int b= 0;
	      if((v>>16&255)*30+(v>>8&255)*59+(v&255)*11<12800) b= 1;
	      if(black!=b) {
	        black= b;
		if(Draw::devcolor==8||colorcapt==0) {
	          rtitle= 1; rdisp= 1;
		};
	      };
	    };
	    if(Draw::devcolor==8) {
	      ((Draw8*)theDraw)->palset(pal, v);
	      SetPaletteEntries(((Draw8*)theDraw)->palette, pal, 1
	      , &((Draw8*)theDraw)->logpal->palPalEntry[pal]);
	      disppalette(); //realize palette
	    } else {
              HDC dc= GetDC(w);
              colordraw(dc);
              ReleaseDC(w, dc);
	      if(colorcapt==0) rdisp= 1;
	    };
 	    if(rtitle) {
              HDC dc= GetDC(wtitle);
	      titledraw(dc);
              ReleaseDC(wtitle, dc);
	    };
	    if(rdisp) {
              theDraw->init(this, wdisp);
	      theDraw->redrawwin(theScreen, wdisp);
	    };
	  };
	  colorcapt= 0;
	};
	return(0);
      };
      if(LOWORD(wp)==IDC_CHECKRND) {
        if(HIWORD(wp)==BN_CLICKED) {
	  if(SendDlgItemMessage(w, IDC_CHECKRND, BM_GETCHECK, 0, 0)==BST_CHECKED) {
	    currand= 1;
            ShowWindow(GetDlgItem(w, IDC_STATICB2), SW_SHOW);
	    setrandom();
	  } else {
	    currand= 0;
            ShowWindow(GetDlgItem(w, IDC_STATICB2), SW_HIDE);
	    if(curcolor==10) {
	      curcolor= 9;
	      WCHAR buf[10];
	      wsprintf(buf, L"%06X", colback1);
	      colorcapt= 1;
	      SetDlgItemText(w, IDC_EDITRGB, buf);
	      colorcapt= 0;
	    };
	    colback= colback1;
	  };
          HDC dc= GetDC(w);
          colordraw(dc);
          ReleaseDC(w, dc);
	  if(Draw::devcolor==8) {
	    ((Draw8*)theDraw)->palset(0x1c+screencur*2, colback);
	    SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1c+screencur*2, 1
	    , &((Draw8*)theDraw)->logpal->palPalEntry[0x1c+screencur*2]);
	    ((Draw8*)theDraw)->palset(0x1b, colback);
	    SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1b, 1
	    , &((Draw8*)theDraw)->logpal->palPalEntry[0x1b]);
	    disppalette();
	  } else {
	    HDC dc= GetDC(wtitle);
	    titledraw(dc);
	    ReleaseDC(wtitle, dc);
	    theDraw->init(this, wdisp);
	    theDraw->redrawwin(theScreen, wdisp);
	  };
	};
	return(0);
      };
      if(LOWORD(wp)==IDM_COLOR_W||LOWORD(wp)==IDM_COLOR_B) {
        if(LOWORD(wp)==IDM_COLOR_W) {
           color[0]= 0x000000;
           color[1]= 0xcc0000;
           color[2]= 0x00aa00;
           color[3]= 0x888800;
           color[4]= 0x0000ff;
           color[5]= 0xcc00cc;
           color[6]= 0x008888;
           color[7]= 0x666666;
           coluline= 0xff7777;
           colback1= colback2= colback= 0xffffff;
	   black= 0;
	} else {
           color[0]= 0xffffff;
           color[1]= 0xff0000;
           color[2]= 0x00ff00;
           color[3]= 0xffff00;
           color[4]= 0x4444ff;
           color[5]= 0xff00ff;
           color[6]= 0x00ffff;
           color[7]= 0x999999;
           coluline= 0xff7777;
           colback1= colback2= colback= 0x000000;
	   black= 1;
	};
	colorset(w);
        theDraw->init(this, wdisp);
        colorpalette();
        HDC dc= GetDC(wtitle);
        titledraw(dc);
        ReleaseDC(wtitle, dc);
        theDraw->redrawwin(theScreen, wdisp);
	return(0);
      };
    };
    break;
  case WM_LBUTTONDOWN:
    {
      int cx, cy;
      cx= LOWORD(lp); cy= HIWORD(lp);
      int sel= 0;
      if(cx>=2&&cx<54) {
        if(cy>=6&&(cy<25||currand==0&&cy<28)) sel= 1, curcolor= 9;
	if(currand==1&&(cy>=25&&cy<45)) sel= 1, curcolor= 10;
	if(cy>=91&&cy<113) sel= 1, curcolor= 8;
      } else if(cx>=54&&cx<106&&cy>=6&&cy<147) {
        sel= 1, curcolor= (cy-8)/17;
	if(curcolor<0) curcolor= 0;
	if(curcolor>7) curcolor= 7;
      };
      if(sel) {
        HDC dc= GetDC(w);
        colordraw(dc);
        ReleaseDC(w, dc);
	int col;
	if(curcolor==8) col= coluline;
	else if(curcolor==9) col= colback1;
	else if(curcolor==10) col= colback2;
	else col= color[curcolor];
	WCHAR buf[10];
	wsprintf(buf, L"%06X", col);
	colorcapt= 1;
	SetDlgItemText(w, IDC_EDITRGB, buf);
	colorcapt= 0;
      };
    };
    break;
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc= BeginPaint(w, &ps);
      colordraw(dc);
      EndPaint(w, &ps);
    };
    return(0);
  };
  return(FALSE);
};

void Config::
gammadraw(HDC dc)
{
  RECT r;
  r.left= 0; r.top= 0; r.right= 158; r.bottom= 30;
  FillRect(dc, &r, GetSysColorBrush(COLOR_STATIC));
  HBRUSH br= (HBRUSH)GetStockObject(BLACK_BRUSH);
  r.left= 15; r.top= 15; r.right= 143; r.bottom= 16;
  FillRect(dc, &r, br);
  r.left= 15; r.top= 11; r.right= 16; r.bottom= 20;
  FillRect(dc, &r, br);
  r.left= 142; r.top= 11; r.right= 143; r.bottom= 20;
  FillRect(dc, &r, br);
  HICON mi= (HICON)LoadImage(theInst, MAKEINTRESOURCE(IDI_GAMMALH), IMAGE_ICON, 16, 16, 0);
  DrawIcon(dc, gammalow/2+7, 7, mi);
  DrawIcon(dc, gammahigh/2+7, 7, mi);
  DestroyIcon(mi);
  mi= (HICON)LoadImage(theInst, MAKEINTRESOURCE(IDI_GAMMAM), IMAGE_ICON, 16, 16, 0);
  DrawIcon(dc, gammamid/2+7, 7, mi);
  DestroyIcon(mi);
};

void Config::
gammaset(HWND w)
{
  HDC dc= GetDC(w);
  gammadraw(dc);
  ReleaseDC(w, dc);

  WCHAR buf[20];
  swprintf(buf, L"%d,%d,%d", gammalow, gammamid, gammahigh);
  SetDlgItemText(wmisc, IDC_GAMMA, buf);
  UpdateWindow(GetDlgItem(wmisc, IDC_GAMMA));

  if(theDraw->picwidth&&Draw::devcolor==8) {
    gammadisp();
  };
};

void Config::
gammadisp()
{
  if(theDraw->picwidth) {
    if(Draw::devcolor==8) {
      int pn= ((Pic8*)theDraw)->picpaln;
      for(int i= 0; i<pn; i++) {
        ((Pic8*)theDraw)->palset(i+0x30
        , gammacolor(((Pic8*)theDraw)->picpal[i]));
      };
      ((Pic8*)theDraw)->palset(0x2e, gammacolor(0x000000));
      ((Pic8*)theDraw)->palset(0x2f, gammacolor(0xffffff));
      SetPaletteEntries(((Pic8*)theDraw)->palette, 0x2e, pn+2
      , &((Pic8*)theDraw)->logpal->palPalEntry[0x2e]);
      disppalette();
    } else { //not palette
      theDraw->init(this, wdisp);
      theDraw->redrawwin(theScreen, wdisp);
    };
  };
};

LRESULT CALLBACK Config::
gammawrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.gammaproc(w, msg, wp, lp);
};

LRESULT Config::
gammaproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc= BeginPaint(w, &ps);
      gammadraw(dc);
      EndPaint(w, &ps);      
    };
    return(0);
  case WM_LBUTTONDOWN:
    {
      SetCapture(w);
      gammacaptx= ((short)LOWORD(lp)-15)*2;
      if((short)HIWORD(lp)>15) { //mid-slider
        gammacapt= 1;
        int x= 0;
        if(gammacaptx<gammamid) {
          if(gammamid>gammalow) x= -1;
        } else {
          if(gammamid<gammahigh) x= 1;
	};
	if(x) {
	  gammamid+= x;
	  gammarate= (double)(gammamid-gammalow)/(gammahigh-gammalow);
	  gammaset(w);
	};
      } else if(gammacaptx*2<gammalow+gammahigh) { //gammalow
        gammacapt= 2;
        int x= 0;
        if(gammacaptx<gammalow) {
          if(gammalow>0) x= -1;
        } else {
          if(gammalow<gammahigh) x= 1;
	};
	if(x) {
	  gammalow+= x;
	  gammamid= (int)((gammahigh-gammalow)*gammarate+0.5)+gammalow;
	  gammaset(w);
	};
      } else { //gammahigh
        gammacapt= 3;
        int x= 0;
        if(gammacaptx<gammahigh) {
          if(gammalow<gammahigh) x= -1;
        } else {
          if(gammahigh<255) x= 1;
	};
	if(x) {
	  gammahigh+= x;
	  gammamid= (int)((gammahigh-gammalow)*gammarate+0.5)+gammalow;
	  gammaset(w);
	};
      };
    };
    return(0);

  case WM_LBUTTONUP:
  case WM_MOUSEMOVE:
    {
      int x= ((short)LOWORD(lp)-15)*2;
      if((gammacaptx-x)>5||(gammacaptx-x)<-5) {
        gammacaptx= 9999;
	if(gammacapt==1) {
	  if(x<gammalow) x= gammalow;
	  if(x>gammahigh) x= gammahigh;
	  if(x!=gammamid) {
	    gammamid= x;
	    gammarate= (double)(gammamid-gammalow)/(gammahigh-gammalow);
	    gammaset(w);
	  };
	} else if(gammacapt==2) {
	  if(x<0) x= 0;
	  if(x>gammahigh) x= gammahigh;
	  if(x!=gammalow) {
	    gammalow= x;
	    gammamid= (int)((gammahigh-gammalow)*gammarate+0.5)+gammalow;
	    gammaset(w);
	  };
	} else if(gammacapt==3) {
	  if(x<gammalow) x= gammalow;
	  if(x>255) x= 255;
	  if(x!=gammahigh) {
	    gammahigh= x;
	    gammamid= (int)((gammahigh-gammalow)*gammarate+0.5)+gammalow;
	    gammaset(w);
	  };
	};
      };
      if(msg==WM_LBUTTONUP) {
        ReleaseCapture();
	gammadisp();
	gammacapt= 0;
      };
    };
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

void Config::
miscget(HWND w)
{
  if(w==0) return;
  if(SendDlgItemMessage(w, IDC_PIC, BM_GETCHECK, 0, 0)==BST_UNCHECKED) pic[0]= 0;
  sjis= SendDlgItemMessage(w, IDC_SJIS, CB_GETCURSEL, 0, 0);
  WCHAR buf[20];
  GetDlgItemText(w, IDC_SCROLL, buf, numof(buf));
  ymax= _wtol(buf)+24;
  if(ymax<24) ymax= 24;
  if(ymax>1024) ymax= 1024;
  enter= SendDlgItemMessage(w, IDC_ENTER, CB_GETCURSEL, 0, 0);
  newline= SendDlgItemMessage(w, IDC_NEWLINE, CB_GETCURSEL, 0, 0);
  yen= 1;
  if(SendDlgItemMessage(w, IDC_YEN, BM_GETCHECK, 0, 0)==BST_UNCHECKED) yen= 0;
  nobold= 0;
  if(SendDlgItemMessage(w, IDC_BOLD, BM_GETCHECK, 0, 0)==BST_UNCHECKED) nobold= 1;
  stay= 0;
  if(SendDlgItemMessage(w, IDC_STAY, BM_GETCHECK, 0, 0)==BST_UNCHECKED) stay= 1;
  bsdel= 0;
  if(SendDlgItemMessage(w, IDC_BSDEL, BM_GETCHECK, 0, 0)==BST_CHECKED) bsdel= 1;
  echo= 0;
  if(SendDlgItemMessage(w, IDC_ECHO, BM_GETCHECK, 0, 0)==BST_CHECKED) echo= 1;
};

void Config::
miscgset(HWND w)
{
  if(w==0) return;

  if(gammalow>=gammahigh) {
    gammarate= (double)1/2;
  } else {
    gammarate= (double)(gammamid-gammalow)/(gammahigh-gammalow);
  };
  gammacapt= 0;
  gammacaptx= 9999;

  WCHAR buf[20];
  swprintf(buf, L"%d,%d,%d", gammalow, gammamid, gammahigh);
  SetDlgItemText(w, IDC_GAMMA, buf);
  SendDlgItemMessage(w, IDC_PIC, BM_SETCHECK, pic[0]?BST_CHECKED:BST_UNCHECKED, 0);
  HDC dc= GetDC(wgamma);
  gammadraw(dc);
  ReleaseDC(wgamma, dc);
};

void Config::
miscset(HWND w)
{
  if(w==0) return;

  WCHAR buf[20];
  swprintf(buf, L"%d", ymax-24);
  SetDlgItemText(w, IDC_SCROLL, buf);
  SendDlgItemMessage(w, IDC_SJIS, CB_SETCURSEL, sjis, 0);
  SendDlgItemMessage(w, IDC_ENTER, CB_SETCURSEL, enter, 0);
  SendDlgItemMessage(w, IDC_NEWLINE, CB_SETCURSEL, newline, 0);
  SendDlgItemMessage(w, IDC_YEN, BM_SETCHECK, yen==1?BST_CHECKED:BST_UNCHECKED, 0);
  SendDlgItemMessage(w, IDC_BOLD, BM_SETCHECK, nobold==0?BST_CHECKED:BST_UNCHECKED, 0);
  SendDlgItemMessage(w, IDC_STAY, BM_SETCHECK, stay==0?BST_CHECKED:BST_UNCHECKED, 0);
  SendDlgItemMessage(w, IDC_BSDEL, BM_SETCHECK, bsdel==1?BST_CHECKED:BST_UNCHECKED, 0);
  SendDlgItemMessage(w, IDC_ECHO, BM_SETCHECK, echo==1?BST_CHECKED:BST_UNCHECKED, 0);
};

BOOL CALLBACK Config::
miscwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.miscproc(w, msg, wp, lp);
};

BOOL Config::
miscproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    {
      wgamma= CreateWindowEx(WS_EX_NOACTIVATE
      , L"24gamma", L"gamma"
      , WS_CHILD|WS_VISIBLE
      , 4, 27, 158, 30, w, 0, theInst, 0);
      SendDlgItemMessage(w, IDC_SJIS, CB_ADDSTRING, 0, (LPARAM)L"EUC");
      SendDlgItemMessage(w, IDC_SJIS, CB_ADDSTRING, 0, (LPARAM)L"SJIS");
      SendDlgItemMessage(w, IDC_SJIS, CB_ADDSTRING, 0, (LPARAM)L"JIS");
      setcomboheight(GetDlgItem(w, IDC_SJIS));
      SendDlgItemMessage(w, IDC_EDITRGB, EM_SETLIMITTEXT, 4, 0);
      SendDlgItemMessage(w, IDC_ENTER, CB_ADDSTRING, 0, (LPARAM)L"CRLF");
      SendDlgItemMessage(w, IDC_ENTER, CB_ADDSTRING, 0, (LPARAM)L"CR");
      SendDlgItemMessage(w, IDC_ENTER, CB_ADDSTRING, 0, (LPARAM)L"LF");
      setcomboheight(GetDlgItem(w, IDC_ENTER));
      SendDlgItemMessage(w, IDC_NEWLINE, CB_ADDSTRING, 0, (LPARAM)L"CRLF");
      SendDlgItemMessage(w, IDC_NEWLINE, CB_ADDSTRING, 0, (LPARAM)L"CR");
      SendDlgItemMessage(w, IDC_NEWLINE, CB_ADDSTRING, 0, (LPARAM)L"LF");
      setcomboheight(GetDlgItem(w, IDC_NEWLINE));
      miscgset(w);
      miscset(w);
    };
    return(TRUE);
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_COMMAND:
    {
      if(LOWORD(wp)==IDC_BITMAP) {
        if(HIWORD(wp)==BN_CLICKED) {
	  WCHAR dir[_MAX_PATH];
	  if(pic[0]!='\\'&&file[0]=='\\') {
            int a= wcsrchr(file, '\\')-file+1;
            wcsncpy(dir, file, a);
	    wcscpy(dir+a, pic);
	  } else {
	    wcscpy(dir, pic);
	  };
          WCHAR fname[_MAX_PATH], *p;
	  p= wcsrchr(dir, '\\');
	  if(p) {
	    wcscpy(fname, p+1);
	    *p= 0;
	  } else {
	    wcscpy(fname, dir);
	    dir[0]= 0;
	  };
          OPENFILENAME fns;
          memset((void*)&fns, 0, sizeof(fns));
          fns.lStructSize= sizeof(fns);
          fns.hwndOwner= w;
          fns.lpstrFilter= L"Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
          fns.nFilterIndex= 1;
          fns.lpstrFile= fname;
          fns.nMaxFile= numof(fname);
	  fns.lpstrInitialDir= dir;
          fns.lpstrTitle= L"Background Picture";
          fns.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
          HWND wback= wmodal;
	  wmodal= (HWND)1;
	  EnableWindow(wcmenu, 0);
          int rt= GetOpenFileName(&fns);
	  wmodal= wback;
	  EnableWindow(wcmenu, 1);
          if(rt) {
	    wcscpy(pic, fname);
	    if(checkpic()[0]) {
              SendDlgItemMessage(w, IDC_PIC, BM_SETCHECK, BST_UNCHECKED, 0);
	    } else {
	      if(Draw::devcolor!=2) {
	        delete theDraw;
	        drawnew();
                disppalette();
                theDraw->redrawwin(theScreen, wdisp);
	      };
              SendDlgItemMessage(w, IDC_PIC, BM_SETCHECK, BST_CHECKED, 0);
	    };
	  };
          return(0);
        };
      } else if(LOWORD(wp)==IDC_PIC) {
        if(HIWORD(wp)==BN_CLICKED) {
	  if(SendDlgItemMessage(w, IDC_PIC, BM_GETCHECK, 0, 0)==BST_CHECKED) {
	    if(pic[0]&&Draw::devcolor!=2) {
	      delete theDraw;
	      drawnew();
              disppalette();
              theDraw->redrawwin(theScreen, wdisp);
	    };
	  } else {
	    if(pic[0]&&theDraw->picwidth) {
	      delete theDraw;
              if(Draw::devcolor==8) {
                theDraw= new Draw8;
              } else if(Draw::devcolor>=15) {
                theDraw= new Draw16;
              };
              theDraw->init(this, wdisp);
              theDraw->redrawwin(theScreen, wdisp);
              disppalette();
	    };
	  };
	};
      };
    };
    break;
  };
  return(FALSE);
};

void Config::
cmenudrop()
{
  if(wmodal!=wconfig) return;
  extern void menudraw(HDC, int);
  HDC dc= GetDC(wcmenu);
  menudraw(dc, 1);
  ReleaseDC(wcmenu, dc);
  HMENU mm= LoadMenu(theInst, MAKEINTRESOURCE(IDR_CONFIG));
  HMENU md= GetSubMenu(mm, 0);

  HKEY reg;
  if(RegOpenKeyEx(HKEY_CLASSES_ROOT, L"24termconfig\\Shell\\Open\\Command", 0, 0, &reg)==0) {
    WCHAR buf[_MAX_PATH];
    DWORD dd, dt;
    dd= sizeof(buf);
    int rt= RegQueryValueEx(reg, L"", 0, &dt, (BYTE*)buf, &dd);
    RegCloseKey(reg);
    if(rt==0) {
      WCHAR cbuf[_MAX_PATH];
      cbuf[0]= '"';
      rt= GetModuleFileName(0, cbuf+1, _MAX_PATH-8);
      cbuf[rt+1]= '"';
      if(wcsncmp(buf, cbuf, rt+2)==0) DeleteMenu(md, IDM_ASSOC, 0);
    };
  };
  long act= TrackPopupMenu(md, 0, 21, 0, 0, wconfig, 0);
  DestroyMenu(mm);
  dc= GetDC(wcmenu);
  menudraw(dc, 0);
  ReleaseDC(wcmenu, dc);
  MSG umsg;
  while(PeekMessage(&umsg, wcmenu, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE)) ;
};

LRESULT CALLBACK Config::
cmenuwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.cmenuproc(w, msg, wp, lp);
};

LRESULT Config::
cmenuproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  extern void menudraw(HDC, int);
  switch(msg) {
  case WM_QUERYNEWPALETTE:
    return(TRUE);
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc= BeginPaint(w, &ps);
      menudraw(dc, 0);
      EndPaint(w, &ps);
    };
    return(0);
  case WM_LBUTTONDOWN:
    cmenudrop();
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

BOOL CALLBACK Config::
configwrap(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
  return theScreen->config.configproc(w, msg, wp, lp);
};

BOOL Config::
configproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    {
      wconfig= w;
      wmodal= w;
      theScreen->configback= *this; //色保持
      if(cert) {
        theScreen->configback.cert= (BYTE*)malloc(certalloc);
	memcpy(theScreen->configback.cert, cert, certlen);
      };
      WNDCLASS wc;
      wc.style= 0;
      wc.lpfnWndProc= (WNDPROC)arrowwrap;
      wc.cbClsExtra= 0; wc.cbWndExtra= 0;
      wc.hInstance= theInst;
      wc.hIcon= 0; wc.hCursor= 0;
      wc.hbrBackground= (HBRUSH)(COLOR_BTNFACE+1);
      wc.lpszMenuName= 0;
      wc.lpszClassName= L"24arrow";
      RegisterClass(&wc);
      wc.style= 0;
      wc.lpfnWndProc= (WNDPROC)gammawrap;
      wc.cbClsExtra= 0; wc.cbWndExtra= 0;
      wc.hInstance= theInst;
      wc.hIcon= 0; wc.hCursor= 0;
      wc.hbrBackground= (HBRUSH)(COLOR_BTNFACE+1);
      wc.lpszMenuName= 0;
      wc.lpszClassName= L"24gamma";
      RegisterClass(&wc);
      wc.style= 0;
      wc.lpfnWndProc= (WNDPROC)cmenuwrap;
      wc.cbClsExtra= 0; wc.cbWndExtra= 0;
      wc.hInstance= theInst;
      wc.hIcon= 0; wc.hCursor= 0;
      wc.hbrBackground= (HBRUSH)(COLOR_BTNFACE+1);
      wc.lpszMenuName= 0;
      wc.lpszClassName= L"24cmenu";
      RegisterClass(&wc);
      SetWindowText(w, theScreen->config.title);

      wcmenu= CreateWindowEx(WS_EX_NOACTIVATE
      , L"24cmenu", L"cmenu"
      , WS_POPUP|WS_VISIBLE
      , 0, 0, 21, 22, w, 0, theInst, 0);

      RECT r;
      GetClientRect(w, &r);
      wtab= CreateWindowEx(0
      , WC_TABCONTROL, L""
      , WS_CHILD|WS_VISIBLE|WS_TABSTOP
      , 0, 0, r.right, r.bottom, w, 0, theInst, 0);
      TCITEM ti;
      ti.mask= TCIF_TEXT;
      ti.iImage= -1;
      ti.pszText= L"Host";
      SendMessage(wtab, TCM_INSERTITEM, 0, (LPARAM)&ti);
      ti.pszText= L"Color";
      SendMessage(wtab, TCM_INSERTITEM, 1, (LPARAM)&ti);
      ti.pszText= L"Misc";
      SendMessage(wtab, TCM_INSERTITEM, 2, (LPARAM)&ti);
      whost= CreateDialogIndirect(theInst
      , (DLGTEMPLATE*)LockResource(LoadResource(theInst
      , FindResource(theInst, MAKEINTRESOURCE(IDD_HOST), RT_DIALOG))), w, hostwrap);
      wcolor= 0;
      wmisc= 0;
      drect.left= 0; drect.top= 0; drect.right= 1; drect.bottom= 1;
      SendMessage(wtab, TCM_ADJUSTRECT, FALSE, (LPARAM)&drect);
      drect.left-= 1;
      SetWindowPos(whost, 0, drect.left, drect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
      //SetWindowPos(wmisc, 0, r.left, r.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
      SetWindowPos(whost, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
    };
    return(TRUE);
  case WM_NOTIFY:
    if(((NMHDR*)lp)->hwndFrom==wtab&&((NMHDR*)lp)->code==TCN_SELCHANGE) {
      int s= SendMessage(wtab, TCM_GETCURSEL, 0, 0);
      ShowWindow(whost, SW_HIDE);
      if(wcolor) ShowWindow(wcolor, SW_HIDE);
      if(wmisc) ShowWindow(wmisc, SW_HIDE);
      if(s==1) {
        if(wcolor==0) {
          wcolor= CreateDialogIndirect(theInst
          , (DLGTEMPLATE*)LockResource(LoadResource(theInst
          , FindResource(theInst, MAKEINTRESOURCE(IDD_COLOR), RT_DIALOG))), w, colorwrap);
          SetWindowPos(wcolor, 0, drect.left, drect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	};
        SetWindowPos(wcolor, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
      } else if(s==2) {
        if(wmisc==0) {
          wmisc= CreateDialogIndirect(theInst
          , (DLGTEMPLATE*)LockResource(LoadResource(theInst
          , FindResource(theInst, MAKEINTRESOURCE(IDD_MISC), RT_DIALOG))), w, miscwrap);
          SetWindowPos(wmisc, 0, drect.left, drect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	};
        SetWindowPos(wmisc, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
      } else {
        SetWindowPos(whost, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
      };
      return(0);
    };
    return(TRUE);
  case WM_COMMAND:
    switch(wp) {
    case IDOK:
      {
        if(theScreen->configback.cert) free(theScreen->configback.cert);
        hostget(whost);
        colorget(wcolor);
        miscget(wmisc);
        titlesetup();
        theDraw->redrawwin(theScreen, wdisp);
        EndDialog(w, TRUE);
      };
      return(TRUE);
    case IDCANCEL:
      {
        //色関連に変更あり？
	int flag= 0;
	if(wmisc&&SendDlgItemMessage(wmisc, IDC_PIC, BM_GETCHECK, 0, 0)==BST_UNCHECKED) pic[0]= 0;
	for(int i= 0; i<8; i++) if(theScreen->configback.color[i]!=color[i]) flag= 1;
	if(theScreen->configback.colback!=colback || theScreen->configback.colback1!=colback1 //白黒ベースの判断
	|| theScreen->configback.coluline!=coluline|| wcscmp(theScreen->configback.pic, pic)!=0) flag= 1;
	if(pic[0]&&(theScreen->configback.gammalow!=gammalow||theScreen->configback.gammamid!=gammamid
	||theScreen->configback.gammahigh!=gammahigh)) flag= 1;
        //recover things…他の変数使わないように注意
	if(cert) free(cert);
	*this= theScreen->configback;
        if(flag) {
	  delete theDraw;
	  drawnew();
          colorpalette();
          HDC dc= GetDC(wtitle);
          titledraw(dc);
          ReleaseDC(wtitle, dc);
          theDraw->redrawwin(theScreen, wdisp);
	};
        EndDialog(w, FALSE);
      };
      return(TRUE);
    case IDM_LOAD:
    case IDM_RESET:
      {
        int rt= 1;
	WCHAR fname[_MAX_PATH];
	fname[0]= 0;
        if(wp==IDM_LOAD) {
          WCHAR dir[_MAX_PATH], *p;
	  wcscpy(dir, file);
          p= wcsrchr(dir, '\\');
	  if(p) {
	    wcscpy(fname, p+1);
	    *p= 0;
	  } else {
	    wcscpy(fname, dir);
	    dir[0]= 0;
	  };
          OPENFILENAME fns;
          memset((void*)&fns, 0, sizeof(fns));
          fns.lStructSize= sizeof(fns);
          fns.hwndOwner= w;
          fns.lpstrFilter= L"Configs (*.24)\0*.24\0All Files (*.*)\0*.*\0";
          fns.nFilterIndex= 1;
          fns.lpstrFile= fname;
          fns.nMaxFile= numof(fname);
	  fns.lpstrInitialDir= dir;
          fns.lpstrTitle= L"Load Configuration";
          fns.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
	  HWND wback= wmodal;
	  wmodal= (HWND)1;
	  EnableWindow(wcmenu, 0);
          rt= GetOpenFileName(&fns);
	  wmodal= wback;
	  EnableWindow(wcmenu, 1);
	};
	if(rt) {
	  Config confignew= *this; //copy things
	  wcscpy(confignew.file, fname);
	  confignew.host[0]= 0;
	  confignew.load();
	  //copy colors & pictures
	  colback1= confignew.colback1;
	  colback2= confignew.colback2;
	  colback= confignew.colback;
	  black= confignew.black;
	  coluline= confignew.coluline;
	  for(int i= 0; i<8; i++) {
	    color[i]= confignew.color[i];
	  };
	  wcscpy(pic, confignew.pic);
	  gammalow= confignew.gammalow;
	  gammamid= confignew.gammamid;
	  gammahigh= confignew.gammahigh;
	  if(wp==IDM_LOAD) wcscpy(file, confignew.file);
	  //他のウィンドウは必要なら作成すること
          if(wcolor==0) {
            wcolor= CreateDialogIndirect(theInst
            , (DLGTEMPLATE*)LockResource(LoadResource(theInst
            , FindResource(theInst, MAKEINTRESOURCE(IDD_COLOR), RT_DIALOG))), w, colorwrap);
            SetWindowPos(wcolor, 0, drect.left, drect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	  };
          if(wmisc==0) {
            wmisc= CreateDialogIndirect(theInst
            , (DLGTEMPLATE*)LockResource(LoadResource(theInst
            , FindResource(theInst, MAKEINTRESOURCE(IDD_MISC), RT_DIALOG))), w, miscwrap);
            SetWindowPos(wmisc, 0, drect.left, drect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	  };
	  if(wp==IDM_LOAD) confignew.hostset(whost);
	  colorset(wcolor);
	  miscgset(wmisc);
	  confignew.miscset(wmisc);
	  if(cert) free(cert);
	  certalloc= confignew.certalloc;
	  certlen= confignew.certlen;
	  cert= confignew.cert;

	  delete theDraw;
	  drawnew();
          colorpalette();
          HDC dc= GetDC(wtitle);
          titledraw(dc);
          ReleaseDC(wtitle, dc);
          theDraw->redrawwin(theScreen, wdisp);
	};
      };      
      return(TRUE);
    case IDM_SAVE:
      {
        WCHAR dir[_MAX_PATH];
	wcscpy(dir, file);
        WCHAR fname[_MAX_PATH], *p;
        p= wcsrchr(dir, '\\');
	if(p) {
	  wcscpy(fname, p+1);
	  *p= 0;
	} else {
	  wcscpy(fname, dir);
	  dir[0]= 0;
	};
        OPENFILENAME fns;
        memset((void*)&fns, 0, sizeof(fns));
        fns.lStructSize= sizeof(fns);
        fns.hwndOwner= w;
        fns.lpstrFilter= L"Configs (*.24)\0*.24\0All Files (*.*)\0*.*\0";
        fns.nFilterIndex= 1;
        fns.lpstrFile= fname;
        fns.nMaxFile= numof(fname);
	fns.lpstrInitialDir= dir;
        fns.lpstrTitle= L"Save Configuration";
        fns.Flags= OFN_OVERWRITEPROMPT;
	HWND wback= wmodal;
	wmodal= (HWND)1;
	EnableWindow(wcmenu, 0);
        int rt= GetSaveFileName(&fns);
	wmodal= wback;
	EnableWindow(wcmenu, 1);
	if(rt) {
          Config confignew= *this;
	  confignew.hostget(whost);
	  confignew.colorget(wcolor);
	  confignew.miscget(wmisc);
          wcscpy(confignew.file, fname);
	  PCWSTR es= confignew.save();
	  if(es[0]!='0'&&es[0]!='1') { //error
            MessageBeep(MB_ICONEXCLAMATION);
	    WCHAR buf[_MAX_PATH];
	    wsprintf(buf, L"mSave Error\n%s", es+1);
	    HWND wback= wmodal;
            DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wconfig, messageproc, (long)buf);
            wmodal= wback;
	  } else if(*es=='0') { //save new file
            //関連付けはメニュー化
	  };
	};
      };      
      return(TRUE);
    case IDM_ASSOC:
      {
        HKEY reg;
	DWORD rd;
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, L".24", 0, 0, 0, 0, 0, &reg, &rd)==0) {
	  RegSetValueEx(reg, L"", 0, REG_SZ, (BYTE*)L"24termconfig", 26);
	  RegCloseKey(reg);
	};
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"24termconfig", 0, 0, 0, 0, 0, &reg, &rd)==0) {
	  RegSetValueEx(reg, L"", 0, REG_SZ, (BYTE*)L"24term Configration", 40);
	  RegCloseKey(reg);
	};
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"24termconfig\\DefaultIcon", 0, 0, 0, 0, 0, &reg, &rd)==0) {
	  WCHAR buf[_MAX_PATH];
	  wsprintf(buf+GetModuleFileName(0, buf, _MAX_PATH-10), L",-%d", IDI_CONFIG);
	  RegSetValueEx(reg, L"", 0, REG_SZ, (BYTE*)buf, wcslen(buf)*2+2);
	  RegCloseKey(reg);
	};
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"24termconfig\\Shell\\Open\\Command", 0, 0, 0, 0, 0, &reg, &rd)==0) {
	  WCHAR buf[_MAX_PATH];
	  buf[0]= '"'; wcscpy(buf+GetModuleFileName(0, buf+1, _MAX_PATH-10)+1, L"\" \"%1\"");
	  RegSetValueEx(reg, L"", 0, REG_SZ, (BYTE*)buf, wcslen(buf)*2+2);
	  RegCloseKey(reg);
	};
      };
      return(TRUE);
    };
    break;
  case WM_SYSCOMMAND:
    if(wp==SC_KEYMENU) {
      cmenudrop();
      return(0);
    };
    break;
  case WM_QUERYNEWPALETTE:
    return disppalette();
  case WM_ACTIVATE:
    return dispactivate(wp);
  case WM_WINDOWPOSCHANGED:
    if(((WINDOWPOS*)lp)->flags&SWP_SHOWWINDOW) disppalette();
    return(FALSE); 
  };
  return(FALSE);
};
