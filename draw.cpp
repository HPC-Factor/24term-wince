// draw.cpp: Draw クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include "common.h"
#include "draw.h"
#include "screen.h"

HBRUSH Draw::basebrush= 0;

HANDLE Draw::k12x10f;
HANDLE Draw::k12x10m;
const char* Draw::k12x10= 0;

HBITMAP Draw::bitmap;
char* Draw::bitdata;
HDC Draw::bitdc= 0;
HDC Draw::picdc= 0; //Pic

int Draw::devcolor;
int Draw::devwidth;
int Draw::winoffx;
int Draw::bitoffx;
int Draw::baseheight;

PCWSTR Draw::
init(const Config* config, HWND wdisp)
{
  int y;
  if(k12x10==0) {
    k12x10f= CreateFileForMapping(L"\\Windows\\Fonts\\k12x10.ttf", GENERIC_READ
    , FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(k12x10f==INVALID_HANDLE_VALUE) return L"fFont \"k12x10.ttf\" not found.";
    k12x10m= CreateFileMapping(k12x10f, 0, PAGE_READONLY, 0, 0, 0);
    k12x10= (const char*)MapViewOfFile(k12x10m, FILE_MAP_READ, 0, 0, 0);
    if(k12x10==0) return L"mFont \"k12x10.ttf\" mapping error.";
  };
  picwidth= 0;

  if(devwidth<640) winoffx= 0; else winoffx= 3;

  for(y= 0; y<24; y++) useb[y]= 80;
  curx= -1; lastsel= 0;
  return L"";
};

//Dirty部分のみ再描画
void Draw::
update(Screen* scr, HDC dc, int mode)
{
  if(scr->iy==0) return;
  int y, a, b, t, cc, e;
  int dirty[24]; //backup of screen->dirty

  if(mode) { //スクロールはなし
    EnterCriticalSection(&scr->lock);
    for(y= 0; y<24; y++) {
      dirty[y]= 0;
      int w= y+scr->yscroll-scr->ybase; if(w<0) w+= scr->ymax;
      iy[y]= scr->iy[w];
      if(scr->copy[y]!=y) {
        scr->copy[y]= y;
	dirty[y]= 0|80<<8;
      };
    };
    LeaveCriticalSection(&scr->lock);
    for(y= 0; y<24; y++) {
      a= InterlockedExchange(&scr->dirty[iy[y]], 80);
      if(dirty[y]==0) dirty[y]= a;
    };
  } else {
    do { // has scroll
      cc= 0; e= 0;
      EnterCriticalSection(&scr->lock);
      for(y= 23; y>=0; y--) {
        a= scr->copy[y];
        if(a<0) { //クリア
          if(cc==0) { //クリアは画面上から
            t= y;
            while(y>0) {
              if(scr->copy[y-1]>=0) break;
              y--;
            };
            b= y;
	  };
	  cc++;
        } else if(a!=y) { //スクロール
	  if(e>=0) { //上スクロール(e<0)は最初の１回、下スクロールは最後の１回
            t= y;
	    e= a-y;
            while(y>0) {
              if(scr->copy[y-1]!=e+y-1) break;
              y--;
            };
            b= y;
	  };
	  cc++;
        };
      };
      if(cc<=1) { //iyコピー
        for(y= 0; y<24; y++) {
          int w= y+scr->yscroll-scr->ybase; if(w<0) w+= scr->ymax;
          iy[y]= scr->iy[w];
        };
      };
      if(cc>0) { //tとbに何か代入
        for(y= b; y<=t; y++) scr->copy[y]= y;
      };
      LeaveCriticalSection(&scr->lock);
      if(e<0) { //上スクロール
        DEBUGMSG(1, (L"up %d-%d %d\n", t, b, -e));
	a= 0;
        for(y= b+e; y<=t; y++) if(a<useb[y]) a= useb[y];
        if(a>0) BitBlt(dc, winoffx, (23-t)*10, a*6, (t-b+1)*10, dc, winoffx, (23-(t+e))*10, SRCCOPY);
        for(y= t; y>=b; y--) useb[y]= useb[y+e];
      } else if(e>0) { //下スクロール
        DEBUGMSG(1, (L"down %d-%d %d\n", t, b, e));
        a= 0;
        for(y= t+e; y>=b; y--) if(a<useb[y]) a= useb[y];
        if(a>0) BitBlt(dc, winoffx, (23-t)*10, a*6, (t-b+1)*10, dc, winoffx, (23-(t+e))*10, SRCCOPY);
        for(y= b; y<=t; y++) useb[y]= useb[y+e];
      } else if(cc>0) { //クリアあり
        DEBUGMSG(1, (L"clear %d-%d\n", t, b));
        //RECT r;
        //r.left= 3; r.top= t*10; r.right= 483; r.bottom= b*10+10;
        //FillRect(dc, &r, basebrush);
      };
    } while(cc>1);
    for(y= 0; y<24; y++) {
      dirty[y]= InterlockedExchange(&scr->dirty[iy[y]], 80);
    };
  };

  //再描画:上からの方が2/120ほど結果的に速い
  for(y= 23; y>=0; y--) {
    t= dirty[y];
    b= t>>8; t&= 255;
    if(t<b) {
      if(b>=80) {
        WORD* tp= &scr->text[iy[y]][80];
	for(b= 80; b>0; b--) {
	  if(*--tp!=1) break;
	};
	if(useb[y]<=b) {
	  if(t<b) {
            draw(&scr->text[iy[y]][t], b-t, t, y);
            BitBlt(dc, winoffx+t*6, (23-y)*10, (b-t)*6, 10, bitdc, bitoffx+t*6, 0, SRCCOPY);
	  };
	  useb[y]= b;
	} else {
          if(t<useb[y]) {
	    if(t<b) {
              draw(&scr->text[iy[y]][t], b-t, t, y);
	      clear(useb[y]-b, b, y);
	    } else {
	      clear(useb[y]-t, t, y);
	    };
            BitBlt(dc, winoffx+t*6, (23-y)*10, (useb[y]-t)*6, 10, bitdc, bitoffx+t*6, 0, SRCCOPY);
	  };
	  useb[y]= b;
	};
      } else {
        if(useb[y]<b) useb[y]= b;
        draw(&scr->text[iy[y]][t], b-t, t, y);
        BitBlt(dc, winoffx+t*6, (23-y)*10, (b-t)*6, 10, bitdc, bitoffx+t*6, 0, SRCCOPY);
      };
    };
  };
};

//全画面再描画
void Draw::
redraw(Screen* scr, HDC dc)
{
  lastsel= 0;
  DEBUGMSG(1, (L"redraw\n"));
  int y, b;
  if(scr->iy==0) {
    for(y= 23; y>=0; y--) {
      clear(80, 0, y);
      BitBlt(dc, winoffx, (23-y)*10, 480, 10, bitdc, bitoffx, 0, SRCCOPY);
    };
    return;
  };
  EnterCriticalSection(&scr->lock);
  int sels= scr->selstart-scr->yscroll*80;
  int sele= scr->selend-scr->yscroll*80;
  for(y= 0; y<24; y++) {
    int w= y+scr->yscroll-scr->ybase; if(w<0) w+= scr->ymax;
    iy[y]= scr->iy[w];
    scr->copy[y]= y;
  };
  LeaveCriticalSection(&scr->lock);
  for(y= 0; y<24; y++) InterlockedExchange(&scr->dirty[iy[y]], 80);
  if(sels<=0||sele>=80*24||sels==sele) {
    sels= 0; sele= 80*24;
  } else {
    if(sels>80*24) sels= 80*24;
    if(sele<0) sele= 0;
  };
  for(y= 23; y>=0; y--) { //画面上から(見栄え)
    WORD* tp= &scr->text[iy[y]][80];
    for(b= 80; b>0; b--) {
      if(*--tp!=1) break;
    };
    if(y*80>=sels||y*80+80<=sele) {
      draw(&scr->text[iy[y]][0], b, 0, y);
      if(b<80) clear(80-b, b, y);
    } else { //セレクション内
      lastsel= 1; //画面上にセレクション表示あります
      WORD lbuf[82];
      memcpy(lbuf, &scr->text[iy[y]][0], sizeof(lbuf));
      int rs= 80-(sels-y*80);
      int re= 80-(sele-y*80);
      if(rs<0) rs= 0;
      if(re>80) re= 80;
      for(; rs<re; rs++) lbuf[rs]^= SC_REVERSE;
      draw(lbuf, 80, 0, y);
    };
    BitBlt(dc, winoffx, (23-y)*10, 480, 10, bitdc, bitoffx, 0, SRCCOPY);
    useb[y]= b;
  };
};


//カーソル表示
// (これを呼ぶ前にupdateかredrawでiy確定のこと)
void Draw::
cursor(Screen* scr, HDC dc, int mode)
{
  WORD t[2];
  if(mode) {//draw
    if(scr->iy==0||scr->sockmode!=1) { curx= -1; return; };
    if(scr->selstart) { curx= -1; return; }; //セレクション中は表示なし
    curx= scr->curx;
    cury= scr->cury-scr->yscroll;
    if(curx<0||cury>23||cury<0) {
      curx= -1; return;
    };
    if(curx>79) curx= 79;
    if(curx>=useb[cury]) {
      curb0= 1; curb= 1; curb1= 1;
    } else {
      curb0= 0; if(curx>0) curb0= scr->text[iy[cury]][curx-1];
      curb= scr->text[iy[cury]][curx];
      curb1= scr->text[iy[cury]][curx+1];
    };
  } else {
    if(curx<0) return;
  };
  if((curb&0x180)==0x180) { //漢字2バイト目
    if((curb0&0x180)==0x100) { //漢字
      t[0]= curb0, t[1]= curb;
      if(mode) t[0]^= SC_REVERSE, t[1]^= SC_REVERSE;
      draw(t, 2, curx-1, cury);
      BitBlt(dc, winoffx+curx*6, (23-cury)*10, 6, 10, bitdc, bitoffx+curx*6, 0, SRCCOPY);
    };
  } else if((curb&0x180)==0x100) {
    t[0]= curb; t[1]= curb1;
    if(mode) t[0]^= SC_REVERSE, t[1]^= SC_REVERSE;
    draw(t, 2, curx, cury);
    BitBlt(dc, winoffx+curx*6, (23-cury)*10, 12, 10, bitdc, bitoffx+curx*6, 0, SRCCOPY);
  } else {
    t[0]= curb;
    if(mode) t[0]^= SC_REVERSE;
    draw(t, 1, curx, cury);
    BitBlt(dc, winoffx+curx*6, (23-cury)*10, 6, 10, bitdc, bitoffx+curx*6, 0, SRCCOPY);
  };
  if(mode==0) curx= -1;
};

//ウィンドウ更新描画
void Draw::
updatewin(Screen* scr, HWND wdisp)
{
  if(scr->selstart||lastsel) {
    redrawwin(scr, wdisp);
    return;
  };
  HDC dc= GetDC(wdisp);
  RECT r;
  int mode= 1;
  if(picwidth==0
  &&GetClipBox(dc, &r)==SIMPLEREGION
  &&r.top<=0&&r.bottom>=240
  &&r.left<=winoffx&&r.right>=winoffx+480) {
    mode= 0; //スクロール可
  };
  cursor(scr, dc, 0);
  update(scr, dc, mode);
  cursor(scr, dc, 1);
  ReleaseDC(wdisp, dc);
};

//フレーム部分(-1:0を除く)
void Draw::
drawframe(Screen* scr, HDC dc)
{
  HBRUSH br;
  RECT r;
  if(scr->config.titleh) {
    br= GetSysColorBrush(COLOR_3DSHADOW);
    r.top= 0; r.bottom= 240; r.left= 483; r.right= 484; FillRect(dc, &r, br);
    r.top= scr->config.titletop+24+scr->config.titleh; r.bottom= 240; r.left= 0; r.right= 1; FillRect(dc, &r, br);
    r.top= 0; r.bottom= scr->config.titletop+24-1; FillRect(dc, &r, br);
    br= GetSysColorBrush(scr->config.black?COLOR_3DDKSHADOW:COLOR_3DLIGHT);
    r.left= 1; r.right= 2; FillRect(dc, &r, br);
    r.top= scr->config.titletop+24+scr->config.titleh+1; r.bottom= 240; FillRect(dc, &r, br);
    br= GetSysColorBrush(scr->config.black?COLOR_3DLIGHT:COLOR_3DDKSHADOW);
    r.top= 0; r.bottom= 240; r.left= 484; r.right= 485; FillRect(dc, &r, br);
    r.top= scr->config.titletop+24-1; r.bottom= scr->config.titletop+24+scr->config.titleh;
    r.left= 0; r.right= 1; FillRect(dc, &r, basebrush);
    r.bottom++; r.left= 1; r.right= 2; FillRect(dc, &r, basebrush);
  } else {
    br= GetSysColorBrush(COLOR_3DSHADOW);
    r.top= 0; r.bottom= 240; r.left= 0;   r.right= 1;   FillRect(dc, &r, br);
    r.left= 483; r.right= 484; FillRect(dc, &r, br);
    br= GetSysColorBrush(scr->config.black?COLOR_3DDKSHADOW:COLOR_3DLIGHT);
    r.left= 1; r.right= 2; FillRect(dc, &r, br);
    br= GetSysColorBrush(scr->config.black?COLOR_3DLIGHT:COLOR_3DDKSHADOW);
    r.left= 484; r.right= 485; FillRect(dc, &r, br);
  };  
};

//ウィンドウ全画面再描画
void Draw::
redrawwin(Screen* scr, HWND wdisp)
{
  HDC dc= GetDC(wdisp);
  if(winoffx>0) { //かざりあり
    if(picwidth==485) {
      BitBlt(dc, 0, 0, 3, 240, picdc, -3+bitoffx, 0, SRCCOPY);
      BitBlt(dc, 483, 0, 2, 240, picdc, 480+bitoffx, 0, SRCCOPY);
    } else if(picwidth==481) {
      BitBlt(dc, 2, 0, 1, 240, picdc, -1+bitoffx, 0, SRCCOPY);
      drawframe(scr, dc);
    } else { //480および0
      RECT r;
      r.top= 0; r.bottom= 240; r.left= 2; r.right= 3; FillRect(dc, &r, basebrush);
      drawframe(scr, dc);      
    };
  };
  redraw(scr, dc);
  cursor(scr, dc, 1);
  ReleaseDC(wdisp, dc);
};
