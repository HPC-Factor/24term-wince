// draw16.cpp: Draw16 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include "common.h"
#include "draw16.h"
#include "screen.h"

#define BD_LINE (488*2)

char* Draw16::patmask= 0;

long Draw16::colfore[16];

BYTE Draw16::bitmask[]= {
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00,
};

BYTE Draw16::revmask[]= {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff,
};

PCWSTR Draw16::
init(const Config* config, HWND wdisp)
{
  PCWSTR rs;
  rs= Draw::init(config, wdisp);
  if(rs[0]) return rs;

  colback= config->colback;
  colback= colback>>3&0x001f|colback>>5&0x07e0|colback>>8&0xf800;
  colback|= colback<<16;

  //pat
  if(patmask==0) {
    patmask= (char*)malloc(4*4*16*4);
    if(patmask==0) return L"aNo memory for patMask.";
  };

  int i, y;
  for(i= 0; i<16; i++) {
    long c1;
    c1= config->color[i];
    c1= c1>>3&0x001f|c1>>5&0x07e0|c1>>8&0xf800;
    c1|= c1<<16;
    colfore[i]= c1;
  };
  coluline= config->coluline;
  coluline= coluline>>3&0x001f|coluline>>5&0x07e0|coluline>>8&0xf800;
  coluline|= coluline<<16;

  char *p= patmask;
  for(i= 0; i<64; i++) {
    long c1, cc;
    c1= colfore[i/4];
    cc= colback;
    if(i&2) { //reverse
      cc= c1; c1= colback;
    };
    if(i&1) { //uline
      cc= config->coluline;
      cc= cc>>3&0x001f|cc>>5&0x07e0|cc>>8&0xf800;
      cc|= cc<<16;
    };
    cc^= c1;
    for(y= 0; y<4; y++) *(long*)p= *(long*)(bitmask+y*4)&cc^c1, p+= 4;
  };

  if(basebrush) DeleteObject(basebrush);
  basebrush= CreateSolidBrush(RGB(config->colback>>16&255, config->colback>>8&255, config->colback&255));
  if(basebrush==0) return L"aCannot create brush.";

  bitoffx= 4;
  if(bitdc==0) {
    BITMAPINFOHEADER *bi;
    bi= (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)+3*4);
    HDC dc= GetDC(wdisp);
    bi->biSize= sizeof(BITMAPINFOHEADER);
    bi->biWidth= BD_LINE/2;
    bi->biHeight= -10;
    bi->biPlanes= 1;
    bi->biBitCount= 16;
    bi->biCompression= BI_BITFIELDS;
    bi->biSizeImage= 0;
    bi->biXPelsPerMeter= 0;
    bi->biYPelsPerMeter= 0;
    bi->biClrUsed= 0;
    bi->biClrImportant= 0;
    ((DWORD*)((BITMAPINFO*)bi)->bmiColors)[0]= 0xf800;
    ((DWORD*)((BITMAPINFO*)bi)->bmiColors)[1]= 0x07e0;
    ((DWORD*)((BITMAPINFO*)bi)->bmiColors)[2]= 0x001f;
    bitmap= CreateDIBSection(dc, (BITMAPINFO*)bi, DIB_RGB_COLORS, (void**)&bitdata, 0, 0);
    free(bi);
    if(bitmap==0) { ReleaseDC(wdisp, dc); return L"bNo memory for bitmap."; };
    bitdata+= bitoffx*2; //offset
    bitdc= CreateCompatibleDC(dc);
    if(bitdc==0) { ReleaseDC(wdisp, dc); return L"bFailed to create bitmap."; };
    SelectObject(bitdc, bitmap);
    ReleaseDC(wdisp, dc);
  };

  return L"";

};

void Draw16::
clear(int vsn, int sx, int sy)
{
  space(vsn, bitdata+sx*12);
};

void Draw16::
space(int vsn, char* bd)
{
  long a= (long)colback;
  int bda;
  bda= vsn*12; bd+= bda; bda+= BD_LINE;
  if(vsn==1) {
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a;
  } else if(vsn==2) {
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd+= bda;
    bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a;
  } else {
    int x;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x); bd+= bda;
    x= vsn; do{ bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; bd-=4;*(long*)bd= a; }while(--x);
  };
};

void Draw16::
draw(const WORD* vs, int vsn, int sx, int sy)
{
  char *bd= bitdata+sx*12;
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
      bd+= nc*12;
    } else if(!(sc&SC_KANJI)) { //ANKS
      long c;
      char* pp;
      pp= patmask+(sc>>3&0x3c0);
      if(sc&SC_REVERSE) pp+= 0x20;
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
      bd+= 12;
      bd-=4;*(long*)bd=*(long*)(pp+(m0&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>2&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>4&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>10&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>12&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0<<2&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>20&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>6&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>8&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>14&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>16&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>18&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>24&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>26&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>28&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>2&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>4&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m0>>22&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>12&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1<<2&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>6&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>8&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>10&0xc)); bd+= BD_LINE+12;
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>16&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>18&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>20&0xc)); bd+= BD_LINE+12;
      if(sc&SC_ULINE) pp+= 16;
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>26&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>28&0xc));
      bd-=4;*(long*)bd=*(long*)(pp+(m1>>14&0xc));
      bd-= BD_LINE*9-12;
    } else if(!(sc&SC_KANJI2)&&vsn>0) { //漢字
      WORD sc2= *vs++&0x7f;
      vsn--;
      char* pp;
      pp= patmask+(sc>>3&0x3c0);
      if(sc&SC_REVERSE) pp+= 0x20;
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

      bd+= BD_LINE+24;
      m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
      if(sc&SC_BOLD) {
	m|= m>>1&~(m<<1)&0x7fe7fe;
	int y= 4; do {
	  m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc)); bd-= BD_LINE-24;
	  m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc)); bd+= BD_LINE*3+24;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	  m|= m>>1&~(m<<1)&0x7fe7fe;
	} while(--y);
      } else {
	int y= 4; do {
	  m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc)); bd-= BD_LINE-24;
	  m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
          m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc)); bd+= BD_LINE*3+24;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	} while(--y);
      };
      if(sc&SC_ULINE) {
        pp+= 16;
	m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
	pp-= 16;
      } else {
	m<<= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
        m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      };
      bd-= BD_LINE-24;
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      m>>= 2; bd-=4;*(long*)bd= *(long*)(pp+(m&0xc));
      bd-= BD_LINE*8-24;
    } else { //漢字2バイト目
      bd+= 12;
    };
  };
};
