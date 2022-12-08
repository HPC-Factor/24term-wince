// screen.cpp: Screen クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include <af_irda.h>
#include <winsock.h>
#include <sslsock.h>
#include "common.h"
#include "screen.h"

void Screen::
rollup(int top, int tail)
{
  int i, y, ic;
  if(top==23&&tail==0) {
    EnterCriticalSection(&lock);
    int ss= selstart+80;
    int se= selend+80;
    if(ss<=80||se>=ymax*80||ss==se) {
      ss= 0; se= 0;
    } else if(ss>=ymax*80) {
      ss= ymax*80;
    };
    selstart= ss; selend= se;
    ybase++; if(ybase>=ymax) ybase= 0;
    int w= -ybase; if(w<0) w+= ymax;
    ic= iy[w];
    if(yscroll>0&&yscroll<ymax-24) { //自動スクロール
      yscroll++;
    } else {
      for(y= 23; y>0; y--) {
        copy[y]= copy[y-1];
      };
      copy[0]= -10000;
    };
    LeaveCriticalSection(&lock);
  } else {
    EnterCriticalSection(&lock);
    int w= top-ybase; if(w<0) w+= ymax;
    ic= iy[w];
    for(i= top; i>tail; i--) {
      int nw= w-1; if(nw<0) nw+= ymax;
      iy[w]= iy[nw];
      w= nw;
    };
    iy[w]= ic;
    if(top>=yscroll) {
      i= top-tail;
      y= top-yscroll;
      while(i>0&&y>0) {
        copy[y]= copy[y-1];
	y--;
	i--;
      };
      copy[y]= -10000;
    };
    LeaveCriticalSection(&lock);
  };
  long* p= (long*)&text[ic][0];
  for(i= 0; i<41; i++) *p++= 0x10001;
  dirty[ic]= 0|80<<8;
};

void Screen::
rolldown(int top, int tail)
{
  int i, y, ic;
  EnterCriticalSection(&lock);
  int w= tail-ybase; if(w<0) w+= ymax;
  ic= iy[w];
  for(i= tail; i<top; i++) {
    int nw= w+1; if(nw>=ymax) nw-= ymax;
    iy[w]= iy[nw];
    w= nw;
  };
  iy[w]= ic;
  if(top-yscroll>=0) {
    i= top-tail;
    y= tail-yscroll;
    if(y<0) i+= y, y= 0;
    while(i>0) {
      copy[y]= copy[y+1];
      y++;
      i--;
    };
    copy[y]= -10000;
  };
  LeaveCriticalSection(&lock);
  long* p= (long*)&text[ic][0];
  for(i= 0; i<41; i++) *p++= 0x10001;
  dirty[ic]= 0|80<<8;
};

void Screen::
enddirty()
{
  if(dirtyx>=80) return;
  int ds, de, ic;
  int cx= curx;
  if((*textp&0x180)==0x180) {
    *textp&= 0xfe00;
    cx++;
  };
  int w= cury-ybase; if(w<0) w+= ymax;
  ic= iy[w];
  ds= dirty[ic]; de= ds>>8; ds&= 0xff;
  if(ds>dirtyx) ds= dirtyx;
  if(de<cx) de= cx;
  dirty[ic]= ds|de<<8;
  dirtyx= 80;
};

void Screen::
textpupdate()
{
  int w= cury-ybase; if(w<0) w+= ymax;
  textp= &text[iy[w]][curx];
};

void Screen::
clearbefore()
{
  enddirty();
  int i, de, ic;
  int cx= curx+1;
  if(cx>=80) cx= 80;
  int w= cury-ybase; if(w<0) w+= ymax;
  ic= iy[w];
  WORD* tp= &text[ic][0];
  for(i= 0; i<cx; i++) *tp++= 1;
  if((*tp&0x180)==0x180) {
    *tp&= 0xfe00;
    cx++;
  };
  de= dirty[ic]>>8;
  if(de<cx) de= cx;
  dirty[ic]= de<<8;
};

void Screen::
clearafter()
{
  int i, ds, ic;
  if(dirtyx>curx) {
    dirtyx= curx;
    if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
  };
  WORD* tp= textp;
  for(i= curx; i<81; i++) {
    *tp++= 1;
  };
  if(dirtyx<80) { //enddirty
    int w= cury-ybase; if(w<0) w+= ymax;
    ic= iy[w];
    ds= dirty[ic]&0xff;
    if(ds>dirtyx) ds= dirtyx;
    dirty[ic]= ds|80<<8;
  };
  dirtyx= 80;
};

void Screen::
nextline()
{
  *textp= 0x20|attr; //next line marker
  int ds;
  if(dirtyx<80) { //enddirty cxは80とする
    int w= cury-ybase; if(w<0) w+= ymax;
    ds= dirty[iy[w]]&0xff;
    if(ds>dirtyx) ds= dirtyx;
    dirty[iy[w]]= ds|80<<8;
  };
  curx= 0;
  if(cury>rolltail) {
    cury--;
  } else {
    rollup(rolltop, rolltail);
    cury= rolltail;
  };
  textpupdate();
  dirtyx= 0;
};

void Screen::
print(const char*rp, int rn)
{
  int printlocked= 0;
  if(config.echo==1) {
    WaitForSingleObject(printlock, 0);
    printlocked= 1;
  };
  int c, ic, v, i, y, a;
  while(--rn>=0) {
    c= *(BYTE*)rp++;
    if(state==STiac) {
      state= STbase;
      if(iac==0) { //IAC ?
        iac= c;
        if(c==240) { //IAC SE
	  if(iacsn>0) {
	    if(iacs[0]==24) { //TERMINAL-TYPE
   	      BYTE buf[99];
	      buf[0]= 255; buf[1]= 250; buf[2]= 24; buf[3]= 0; //IAC SB TT IS
	      memcpy(buf+4, "vt100", 5);
              buf[9]= 255; buf[10]= 240;
	      write((char*)buf, 11);
	    } else if(iacs[0]==37&&iacs[1]==1&&ssl==1) { //authentication send
   	      BYTE buf[99];
	      buf[0]= 255; buf[1]= 250; buf[2]= 37; buf[3]= 0; //IAC SB AUTH IS
	      buf[4]= 7; buf[5]= 0;
	      buf[6]= 1; //start
	      buf[7]= 255; buf[8]= 240;
	      write((char*)buf, 9);
	    } else if(iacs[0]==37&&iacs[1]==2&&ssl==1) { //authentication reply
	      if(iacsn>=5&&iacs[2]==7&&iacs[3]==0&&iacs[4]==2) {
	        DEBUGMSG(1, (L"entered\n"));
	        int rt= WSAIoctl(sock, SO_SSL_PERFORM_HANDSHAKE, 0, 0, 0, 0, 0, 0, 0);
                DEBUGMSG(1, (L"handshake %d\n", rt));
	        if(rt==0) { //connected
	          ssl= 2;
		  sockmode= 1; //connected
		  extern HWND wdisp;
                  PostMessage(wdisp, WM_USER+2, 0, (long)this);
                  if(theScreen==this) PostMessage(wdisp, WM_USER, 0, (long)this);
		} else {
                  wcscpy(error, L"tFailed to connect to host.");
		  sockmode= -2;
		  //exit to main loop
		};
	      };
	    };
	  };
	} else if(c==250) {
	  iacsn= 0;
	  state= STiac;
	} else if(c>=251&&c<=254) {
	  state= STiac;
	};
      } else if(iac==250) { //SB
        if(c==255) {
	  iac= 0;
	} else if(iacsn<32) {
	  iacs[iacsn++]= c;
	};
	state= STiac;
      } else if(iac==253) { //DO request
        if(c==24||c==3||c==0) { //terminal type,sup-go-ahead, binary
	  BYTE buf[3];
	  buf[0]= 255; buf[1]= 251; buf[2]= c; //WILL
	  write((char*)buf, 3);
	} else if(c==31) { //window size
	  BYTE buf[99];
	  buf[0]= 255; buf[1]= 251; buf[2]= c; //WILL
	  buf[3]= 255; buf[4]= 250; buf[5]= c; buf[6]= 0; buf[7]= 80; buf[8]= 0; buf[9]= 24; //SB 80x24
	  buf[10]= 255; buf[11]= 240;
	  write((char*)buf, 12);
	} else if(c==37&&ssl==1) { //authentication
	  BYTE buf[3];
	  buf[0]= 255; buf[1]= 251; buf[2]= c; //WILL
	  write((char*)buf, 3);
	} else {
	  BYTE buf[3];
	  buf[0]= 255; buf[1]= 252; buf[2]= c; //WON'T
	  write((char*)buf, 3);
	};
      } else if(iac==251) { //WILL request
        if(c==1||c==3||c==0) { //echo,sup-go-ahead, binary
          BYTE buf[3];
	  buf[0]= 255; buf[1]= 253; buf[2]= c; //DO
	  write((char*)buf, 3);
	} else {
          BYTE buf[3];
	  buf[0]= 255; buf[1]= 254; buf[2]= c; //DON'T
	  write((char*)buf, 3);
	};
      };
    } else if(ssl==1) { //ssl-telnet-connecting
      if(c==0xff) { //IAC
        state= STiac; iac= 0; iacsn= 0;
      } else if(c!=0) { //null以外
        state= STbase;
      };
    } else {
      if(c>=0x20&&c<=0xfe&&c!=0x7f) {
        if(state==STbase) {
	  if(config.sjis==0&&(c==0x8e||c>=0xa1&&c<=0xfe)
          || config.sjis==1&&(c>=0x81&&c<=0x9f||c>=0xe0&&c<=0xfc)
	  || recvset==3&&c>=0x21&&c<=0x7e) { //normal-kanji
            state= STkanji;
	    kanji1= c;
	  } else if(c<=0x7e&&!(recvset==2&&c>=0x60)
	  ||c>=0xa0&&c<=0xdf) { //ascii & kana
	    if(recvset==2) c|= 0x80;
            if(dirtyx>curx) {
                dirtyx= curx;
              if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
            };
            if(curx>=80) nextline();
	    if(c==0x5c&&config.yen==0) c= 0x7f;
	    *textp++= c|attr;
	    curx++;
	  };
        } else if(state==STkanji) {
	  state= STbase;
          if(config.sjis==0&&kanji1==0x8e) {
	    if(c>=0xa0&&c<=0xdf) { //hankaku-kana
              if(dirtyx>curx) {
                dirtyx= curx;
                if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
              };
              if(curx>=80) nextline();
	      *textp++= c|attr;
	      curx++;
	    };
	  } else if(config.sjis==0&&c>=0xa1&&c<=0xfe
          || config.sjis==1&&c<=0xfc&&kanji1<=0xef
	  || kanji1<=0x7e&&c>=0x21&&c<=0x7e) {
	    if(dirtyx>curx) {
  	      dirtyx= curx;
	      if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
	    };
            if(curx==79) *textp++= attr;
	    if(curx>=79) nextline();
  	    textp[0]= attr;
	    if(kanji1<=0x7e) {
	      textp[1]= c-0x21+0x180|attr;
	      textp[0]= kanji1-0x21+0x100|attr;
	    } else if(config.sjis==0) {
	      textp[1]= c-0xa1+0x180|attr;
	      textp[0]= kanji1-0xa1+0x100|attr;
	    } else if(config.sjis==1) {
	      if(c>=0x9f) {
	        textp[1]= c-0x9f+0x180|attr;
	        textp[0]= (kanji1&0x3f)*2-1+0x100|attr;
	      } else {
	        textp[1]= c-0x40-(c>=0x80)+0x180|attr;
	        textp[0]= (kanji1&0x3f)*2-2+0x100|attr;
	      };
	    };
            textp+= 2; curx+= 2;
	  };
        } else if(state==STesc) {
          state= STbase;
          if(c=='[') {
	    csimode= 0;
	    csiargc= 0;
	    for(i= 0; i<numof(csiargv); i++) csiargv[i]= 0;
	    state= STcsi;
	  } else if(c=='c') { //reset
            attrfg= 0;
	    attr= attrfg; attrbg= 0;
	    rolltop= 23; rolltail= 0;
	    recvset= 0; sendset= 0;
	  } else if(c=='D'||c=='E') { //LF or NL
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    if(cury>rolltail) {
              cury--;
            } else {
              rollup(rolltop, rolltail);
              cury= rolltail;
            };
	    if(c=='E') curx= 0; //NewLINE
            textpupdate();
	  } else if(c=='M') { //reverse line feed
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    if(cury<rolltop) {
	      cury++;
	    } else {
              rolldown(rolltop, rolltail);
              cury= rolltop;
            };
	    textpupdate();
	  } else if(c=='$') {
	    state= STchar;
	    charset= 8;
	  } else if(c>='('&&c<='/'&&c!=',') {
	    state= STchar;
            charset= c-'(';
	  };
        } else if(state==STcsi) {
          state= STbase;
	  if(c>='0'&&c<='9') {
	    state= STcsi;
	    if(csiargv[csiargc]<1000) {
	      csiargv[csiargc]= csiargv[csiargc]*10+c-'0';
	    };
          } else if(c=='?') {
	    state= STcsi;
	    csimode|= 1;
	  } else if(c==';') {
	    state= STcsi;
	    if(csiargc<numof(csiargv)-1) csiargc++;
	  } else if(c=='@') {
	  } else if(c=='A') { //上方向
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    v= csiargv[0]; if(v<=0) v= 1;
	    v= cury+v; if(v>23) v= 23;
	    cury= v;
            textpupdate();
	  } else if(c=='B') { //下方向
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    v= csiargv[0]; if(v<=0) v= 1;
	    v= cury-v; if(v<0) v= 0;
	    cury= v;
	    textpupdate();
	  } else if(c=='C') { //右方向
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    v= csiargv[0]; if(v<=0) v= 1;
	    v= curx+v; if(v>79) v= 79;
	    curx= v;
	    textpupdate();
	  } else if(c=='D') { //左方向
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    v= csiargv[0]; if(v<=0) v= 1;
	    v= curx-v; if(v<0) v= 0;
	    curx= v;
	    textpupdate();
          } else if(c=='H'||c=='f') { //カーソル移動
  	    if(config.sjis!=2) recvset= 0;
            enddirty();
	    v= csiargv[1]-1;
	    if(v<0) v= 0;
	    if(v>80) v= 80;
	    curx= v;
	    v= 24-csiargv[0];
	    if(v<0) v= 0;
	    if(v>23) v= 23;
	    cury= v;
            textpupdate();
	  } else if(c=='J') { //クリア、カーソル移動なし
	    if(csiargv[0]==2) { //全クリア
	      enddirty();
	      EnterCriticalSection(&lock);
	      if(yscroll<24) {
	        for(y= 0; y<24-yscroll; y++) {
		  copy[y]= -10000;
		};
	      };
	      LeaveCriticalSection(&lock);
	      for(y= 0; y<24; y++) {
                int w= y-ybase; if(w<0) w+= ymax;
	        ic= iy[w];
	        long* p= (long*)&text[ic][0];
                for(i= 0; i<41; i++) *p++= 0x10001;
	        dirty[ic]= 0|80<<8;
	      };
	    } else if(csiargv[0]==1) { //カーソル以前クリア
	      clearbefore();
	      EnterCriticalSection(&lock);
	      if(yscroll<24) {
	        y= cury+1-yscroll; if(y<0) y= 0;
	        for(;y<24-yscroll; y++) {
		  copy[y]= -10000;
		};
	      };
	      LeaveCriticalSection(&lock);
	      for(y= cury+1; y<24; y++) {
                int w= y-ybase; if(w<0) w+= ymax;
	        ic= iy[w];
	        long* p= (long*)&text[ic][0];
                for(i= 0; i<41; i++) *p++= 0x10001;
	        dirty[ic]= 0|80<<8;
	      };
	    } else { //カーソル以後クリア
	      clearafter();
	      EnterCriticalSection(&lock);
	      if(yscroll<cury) {
	        for(y= 0; y<cury-yscroll; y++) {
		  copy[y]= -10000;
		};
	      };
	      LeaveCriticalSection(&lock);
	      for(y= 0; y<cury; y++) {
                int w= y-ybase; if(w<0) w+= ymax;
	        ic= iy[w];
	        long* p= (long*)&text[ic][0];
                for(i= 0; i<41; i++) *p++= 0x10001;
	        dirty[ic]= 0|80<<8;
	      };
	    };
	  } else if(c=='K') { //クリア
	    if(csiargv[0]==2) { //行クリア
	      EnterCriticalSection(&lock);
	      if(yscroll<=cury) {
	        copy[cury-yscroll]= -10000;
	      };
	      LeaveCriticalSection(&lock);
              int w= cury-ybase; if(w<0) w+= ymax;
	      ic= iy[w];
	      long* p= (long*)&text[ic][0];
              for(i= 0; i<41; i++) *p++= 0x10001;
	      dirty[ic]= 0|80<<8;
	      dirtyx= 80;
	    } else if(csiargv[0]==1) { //カーソル以前クリア
	      clearbefore();
	    } else {
	      clearafter();
	    };
	  } else if(c=='m') { //色
	    for(i= 0; i<=csiargc; i++) {
 	      if(csiargv[i]==0) {
                attrfg= 0;
		attrbg= 0;
	      } else if(csiargv[i]==1) {
	        if(config.nobold==0) attrfg|= SC_BOLD;
	      } else if(csiargv[i]==4) {
	        attrfg|= SC_ULINE;
	      } else if(csiargv[i]==5) {
	        attrfg|= SC_BLINK;
	      } else if(csiargv[i]==7) {
	        attrfg|= SC_REVERSE;
	      } else if(csiargv[i]>=30&&csiargv[i]<=37) {
	        attrfg= (attrfg&~SC_COLOR)|(csiargv[i]-30)<<9;
	      } else if(csiargv[i]>=40&&csiargv[i]<=47) {
	        attrbg= csiargv[i];
	      };
	    };
	    if(attrbg==0||attrbg==47) {
	      attr= attrfg;
	    } else {
	      attr= (attrfg&~SC_COLOR^SC_REVERSE)|(attrbg-40)<<9;
	    };
	  } else if(c=='r') { //スクロール範囲
	    rolltop= 24-csiargv[0]; rolltail= 24-csiargv[1];
	    if(rolltop<0) rolltop= 0;
	    if(rolltail<0) rolltail= 23;
            if(rolltop>23) rolltop= 23;
	    if(rolltail>23) rolltail= 23;
	    if(rolltop<rolltail) {
	      a= rolltop; rolltop= rolltail; rolltail= a;
	    };
	  } else {
	  };
        } else if(state==STchar) {
	  state= STbase;
	  if(c=='$') {
	    charset|= 8;
	    state= STchar;
	  } else if(c=='('&&c<='/'&&c!=',') {
	    charset= charset&8|(c-'(');
	    state= STchar;
	  } else if(c=='@') {
	    if(charset==8) recvset= 3; //kanji
	  } else if(c=='B') {
	    if(charset==8) recvset= 3; //kanji
	    if(charset==0) recvset= 0;
	  } else if(c=='I') {
	    if(charset==0) recvset= 2; //kana
	  } else if(c=='J') {
	    if(charset==0) recvset= 1; //roman code
	  };
	};
      } else if(c==9) { //tab
        state= STbase;
	enddirty();
	v= (curx/8+1)*8;
	if(v>79) v= 79;
	curx= v;
	textpupdate();
      } else if(c==0xd) { //cr
	if(config.sjis!=2) recvset= 0;
        state= STbase;
        enddirty();
	curx= 0;
	if(config.newline==1) { //newline=cr
	  if(cury>rolltail) {
            cury--;
	  } else {
            rollup(rolltop, rolltail);
            cury= rolltail;
          };
	};
        textpupdate();
      } else if(c==0xa||c==0xb||c==0xc) { //lf
	if(config.sjis!=2) recvset= 0;
        state= STbase;
        enddirty();
	if(config.newline==2) curx= 0; //newline=lf
	if(cury>rolltail) {
          cury--;
	} else {
          rollup(rolltop, rolltail);
          cury= rolltail;
        };
	textpupdate();
      } else if(c==0x7) { //bell
        state= STbase;
        MessageBeep(-1);
      } else if(c==0x8) { //bs
        state= STbase;
        enddirty();
	if(curx>0) curx--;
	textpupdate();
      } else if(c==0x1b) { //escape
        state= STesc;
	charset= 0;
      } else if(c==0xff) { //IAC
        state= STiac; iac= 0; iacsn= 0;
      } else if(c!=0) { //null以外
        state= STbase;
      };
    };
  };
  enddirty();
  if(printlocked) ReleaseMutex(printlock);
};

//ローカルエコー用
void Screen::
printk(const char*rp, int rn)
{
  if(WaitForSingleObject(printlock, 500)!=WAIT_OBJECT_0) return;
  int c, v;
  int statek= STbase;
  int kanji1k;
  while(--rn>=0) {
    c= *(BYTE*)rp++;
    if(1) {
      if(c>=0x20&&c<=0xfe&&c!=0x7f) {
        if(statek==STbase) {
	  if(c>=0x81&&c<=0x9f||c>=0xe0&&c<=0xfc) {
            statek= STkanji;
	    kanji1k= c;
	  } else if(c<=0x7e||c>=0xa0&&c<=0xdf) { //ascii & kana
            if(dirtyx>curx) {
                dirtyx= curx;
              if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
            };
            if(curx>=80) nextline();
	    if(c==0x5c&&config.yen==0) c= 0x7f;
	    *textp++= c|(attr^SC_ULINE);
	    curx++;
	  };
        } else if(statek==STkanji) {
	  statek= STbase;
	  if(c<=0xfc&&kanji1k<=0xef) {
	    if(dirtyx>curx) {
  	      dirtyx= curx;
	      if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
	    };
            if(curx==79) *textp++= attr^SC_ULINE;
	    if(curx>=79) nextline();
  	    textp[0]= attr^SC_ULINE;
	    if(1) {
	      if(c>=0x9f) {
	        textp[1]= c-0x9f+0x180|(attr^SC_ULINE);
	        textp[0]= (kanji1k&0x3f)*2-1+0x100|(attr^SC_ULINE);
	      } else {
	        textp[1]= c-0x40-(c>=0x80)+0x180|(attr^SC_ULINE);
	        textp[0]= (kanji1k&0x3f)*2-2+0x100|(attr^SC_ULINE);
	      };
	    };
            textp+= 2; curx+= 2;
	  };
	};
      } else if(c==9) { //tab
        statek= STbase;
	enddirty();
	v= (curx/8+1)*8;
	if(v>79) v= 79;
	curx= v;
	textpupdate();
      } else if(c==0xd) { //cr
        statek= STbase;
        enddirty();
	curx= 0;
	if(config.enter==1) { //enter=cr
	  if(cury>rolltail) {
            cury--;
	  } else {
            rollup(rolltop, rolltail);
            cury= rolltail;
          };
	};
        textpupdate();
      } else if(c==0xa||c==0xb||c==0xc) { //lf
        statek= STbase;
        enddirty();
	if(config.enter==2) curx= 0; //enter=lf
	if(cury>rolltail) {
          cury--;
	} else {
          rollup(rolltop, rolltail);
          cury= rolltail;
        };
	textpupdate();
      } else if(c==0x8) { //bs
        statek= STbase;
        enddirty();
	if(curx>0) curx--;
	textpupdate();
      } else { //それ以外
        statek= STbase;
        if(dirtyx>curx) {
          dirtyx= curx;
          if((*textp&0x180)==0x180) { textp[-1]&= 0xfe00; dirtyx--; };
        };
        if(curx==79) *textp++= attr^SC_ULINE;
	if(curx>=79) nextline();
	v= c%16; if(v<10) v= v+'0'; else v= v-10+'a';
	c/= 16; if(c<10) c= c+'0'; else c= c-10+'a';
  	textp[0]= attr^SC_ULINE^SC_REVERSE;
	textp[1]= v|(attr^SC_ULINE^SC_REVERSE);
	textp[0]= c|(attr^SC_ULINE^SC_REVERSE);
        textp+= 2; curx+= 2;
      };
    };
  };
  enddirty();
  ReleaseMutex(printlock);
};

void Screen::
init()
{
  thread= 0;
  sock= INVALID_SOCKET;
  sockmode= 0;
  socktype= 0;
  sockfreeze= 0;
  socklock= 0;
  ssl= 0;
  certreq= 0;
  error[0]= 0;
  printlock= CreateMutex(0, 0, 0);

  InitializeCriticalSection(&lock);
  ybase= 0;
  selstart= 0; selend= 0;
  ymax= 24; //sentinel
  yscroll= 0;
  iy= 0;
  text= 0;
  dirty= 0;

  state= STbase;
  curx= 0; cury= 23;
  rolltop= 23; rolltail= 0;
  attrfg= 0;
  attr= attrfg; attrbg= 0;
  sendset= 0; recvset= 0;
  rts= 0; dtr= 0;
};

PCWSTR Screen::
allocate()
{
  int i, w;
  ymax= config.ymax;
  iy= (int*)malloc(sizeof(int)*ymax);
  if(iy==0) return L"aNo memory for screen.";
  text= (WORD(*)[82])malloc(sizeof(WORD)*82*ymax);
  if(text==0) return L"aNo memory for screen.";
  dirty= (long*)malloc(sizeof(long)*ymax);
  if(dirty==0) return L"aNo memory for screen.";
  for(i= 0; i<24; i++) copy[i]= -10000;
  for(w= 0; w<ymax; w++) {
    iy[w]= w;
    long* p= (long*)&text[w][0];
    for(i= 0; i<41; i++) *p++= 0x10001;
    dirty[w]= 0|80<<8;
  };
  dirtyx= 80;
  textpupdate();
  csimode= 0;
  csiargc= 0;
  for(i= 0; i<numof(csiargv); i++) csiargv[i]= 0;

  return L"";
};


PCWSTR Screen::
open()
{
  int rt;
  if(config.host[0]!='#') {
    socktype= 1;
    sock= socket(AF_INET, SOCK_STREAM, 0);
    if(sock==INVALID_SOCKET) return L"sFailed to create socket.";

    DWORD ipaddr= INADDR_NONE;
    int port= 0;
    int passive= 0;
    WCHAR *p= config.host;
    for(;*p&&*p!=':';p++) ;
    char hbuf[_MAX_PATH];
    int hn= WideCharToMultiByte(0, 0, config.host, p-config.host, hbuf, numof(hbuf), 0, 0);
    hbuf[hn]= 0;
    if(*p==':') {
      p++;
      port= 0;
      while(*p>='0'&&*p<='9') port= port*10+(*p++ - '0');
      WCHAR c;
      for(;(c= *p)&&c!=':';p++) {
        switch(c) {
	case 'p': case 'P':
	  passive= 1; break;
	case 's': case 'S':
	  if(ssl==0) ssl= 2;
	  break;
	case 'z': case 'Z':
	  ssl= 1; break;
	case 'c': case 'C':
	  certreq= 1; break;
	};
      };
    };
    if(ssl==0) certreq= 0;
    if(hbuf[0]!=0) {
      ipaddr= inet_addr(hbuf);
      if(ipaddr==INADDR_NONE) {
        struct hostent *he;
        he= gethostbyname(hbuf); //thread依存のためfree不要
        if(he!=0) {
          ipaddr= *(DWORD*)he->h_addr;
        };
      };
      if(ipaddr==INADDR_NONE) return L"tFailed to lookup host name.";
    };

    if(passive==0) {
      if(ssl) {
        socktype= 2; //排他socket
        DWORD f= SO_SEC_SSL;
        setsockopt(sock, SOL_SOCKET, SO_SECURE, (const char*)&f, sizeof(f));
        SSLVALIDATECERTHOOK hook;
        hook.HookFunc= certwrap;
        hook.pvArg= this;
        WSAIoctl(sock, SO_SSL_SET_VALIDATE_CERT_HOOK, &hook, sizeof(hook), 0, 0, 0, 0, 0);
	if(ssl==1) {
          DWORD dd;
	  rt= WSAIoctl(sock, SO_SSL_GET_FLAGS, 0, 0, &f, sizeof(f), &dd, 0, 0);
	  if(rt==0) {
	    f|= SSL_FLAG_DEFER_HANDSHAKE;
	    rt= WSAIoctl(sock, SO_SSL_SET_FLAGS, &f, sizeof(f), 0, 0, 0, 0, 0);
	  };
	};
      };
      if(ipaddr==INADDR_NONE) ipaddr= htonl(INADDR_LOOPBACK);
      if(port==0) port= 23;
      SOCKADDR_IN addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family= AF_INET;
      addr.sin_port= htons(port);
      addr.sin_addr.S_un.S_addr= ipaddr;
      rt= connect(sock, (SOCKADDR*)&addr, sizeof(addr));
      if(rt==SOCKET_ERROR) return L"tFailed to connect to host.";
    } else { //待ち受け
      if(ssl) return L"tCannot SSL in passive.";
      if(ipaddr==INADDR_NONE) ipaddr= INADDR_ANY;
      SOCKADDR_IN addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family= AF_INET;
      addr.sin_port= htons(port);
      addr.sin_addr.S_un.S_addr= ipaddr;
      rt= bind(sock, (SOCKADDR*)&addr, sizeof(addr));
      if(rt==SOCKET_ERROR) return L"tFailed to bind address.";
      rt= listen(sock, 1);
      if(rt==SOCKET_ERROR) return L"tFailed to listen.";
      SOCKET tsock;
      tsock= accept(sock, 0, 0);
      if(tsock==INVALID_SOCKET) return L"tFailed to accept.";
      closesocket(sock);
      sock= tsock;
    };
    if(socktype==2) {
      socklock= CreateMutex(0, 0, 0);
      //DWORD f= 1;
      //WSAIoctl(sock, FIONBIO, &f, sizeof(f), 0, 0, 0, 0, 0);
    };
    if(ssl!=1) sockmode= 1;
  } else if(config.host[1]!='#') { //serial
    socktype= 4;
    WCHAR *p= config.host;
    p++;
    WCHAR cname[_MAX_PATH], *np;
    np= cname;
    if(*p>='0'&&*p<='9') wcscpy(cname, L"COM"), np= cname+3;
    while(*p&&*p!=':') {
      *np++= *p++;
    };
    *np++= ':'; *np= 0;
    sock= (SOCKET)CreateFile(cname, GENERIC_READ|GENERIC_WRITE
    , 0, 0, OPEN_EXISTING, 0, 0);
    if((HANDLE)sock==INVALID_HANDLE_VALUE) {
      return L"sFailed to open com port.";
    };
    SetupComm((HANDLE)sock, 1600, 1600);
    COMMPROP cp;
    int rsize= 0;
    if(GetCommProperties((HANDLE)sock, &cp)) rsize= cp.dwCurrentRxQueue;
    if(rsize==0) rsize= 256;
    int irmode= 0;
    DCB cd;
    GetCommState((HANDLE)sock, &cd);
    cd.BaudRate= 9600; cd.ByteSize= 8;
    cd.Parity= NOPARITY; cd.StopBits= ONESTOPBIT;
    cd.fOutxCtsFlow= FALSE; cd.fRtsControl= RTS_CONTROL_ENABLE;

    cd.fOutX= FALSE; cd.fInX= FALSE; //no X_ON_X_OFF

    cd.fNull= FALSE; cd.fTXContinueOnXoff= TRUE;
    cd.fOutxDsrFlow= FALSE; cd.fDsrSensitivity= FALSE; //ignore DSR
    cd.fDtrControl= DTR_CONTROL_ENABLE; //DTR on
    cd.fParity= FALSE; cd.fErrorChar= FALSE; cd.fAbortOnError= FALSE;
    cd.XonChar= 17; cd.XoffChar= 19;
    cd.XonLim= rsize/4; cd.XoffLim= rsize*3/4;
    if(*p==':') {
      p++;
      if(*p>='0'&&*p<='9') {
        cd.BaudRate= 0;
        while(*p>='0'&&*p<='9') cd.BaudRate= cd.BaudRate*10+(*p++ - '0');
      };
      for(;*p&&*p!=':';p++) ;
      if(*p==':') {
        WCHAR c;
        while((c= *++p)&&c!=':') {
	  switch(c) {
	  case '7':
	    cd.ByteSize= 7; break;
	  case 'e': case 'E':
	    cd.Parity= EVENPARITY; break;
	  case 'o': case 'O':
	    cd.Parity= ODDPARITY; break;
	  case '2':
	    cd.StopBits= TWOSTOPBITS; break;
          case 'h': case 'H': case 'r': case 'R':
	    cd.fOutxCtsFlow= TRUE; cd.fRtsControl= RTS_CONTROL_HANDSHAKE; break;
	  case 's': case 'S': case 'x': case 'X':
	    cd.fOutX= TRUE; cd.fInX= TRUE; break;
          case 'i': case 'I':
	    irmode= 1; break;
	  };
	};
      };
    };
    SetCommState((HANDLE)sock, &cd); //ignore error//return L"tFailed to set com state.";
    if(irmode==1) {
      EscapeCommFunction((HANDLE)sock, SETIR);
    } else {
      EscapeCommFunction((HANDLE)sock, CLRIR);
    };
    PurgeComm((HANDLE)sock, PURGE_RXABORT|PURGE_RXCLEAR);
    COMMTIMEOUTS cto;
    cto.ReadIntervalTimeout= MAXDWORD;
    cto.ReadTotalTimeoutMultiplier= 0; //MAXDWORD;
    cto.ReadTotalTimeoutConstant= 0; //10000; //すぐタイムアウト
    cto.WriteTotalTimeoutMultiplier= MAXDWORD;
    cto.WriteTotalTimeoutConstant= 10000;
    SetCommTimeouts((HANDLE)sock, &cto); //無通信でも10秒に1度タイムアウト…ささり防止
    SetCommMask((HANDLE)sock, EV_DSR|EV_CTS|EV_RLSD|EV_RING|EV_RXCHAR);
    rts= 1; dtr= 1;
    if(cd.fRtsControl==RTS_CONTROL_HANDSHAKE) rts= -1;
    sockmode= 1;
  } else { //IRDA socket
    socktype= 3;
    sock= socket(AF_IRDA, SOCK_STREAM, 0);
    if(sock==INVALID_SOCKET) return L"sFailed to create IR socket.";
    DEVICELIST dv;
    int i= 0;
    for(;;) {
      int len= sizeof(dv);
      dv.numDevice= 0;
      getsockopt(sock, SOL_IRLMP, IRLMP_ENUMDEVICES
      , (char*)&dv, &len);
      if(dv.numDevice!=0) break;
      if(++i==3) return L"tFailed to get IR device";
      Sleep(1000);
    };
    SOCKADDR_IRDA addr;
    memset(&addr, 0, sizeof(addr));
    addr.irdaAddressFamily= AF_IRDA;
    
    WideCharToMultiByte(0, 0, config.host+2, -1
    , addr.irdaServiceName, numof(addr.irdaServiceName), 0, 0);

    for(i= 0; i<4; i++) addr.irdaDeviceID[i]= dv.Device[0].irdaDeviceID[i];
    rt= connect(sock, (SOCKADDR*)&addr, sizeof(addr));
    if(rt==SOCKET_ERROR) return L"tFailed to connect IR socket.";
    sockmode= 1;
  };
  return L"";
};

int Screen::
certhook(DWORD type, DWORD clen, BLOB* cert, DWORD flags)
{
  DEBUGMSG(1, (L"blob bytes=%d %d\n", cert->cbSize, config.certlen));
  if(certreq==0) return SSL_ERR_OKAY;
  if(config.cert==0||(int)cert->cbSize!=config.certlen
  || memcmp(cert->pBlobData, config.cert, config.certlen)!=0) return SSL_ERR_CERT_UNKNOWN;
  return SSL_ERR_OKAY;
};

int CALLBACK Screen::
certwrap(DWORD type, void* me, DWORD clen, BLOB* cert, DWORD flags)
{
  return ((Screen*)me)->certhook(type, clen, cert, flags);
};

void Screen::
write(const char*p, int n)
{
  if(sockmode<0) return;
#ifdef DEBUGXXXXX
    for(int i= 0; i<n; i++) {
      DEBUGMSG(1, (L"%02x ", ((BYTE*)p)[i]));
      if(i%16==15||i==n-1) DEBUGMSG(1, (L" send\n"));
    };
#endif
  while(n>0&&sockfreeze==0) {
    if(socktype==1||socktype==3) {
      int rt= send(sock, p, n, 0);
      if(rt>0) {
        p+= rt;
	n-= rt;
      } else {
        sockfreeze= 1;
      };
    } else if(socktype==2) { //排他制御
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sock, &fds);
      select(1, 0, &fds, 0, 0);
      WaitForSingleObject(socklock, INFINITE);
      int rt= send(sock, p, n, 0);
      ReleaseMutex(socklock);
      if(rt>0) {
        p+= rt;
	n-= rt;
      } else {
        sockfreeze= 1;
      };
    } else if(socktype==4) {
      DWORD d= 0;
      int rt= WriteFile((HANDLE)sock, p, n, &d, 0);
      if(rt&&d>0) {
        p+= d;
	n-= d;
      } else {
        sockfreeze= 1;
      };
    } else {
      return;
    };
  };
  return;
};

//漢字を意識して書き込み
void Screen::
writek(const char*p, int n)
{
  BYTE c, d[2];
  while(n>0) {
    c= *(BYTE*)p++; n--;
    if(c>=0xa0&&c<=0xdf) { //カナ
      if(config.sjis==0) { //EUC
        d[0]= 0x8e; d[1]= c;
	write((char*)d, 2);
      } else if(config.sjis==1) { //SJIS
        write((char*)&c, 1);
      } else { //JIS
	if(sendset!=2) {
	  write("\33(I", 3);
	  sendset= 2;
	};
	c&= 0x7f;
	write((char*)&c, 1);
      };
    } else if(c>=0x81&&c<=0xfc) { //漢字
      BYTE c1;
      if(n>0&&(c1= *(BYTE*)p)>=0x40&&c1<=0xfc&&c1!=0x7f) {
        p++; n--;
	if(c<=0xef) { //漢字コードOK
          if(config.sjis!=1) { //SJIS->EUCコード変換
	    d[0]= (c&0x3f)*2+0x9f;
	    if(c1<=0x7f) {
	      d[1]= c1+0x61;
	    } else if(c1<=0x9e) {
	      d[1]= c1+0x60;
	    } else {
	      d[0]++; d[1]= c1+2;
	    };
	  } else { //SJIS
	    d[0]= c; d[1]= c1;
          };
	  if(config.sjis>=2) { //JIS
	    if(sendset!=3) {
	      write("\33$B", 3);
	      sendset= 3;
	    };
	    d[0]&= 0x7f; d[1]&= 0x7f;
	  };
	  write((char*)d, 2);
        };
      };
    } else { //ASCII
      if(config.sjis>=2) { //JIS
        if(sendset) {
	  write("\33(B", 3);
	};
        sendset= 0;
      };
      const char* q= p-1;
      while(n>0&&!(*(BYTE*)p>=0x81&&*(BYTE*)p<=0xfc)) {
        p++; n--;
      };
      write(q, p-q);
    };
  };
};

DWORD WINAPI Screen::
screenwrap(void* me)
{
  ((Screen*)me)->main();
  return(0);
};

void Screen::
main()
{
  extern HWND wdisp;
  int rt;
  BYTE buf[200];
  PCWSTR es;
  es= open();
  if(es[0]) {
    wcscpy(error, es);
    sockmode= -2;
    close();
    PostMessage(wdisp, WM_USER+2, 0, (long)this);
    return;
  };
  if(sockmode>0) {
    PostMessage(wdisp, WM_USER+2, 0, (long)this);
    if(theScreen==this) PostMessage(wdisp, WM_USER, 0, (long)this);
  };

  DWORD commev= 0;

  while(sockmode>=0) {
    if(socktype==1||socktype==3) {
      rt= recv(sock, (char*)buf, sizeof(buf), 0);
      if(rt<=0) break;
    } else if(socktype==2) { //排他制御
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sock, &fds);
      select(1, &fds, 0, 0, 0);
      WaitForSingleObject(socklock, INFINITE);
      rt= recv(sock, (char*)buf, sizeof(buf), 0);
      ReleaseMutex(socklock);
      if(rt<=0) break;
    } else if(socktype==4) {
      int waited= 0;
      if(commev==0) {
        DEBUGMSG(1, (L"wait\n"));
        rt= WaitCommEvent((HANDLE)sock, &commev, 0);
        DEBUGMSG(1, (L"waited\n"));
	waited= 1;
        if(rt==0) break; //closed
      };
      if(commev&(~EV_RXCHAR)) { //other event occured
        if(theScreen==this) PostMessage(wdisp, WM_USER+1, 0, (long)this);
      };
      commev&= EV_RXCHAR;
      if(commev) {
        DWORD d= 0;
        rt= ReadFile((HANDLE)sock, (char*)buf, sizeof(buf), &d, 0);
	DEBUGMSG(1, (L"read=%d\n", d));
        if(rt==0) break;
        rt= d;
	if(rt==0) {
	  if(waited==1) Sleep(50); //to avoid high load when error device was opened
	  commev= 0; //no more read
	};
      } else rt= 0;
    } else break;
    if(rt>0) {
#ifdef DEBUGXXXXX
      for(int i= 0; i<rt; i++) {
        DEBUGMSG(1, (L"%02x ", ((BYTE*)buf)[i]));
        if(i%16==15||i==rt-1) DEBUGMSG(1, (L" rcvd\n"));
      };
#endif
      print((char*)buf, rt);
      if(theScreen==this) PostMessage(wdisp, WM_USER, 0, (long)this);
    };
  };

  if(sockmode>=0) sockmode= -1;
  close();
  if(theScreen==this) PostMessage(wdisp, WM_USER, 0, (long)this);
  PostMessage(wdisp, WM_USER+2, 0, (long)this);
};

void Screen::
close()
{
  if(sock!=INVALID_SOCKET) {
    if(socktype==1||socktype==2||socktype==3) {
      closesocket(sock);
    } else if(socktype==4) {
      PurgeComm((HANDLE)sock, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);
      CloseHandle((HANDLE)sock);
    };
  };
  sock= INVALID_SOCKET;
};

void Screen::
release()
{
  if(printlock) CloseHandle(printlock);
  if(socklock) CloseHandle(socklock);
  if(iy) free(iy);
  if(text) free(text);
  if(dirty) free(dirty);
  if(thread) CloseHandle(thread);
};
