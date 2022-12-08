// draw8.cpp: Draw8 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include "common.h"
#include "draw8.h"
#include "screen.h" //SC_XXX

#define BD_LINE 488
// #define ANKS1 むしろ遅い(2:40/2:43)

long Draw8::colfore[]= {
  0x0a0a0a0a, 0x0b0b0b0b, 0x0c0c0c0c, 0x0d0d0d0d,
  0x0e0e0e0e, 0x0f0f0f0f, 0x10101010, 0x11111111,
  0x12121212, 0x13131313, 0x14141414, 0x15151515,
  0x16161616, 0x17171717, 0x18181818, 0x19191919,
};
long Draw8::coluline= 0x1a1a1a1a;
long Draw8::colback= 0x1b1b1b1b;

BYTE Draw8::bitmask[]= {
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x00,
  0xff, 0xff, 0x00, 0xff,
  0xff, 0xff, 0x00, 0x00,
  0xff, 0x00, 0xff, 0xff,
  0xff, 0x00, 0xff, 0x00,
  0xff, 0x00, 0x00, 0xff,
  0xff, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xff, 0xff,
  0x00, 0xff, 0xff, 0x00,
  0x00, 0xff, 0x00, 0xff,
  0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00,
};

BYTE Draw8::revmask[]= {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0xff, 0xff,
  0x00, 0xff, 0x00, 0x00,
  0x00, 0xff, 0x00, 0xff,
  0x00, 0xff, 0xff, 0x00,
  0x00, 0xff, 0xff, 0xff,
  0xff, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0xff,
  0xff, 0x00, 0xff, 0x00,
  0xff, 0x00, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0xff,
  0xff, 0xff, 0xff, 0x00,
  0xff, 0xff, 0xff, 0xff,
};

HPALETTE Draw8::palette= 0;
LOGPALETTE *Draw8::logpal= 0;

char* Draw8::patmask= 0;
#ifdef ANKS1
char* Draw8::hanfont= 0;
#endif

PCWSTR Draw8::
init(const Config* config, HWND wdisp)
{
  PCWSTR rs;
  rs= Draw::init(config, wdisp);
  if(rs[0]) return rs;

  int i, y;
  char *p;

#ifdef ANKS1
  if(hanfont==0) {
    hanfont= (char*)malloc(96*80);
    if(hanfont==0) return L"aNo memory for hanfont.";

    //hanfontは定数
    char c0, c1;
    p= &hanfont[0];
    c0= (char)colback; c1= (char)colfore[0];
    for(i= 0; i<96; i++) {
      for(y= 5; y<60; y+= 6) {
        WORD a;
	if(i==95) { //7f=backslash
          a= "\0\0\40\20\10\4\2\1\0\0\0"[y/6];
	} else {
          const BYTE *bp;
          bp= (const BYTE*)(k12x10+255+i*8+y/8);
          a= (WORD)*bp<<8|(WORD)*(bp+1);
          a>>= 7-y%8;
	};
        if(a&32)*p++= c1; else *p++= c0;
        if(a&16)*p++= c1; else *p++= c0;
        if(a&8) *p++= c1; else *p++= c0;
        if(a&4) *p++= c1; else *p++= c0;
        if(a&8) *p++= c1; else *p++= c0;
        if(a&4) *p++= c1; else *p++= c0;
        if(a&2) *p++= c1; else *p++= c0;
        if(a&1) *p++= c1; else *p++= c0;
      };
    };
  };
#endif

  //pat
  if(patmask==0) {
    patmask= (char*)malloc(4*16*16*2);
    if(patmask==0) return L"ano memory for patMask.";

    //patMaskは定数
    p= patmask;
    for(i= 0; i<32; i++) {
      long c1, cc;
      c1= colfore[i/2]; cc= colback;
      if(i&1) { //reverse
        cc= c1; c1= colback;
      };
      cc^= c1;
      for(y= 0; y<16; y++) *(long*)p= *(long*)(bitmask+y*4)&cc^c1, p+= 4;
    };
  };

  //baseBrush
  if(basebrush==0) {
    basebrush= CreateSolidBrush(0x01000000|colback&0xff);
    if(basebrush==0) return L"aCannot create brush.";
  };

  //bitmap buffer
  bitoffx= 4;
  if(bitdc==0) {
    BITMAPINFOHEADER *bi;
    bi= (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)+1024);//bmiColorsは領域のみ必要
    HDC dc= GetDC(wdisp);
    bi->biSize= sizeof(BITMAPINFOHEADER);
    bi->biWidth= BD_LINE;
    bi->biHeight= -10;
    bi->biPlanes= 1;
    bi->biBitCount= 8;
    bi->biCompression= BI_RGB;
    bi->biSizeImage= 0;
    bi->biXPelsPerMeter= 0;
    bi->biYPelsPerMeter= 0;
    bi->biClrUsed= 0;
    bi->biClrImportant= 0;
    bitmap= CreateDIBSection(dc, (BITMAPINFO*)bi, DIB_PAL_COLORS, (void**)&bitdata, 0, 0);
    free(bi);
    if(bitmap==0) { ReleaseDC(wdisp, dc); return L"bNo memory for bitmap."; };
    bitdata+= bitoffx; //offset
    bitdc= CreateCompatibleDC(dc);
    if(bitdc==0) { ReleaseDC(wdisp, dc); return L"bFailed to create bitmap."; };
    SelectObject(bitdc, bitmap);
    ReleaseDC(wdisp, dc);
  };

  if(logpal==0) {
    logpal= (LOGPALETTE*)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*256);
    if(logpal==0) return L"sNo memory for palette.";
  };
  logpal->palVersion= 0;
  logpal->palNumEntries= 256;

  HDC dc= GetDC(0);
  GetSystemPaletteEntries(dc, 0, 256, &logpal->palPalEntry[0]);
  ReleaseDC(0, dc);

  int col;
  col= config->color[0];
  palset(0x0a, col);
  col= config->color[1];
  palset(0x0b, col);
  col= config->color[2];
  palset(0x0c, col);
  col= config->color[3];
  palset(0x0d, col);
  col= config->color[4];
  palset(0x0e, col);
  col= config->color[5];
  palset(0x0f, col);
  col= config->color[6];
  palset(0x10, col);
  col= config->color[7];
  palset(0x11, col);
  col= config->coluline;
  palset(0x1a, col);
  col= config->colback;
  palset(0x1b, col);
  
  if(palette==0) {
    palette= CreatePalette(logpal);
    if(palette==0) return L"sFailed to create palette.";
  } else {
    SetPaletteEntries(palette, 0, 256, &logpal->palPalEntry[0]);
  };

  return L"";
};

void Draw8::
palset(int n, int col)
{
  logpal->palPalEntry[n].peRed= (BYTE)(col>>16);
  logpal->palPalEntry[n].peGreen= (BYTE)(col>>8);
  logpal->palPalEntry[n].peBlue= (BYTE)(col);
  logpal->palPalEntry[n].peFlags= PC_RESERVED;
};

void Draw8::
clear(int vsn, int sx, int sy)
{
  space(vsn, bitdata+sx*6);
};

//空白を書く。
void Draw8::
space(int vsn, char* bd)
{
  long a= (long)colback; //white
  long bda;
  bda= vsn*6; bd+= bda; bda+= BD_LINE;
  if(vsn&1) {
    if((long)bd&2) { //aligned-misaligned
      if(vsn==1) { //1文字
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;
      } else {
        vsn= vsn>>1; int y= 10; do {
	  bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;
          int x= vsn; do{ bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; }while(--x); bd+= bda;
	} while(--y);
      };
    } else { //misaligned-aligned
      if(vsn==1) { //1文字
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a;
      } else {
        vsn= vsn>>1; int y= 10; do {
          int x= vsn; do{ bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; }while(--x);
	  bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	} while(--y);
      };
    };
  } else {
    if(!((long)bd&2)) { //aligned-aligned
      if(vsn==2) { //2文字
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd+= bda;
        bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a;
      } else {
        vsn= vsn>>1; int y= 10; do {
          int x= vsn; do{ bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; bd-=4;*(long*)bd=a; }while(--x); bd+= bda;
	} while(--y);
      };
    } else { //misaligned-misaligned
      if(vsn==2) { //2文字
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;	bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a;
      } else {
        vsn= (vsn>>1)-1; int y= 10; do {
	  bd-=2;*(short*)bd=(short)a; bd-=4;*(long*)bd=a;
	  int x= vsn; do{ bd-=4;*(long*)bd=a;bd-=4;*(long*)bd=a;bd-=4;*(long*)bd=a; }while(--x);
	  bd-=4;*(long*)bd=a; bd-=2;*(short*)bd=(short)a; bd+= bda;
	} while(--y);
      };
    };
  };
};

//画面へ展開する。
void Draw8::
draw(const WORD* vs, int vsn, int sx, int sy)
{
  char *bd= bitdata+sx*6;
  WORD sc;
  while(vsn>0) {
    sc= *vs++; vsn--;
    if((sc&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {//space
      int nc= vsn;
      while(vsn>0&&(*vs&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {
	vs++; vsn--;
      };
      nc= nc-vsn+1;
      space(nc, bd);
      bd+= nc*6;
#ifdef ANKS1
    } else if(sc<0x80) { //anks normal color
      char* cd= hanfont+(sc-32)*80;
      if(!((long)bd&2)) { //aligned
        bd+= 6;
	bd-=2;*(short*)bd= *(short*)(cd+6);  bd-=4;*(long*)bd= *(long*)(cd);    bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+14); bd-=4;*(long*)bd= *(long*)(cd+8);  bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+22); bd-=4;*(long*)bd= *(long*)(cd+16); bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+30); bd-=4;*(long*)bd= *(long*)(cd+24); bd+= BD_LINE+6;
	cd+= 32;
	bd-=2;*(short*)bd= *(short*)(cd+6);  bd-=4;*(long*)bd= *(long*)(cd);    bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+14); bd-=4;*(long*)bd= *(long*)(cd+8);  bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+22); bd-=4;*(long*)bd= *(long*)(cd+16); bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+30); bd-=4;*(long*)bd= *(long*)(cd+24); bd+= BD_LINE+6;
	cd+= 32;
	bd-=2;*(short*)bd= *(short*)(cd+6);  bd-=4;*(long*)bd= *(long*)(cd);    bd+= BD_LINE+6;
	bd-=2;*(short*)bd= *(short*)(cd+14); bd-=4;*(long*)bd= *(long*)(cd+8);
	bd-= BD_LINE*9-6;
      } else { //misaligned
	bd+= 6;
	bd-=4;*(long*)bd= *(long*)(cd+4);  bd-=2;*(short*)bd= *(short*)(cd);    bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+12); bd-=2;*(short*)bd= *(short*)(cd+8);  bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+20); bd-=2;*(short*)bd= *(short*)(cd+16); bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+28); bd-=2;*(short*)bd= *(short*)(cd+24); bd+= BD_LINE+6;
	cd+= 32;
	bd-=4;*(long*)bd= *(long*)(cd+4);  bd-=2;*(short*)bd= *(short*)(cd);    bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+12); bd-=2;*(short*)bd= *(short*)(cd+8);  bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+20); bd-=2;*(short*)bd= *(short*)(cd+16); bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+28); bd-=2;*(short*)bd= *(short*)(cd+24); bd+= BD_LINE+6;
	cd+= 32;
	bd-=4;*(long*)bd= *(long*)(cd+4);  bd-=2;*(short*)bd= *(short*)(cd);    bd+= BD_LINE+6;
	bd-=4;*(long*)bd= *(long*)(cd+12); bd-=2;*(short*)bd= *(short*)(cd+8);
	bd-= BD_LINE*9-6;
      };
#endif
    } else if(!(sc&SC_KANJI)) { //ANKS 色つき
      long c;
      char const* pp;
      pp= patmask+(sc>>2&0x780);
      if(sc&SC_REVERSE) pp+= 0x40;
      c= sc&0xff;
      DWORD m0, m1;
      if(c>=0xe0) c= 0;
      else if(c>=0xa0) c-= 0x40;
      else if(c>=0x80) c= 0;
      else if(c>=0x20) c-= 0x20;
      else c= 0;
      if(!(sc&SC_BOLD)) {
        if(c==0x5f) { //backslash
          m0= 0x20100800; m1= 0x00008040;
        } else {
          const char *cd;
          cd= (const char*)k12x10+256+c*8;
          m0= *(DWORD*)cd; m1= *(DWORD*)(cd+4);
	};
      } else { //bold
        //DWORD m0t= m0>>17&0x00000080;
        //m0|= (m0>>1&0x7f7f7f7f|m0<<15&0x80808000)
	//&~(m0<<1&0xfefefefe|m0>>15&0x00010101|m1<<17&0x01000000)&0x799ee779;
	//m1|= (m1>>1&0x7f7f7f7f|m1<<15&0x80808000|m0t)
	//&~(m1<<1&0xfefefefe|m1>>15&0x00010101)&0xe0799ee7;
        const char *cd;
        cd= (const char*)boldanks+c*8;
        m0= *(DWORD*)cd; m1= *(DWORD*)(cd+4);
      };
      if(!((long)bd&2)) {
        bd+= 6;
        bd-=2;*(short*)bd=*(short*)(pp+(m0<<2&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>2&0x3c));  bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>8&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0<<4&0x30|m0>>12&0xc)); bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>18&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>6&0x3c));  bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>12&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>16&0x3c)); bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>22&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>26&0x3c)); bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m1&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>20&0x30|m1>>4&0xc)); bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m1>>10&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m1<<2&0x3c));  bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m1>>4&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m1>>8&0x3c));  bd+= BD_LINE+6;
        bd-=2;*(short*)bd=*(short*)(pp+(m1>>14&0x30));
        bd-=4; *(long*)bd= *(long*)(pp+(m1>>18&0x3c)); bd+= BD_LINE+6;
	if(!(sc&SC_ULINE)) {
          bd-=2;*(short*)bd=*(short*)(pp+(m1>>24&0x30));
          bd-=4; *(long*)bd= *(long*)(pp+(m1>>12&0x30|m1>>28&0xc));
	} else {
	  long c1, cu;
	  c1= colfore[sc>>9&0xf]; if(sc&SC_REVERSE) c1= colback;
	  cu= coluline^c1;
          c=*(short*)(bitmask+(m1>>24&0x30));           bd-=2;*(short*)bd= (short)(c&cu^c1);
          c= *(long*)(bitmask+(m1>>12&0x30|m1>>28&0xc));bd-=4; *(long*)bd= c&cu^c1;
	};
        bd-= BD_LINE*9-6;
      } else {
        bd+= 6;
        bd-=4; *(long*)bd= *(long*)(pp+(m0&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>2&0x30));  bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>10&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m0<<4&0x30));  bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>4&0x30|m0>>20&0xc));
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>6&0x30));  bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>14&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>16&0x30)); bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m0>>24&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>26&0x30)); bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m1>>2&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m0>>20&0x30)); bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m1<<4&0x30|m1>>12&0xc));
        bd-=2;*(short*)bd=*(short*)(pp+(m1<<2&0x30));  bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m1>>6&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m1>>8&0x30));  bd+= BD_LINE+6;
        bd-=4; *(long*)bd= *(long*)(pp+(m1>>16&0x3c));
        bd-=2;*(short*)bd=*(short*)(pp+(m1>>18&0x30)); bd+= BD_LINE+6;
	if(!(sc&SC_ULINE)) {
          bd-=4; *(long*)bd= *(long*)(pp+(m1>>26&0x3c));
          bd-=2;*(short*)bd=*(short*)(pp+(m1>>12&0x30));
	} else {
	  long c1, cu;
	  c1= colfore[sc>>9&0xf]; if(sc&SC_REVERSE) c1= colback;
	  cu= coluline^c1;
          c= *(long*)(bitmask+(m1>>26&0x3c)); bd-=4; *(long*)bd= c&cu^c1;
          c=*(short*)(bitmask+(m1>>12&0x30)); bd-=2;*(short*)bd= (short)(c&cu^c1);
	};
        bd-= BD_LINE*9-6;
      };
    } else if(!(sc&SC_KANJI2)&&vsn>0) { //漢字
      WORD sc2= *vs++&0x7f;
      vsn--;
      long c;
      char const* pp;
      pp= patmask+(sc>>2&0x780);
      if(sc&SC_REVERSE) pp+= 0x40;
      long m;
      m= sc&0x7f;
      if(m>=84) m= 128;
      else if(m>=15) m-= 6;
      else if(m>=13) m= 128;
      else if(m==12) m= 8;
      else if(m>=8) m= 128;
      if(sc2>=94) m= 128;
      const char *cd;
      if(m&128) cd= (const char*)k12x10+1536; //invalid kanji
      else cd= (const char*)k12x10+1536+(m*94+sc2)*15;
      if(!((long)bd&2)) { //aligned
        bd+= BD_LINE+12;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
            m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd-= BD_LINE-12;
            m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd+= BD_LINE*3+12;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
	} else {
          m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd-= BD_LINE-12;
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd-= BD_LINE-12;
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd-= BD_LINE-12;
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd-= BD_LINE-12;
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	};
	if(!(sc&SC_ULINE)) {
          m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
	} else {
	  long c1, cu;
	  c1= colfore[sc>>9&0xf]; if(sc&SC_REVERSE) c1= colback;
	  cu= coluline^c1;
          m<<= 2; c= *(long*)(bitmask+(m&0x3c)); bd-=4;*(long*)bd= (long)(c&cu^c1);
          m>>= 4; c= *(long*)(bitmask+(m&0x3c)); bd-=4;*(long*)bd= (long)(c&cu^c1);
          m>>= 4; c= *(long*)(bitmask+(m&0x3c)); bd-=4;*(long*)bd= (long)(c&cu^c1);
	};
        bd-= BD_LINE-12;
        m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
        m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
        m>>= 4; bd-=4;*(long*)bd= *(long*)(pp+(m&0x3c));
        bd-= BD_LINE*8-12;
      } else {
        bd+= BD_LINE+12;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
  	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
            m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
            m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd-= BD_LINE-12;
            m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
            m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
            m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd+= BD_LINE*3+12;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
  	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
        } else {
          m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd-= BD_LINE-12;
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd-= BD_LINE-12;
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd-= BD_LINE-12;
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd-= BD_LINE-12;
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30)); bd+= BD_LINE*3+12;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	};
	if(!(sc&SC_ULINE)) {
          m<<= 4; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
          m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
	} else {
	  long c1, cu;
	  c1= colfore[sc>>9&0xf]; if(sc&SC_REVERSE) c1= colback;
	  cu= coluline^c1;
          m<<= 4; c=*(short*)(bitmask+(m&0x30)); bd-=2;*(short*)bd= (short)(c&cu^c1);
          m>>= 4; c= *(long*)(bitmask+(m&0x3c)); bd-=4; *(long*)bd= (long)(c&cu^c1);
          m>>= 4; c= *(long*)(bitmask+(m&0x3c)); bd-=4; *(long*)bd= (long)(c&cu^c1);
          m>>= 2; c=*(short*)(bitmask+(m&0x30)); bd-=2;*(short*)bd= (short)(c&cu^c1);
	};
	bd-= BD_LINE-12;
        m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
        m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
        m>>= 4; bd-=4; *(long*)bd= *(long*)(pp+(m&0x3c));
        m>>= 2; bd-=2;*(short*)bd=*(short*)(pp+(m&0x30));
        bd-= BD_LINE*8-12;
      };
    } else { //漢字2バイト目
      bd+= 6;
    };
  };
};
