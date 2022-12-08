// draw2.cpp: Draw2 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include "common.h"
#include "screen.h"

#include "draw2.h"

//横幅512pix/4
#define BD_LINE 128

extern Screen* theScreen; //設定値の読み取りのみ

char* Draw2::patmask= 0;
char* Draw2::colpat[16];

PCWSTR Draw2::
init(const Config* config, HWND wdisp)
{
  PCWSTR rs;
  rs= Draw::init(config, wdisp);
  if(rs[0]) return rs;

  int cb;
  cb= config->colback;
  colback= (((cb>>16&255)*30+(cb>>8&255)*59+(cb&255)*11+50)/6400) * 0x55;

  int c= config->coluline;
  int d= colback&3;
  if(c!=cb) { //color differ
    c= (c>>16&255)*30+(c>>8&255)*59+(c&255)*11+50;
    if(c/6400!=d) {
      d= c/6400;
    } else {
      if(c>(cb>>16&255)*30+(cb>>8&255)*59+(cb&255)*11+50) { //明るい
        d++;
      } else {
        d--;
      };
    };
  };
  if(d<0) d= 0; if(d>3) d= 3;
  d*= 0x55; //underline color

  if(patmask==0) {
    patmask= (char*)malloc(32*4*4);
    if(patmask==0) return L"aNo memory for patmask.";
  };

  int i, y;
  char *p= patmask;
  for(i= 0; i<16; i++) {
    int forec= (i/4)*0x55;
    int backc= colback;
    if(i&2) { //reverse
      backc= forec;
      forec= colback;
    };
    if(i&1) backc= d; //uline
    for(y= 0; y<16; y++) {
      int c= y<<4&0x80|y<<3&0x40|y<<3&0x20|y<<2&0x10|y<<2&8|y<<1&4|y<<1&2|y&1;
      *p++= c&forec|~c&backc;
    };
    for(y= 0; y<4; y++) {
      int c= y<<6&0x80|y<<5&0x40|y<<5&0x20|y<<4&0x10;
      *p++= (c&forec|~c&backc)&0xf0;
    };
    for(y= 0; y<4; y++) {
      int c= y<<2&8|y<<1&4|y<<1&2|y&1;
      *p++= (c&forec|~c&backc)|0xf0;
    };
    for(y= 0; y<8; y++) *p++= 0;
  };

  for(i= 0; i<16; i++) {
    c= config->color[i];
    d= colback&3;
    if(c!=cb) { //color differ
      c= (c>>16&255)*30+(c>>8&255)*59+(c&255)*11+50;
      if(c/6400!=d) {
        d= c/6400;
      } else {
        if(c>(cb>>16&255)*30+(cb>>8&255)*59+(cb&255)*11+50) { //明るい
	  d++;
	} else {
	  d--;
	};
      };
    };
    if(d<0) d= 0; if(d>3) d= 3;
    colpat[i]= patmask+d*128;
  };

  if(basebrush) DeleteObject(basebrush);
  basebrush= CreateSolidBrush(0x01000000|colback&3);
  if(basebrush==0) return L"aCannot create brush.";


  //bitmap buffer
  if(devwidth==480) bitoffx= 16; else bitoffx= 8;
  if(bitdc==0) {
    BITMAPINFOHEADER *bi;
    bi= (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)+4*4);//bmiColorsは領域のみ必要
    HDC dc= GetDC(wdisp);
    bi->biSize= sizeof(BITMAPINFOHEADER);
    bi->biWidth= BD_LINE*4;
    bi->biHeight= -10;
    bi->biPlanes= 1;
    bi->biBitCount= 2;
    bi->biCompression= BI_RGB;
    bi->biSizeImage= 0;
    bi->biXPelsPerMeter= 0;
    bi->biYPelsPerMeter= 0;
    bi->biClrUsed= 0;
    bi->biClrImportant= 0;
    bitmap= CreateDIBSection(dc, (BITMAPINFO*)bi, DIB_PAL_COLORS, (void**)&bitdata, 0, 0);
    free(bi);
    if(bitmap==0) { ReleaseDC(wdisp, dc); return L"bNo memory for bitmap."; };
    bitdata+= bitoffx/4; //offset
    bitdc= CreateCompatibleDC(dc);
    if(bitdc==0) { ReleaseDC(wdisp, dc); return L"bFailed to create bitmap."; };
    SelectObject(bitdc, bitmap);
    ReleaseDC(wdisp, dc);
  };

  return L"";
};

void Draw2::
clear(int vsn, int sx, int sy)
{
  space(vsn, bitdata+sx*3/2, sx&1);
};

void Draw2::
space(int vsn, char* bd, int sx)
{
  char a, al, ah;
  long bda;

  a= colback;
  bda= vsn*3>>1; bd+= bda; bda+= BD_LINE;
  if(vsn&1) {
    if(!sx) { // aligned-misaligned
      ah= a&0xf0;
      if(vsn==1) { //1文字
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd= a;
      } else {
        vsn= vsn>>1; int y= 10; do {
	  *bd= *bd&0x0f|ah; *--bd= a;
	  int x= vsn; do{ *--bd=a; *--bd=a; *--bd=a; }while(--x);
	  bd+= bda;
	} while(--y);
      };
    } else { // misaligned-aligned
      al= a|0xf0;
      if(vsn==1) { //1文字
        bda--;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
	*bd= a; bd[-1]= (bd[-1]|0x0f)&al;
      } else {
        vsn= vsn>>1; int y= 10; do {
	  *bd= a;
	  int x= vsn; do{ *--bd=a; *--bd=a; *--bd=a; }while(--x);
	  bd--; *bd= (*bd|0x0f)&al; bd+= bda;
	} while(--y);
      };
    };
  } else {
    if(!sx) { // aligned-aligned
      if(vsn==2) { //2文字
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a; bd+= bda;
        *--bd= a; *--bd= a; *--bd= a;
      } else {
        vsn= vsn>>1; int y= 10; do {
	  int x= vsn; do{ *--bd=a; *--bd=a; *--bd=a; }while(--x);
	  bd+= bda;
	} while(--y);
      };
    } else { // misaligned-misaligned
      ah= a&0xf0; al= a|0xf0;
      if(vsn==2) { //2文字
        bda--;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al; bd+= bda;
        *bd= *bd&0x0f|ah; *--bd=a; *--bd=a; bd[-1]= (bd[-1]|0x0f)&al;
      } else {
        vsn= (vsn>>1)-1; int y= 10; do {
	  *bd= *bd&0x0f|ah; *--bd=a; *--bd=a;
	  int x= vsn; do{ *--bd=a; *--bd=a; *--bd=a; }while(--x);
	  bd--; *bd= (*bd|0x0f)&al; bd+= bda;
	} while(--y);
      };
    };
  };
};

void Draw2::
draw(const WORD* vs, int vsn, int sx, int sy)
{
  char* bd= bitdata+sx*3/2;
  sx&= 1;
  WORD sc;
  while(vsn>0) {
    sc= *vs++; vsn--;
    if((sc&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {//space
      int nc= vsn;
      while(vsn>0&&(*vs&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {
	vs++; vsn--;
      };
      nc= nc-vsn+1;
      space(nc, bd, sx);
      bd+= nc*3/2; if(nc&sx) bd++;
      sx^= nc&1;
    } else if(!(sc&SC_KANJI)) { //ANKS
      long c;
      char* pp;
      pp= colpat[sc>>9&15];
      if(sc&SC_REVERSE) pp+= 64;
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
      if(!sx) {
	char* pph= pp+16;
	bd++;
        *bd= *bd&0x0f|pph[m0>>2&3];  *--bd= pp[m0>>4&15];          bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m0>>12&3]; *--bd= pp[m0<<2&12|m0>>14&3]; bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m0>>22&3]; *--bd= pp[m0>>8&15];          bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m0>>16&3]; *--bd= pp[m0>>18&15];         bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m0>>26&3]; *--bd= pp[m0>>28&15];         bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m1>>4&3];  *--bd= pp[m0>>22&12|m1>>6&3]; bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m1>>14&3]; *--bd= pp[m1&15];             bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m1>>8&3];  *--bd= pp[m1>>10&15];         bd+= BD_LINE+1;
        *bd= *bd&0x0f|pph[m1>>18&3]; *--bd= pp[m1>>20&15];         bd+= BD_LINE+1;
	if(sc&SC_ULINE) { pp+= 32; pph+= 32; };
        *bd= *bd&0x0f|pph[m1>>28&3]; *--bd= pp[m1>>14&12|m1>>30&3];
        bd-= BD_LINE*9-1; sx= 1;
      } else {
        char* ppl= pp+20;
        bd[1]= pp[m0>>2&15];          *bd= (*bd|0x0f)&ppl[m0>>6&3];  bd+= BD_LINE;
        bd[1]= pp[m0>>12&15];         *bd= (*bd|0x0f)&ppl[m0&3];     bd+= BD_LINE;
        bd[1]= pp[m0>>6&12|m0>>22&3]; *bd= (*bd|0x0f)&ppl[m0>>10&3]; bd+= BD_LINE;
        bd[1]= pp[m0>>16&15];         *bd= (*bd|0x0f)&ppl[m0>>20&3]; bd+= BD_LINE;
        bd[1]= pp[m0>>26&15];         *bd= (*bd|0x0f)&ppl[m0>>30&3]; bd+= BD_LINE;
        bd[1]= pp[m1>>4&15];          *bd= (*bd|0x0f)&ppl[m0>>24&3]; bd+= BD_LINE;
        bd[1]= pp[m1<<2&12|m1>>14&3]; *bd= (*bd|0x0f)&ppl[m1>>2&3];  bd+= BD_LINE;
        bd[1]= pp[m1>>8&15];          *bd= (*bd|0x0f)&ppl[m1>>12&3]; bd+= BD_LINE;
        bd[1]= pp[m1>>18&15];         *bd= (*bd|0x0f)&ppl[m1>>22&3]; bd+= BD_LINE;
	if(sc&SC_ULINE) { pp+= 32; ppl+= 32; };
	bd[1]= pp[m1>>28&15];         *bd= (*bd|0x0f)&ppl[m1>>16&3];
	bd-= BD_LINE*9-2; sx= 0;
      };
    } else if(!(sc&SC_KANJI2)&&vsn>0) { //漢字
      WORD sc2= *vs++&0x7f;
      vsn--;
      char* pp;
      pp= colpat[sc>>9&15];
      if(sc&SC_REVERSE) pp+= 64;
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
      if(!sx) { //aligned
        bd+= BD_LINE+3;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
  	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
	    *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	    *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd+= BD_LINE*3+3;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
  	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
	} else {
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd+= BD_LINE*3+3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd+= BD_LINE*3+3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd+= BD_LINE*3+3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd+= BD_LINE*3+3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
        };
	if(sc&SC_ULINE) {
	  pp+= 32;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  pp-= 32;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE*8-3;
	} else {
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE-3; m>>=4;
	  *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; m>>=4; *--bd= pp[m&15]; bd-= BD_LINE*8-3;
	};
      } else {
	char* pph= pp+16;
	char* ppl= pp+20;
        bd+= BD_LINE;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
  	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
            bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
            bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd+= BD_LINE*3;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
  	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
	} else {
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	};
	if(sc&SC_ULINE) {
	  pp+= 32;
          bd[3]= bd[3]&0x0f|pph[(m&3)+32]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[(m&3)+32]; bd-= BD_LINE; m>>=2;
	  pp-= 32;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE*8-3;
	} else {
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE; m>>=2;
          bd[3]= bd[3]&0x0f|pph[m&3]; m>>=2; bd[2]= pp[m&15]; m>>=4; bd[1]= pp[m&15]; m>>=4; *bd= (*bd|0x0f)&ppl[m&3]; bd-= BD_LINE*8-3;
	};
      };
    } else { //漢字2バイト目
      if(!sx) {
        bd++; sx= 1;
      } else {
        bd+= 2; sx= 0;
      };
    };
  };
};
