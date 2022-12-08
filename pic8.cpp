// pic8.cpp: Pic8 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include "common.h"
#include "pic8.h"
#include "screen.h" //SC_XXX

#define BD_LINE 488

HBITMAP Pic8::picmap;
char* Pic8::picdata;
int* Pic8::picpal= 0;

PCWSTR Pic8::
init(const Config* config, HWND wdisp)
{
  PCWSTR rs;
  rs= Draw8::init(config, wdisp);
  if(rs[0]) return(rs);

  //picmap
  if(picdc==0) {
    BITMAPINFOHEADER *bi;
    bi= (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)+1024);//bmiColorsは領域のみ必要
    HDC dc= GetDC(wdisp);
    bi->biSize= sizeof(BITMAPINFOHEADER);
    bi->biWidth= BD_LINE;
    bi->biHeight= -240;
    bi->biPlanes= 1;
    bi->biBitCount= 8;
    bi->biCompression= BI_RGB;
    bi->biSizeImage= 0;
    bi->biXPelsPerMeter= 0;
    bi->biYPelsPerMeter= 0;
    bi->biClrUsed= 0;
    bi->biClrImportant= 0;
    picmap= CreateDIBSection(dc, (BITMAPINFO*)bi, DIB_PAL_COLORS, (void**)&picdata, 0, 0);
    free(bi);
    if(picmap==0) { ReleaseDC(wdisp, dc); return L"bNo memory for picture."; };
    picdata+= bitoffx; //offset
    picdc= CreateCompatibleDC(dc);
    if(picdc==0) { ReleaseDC(wdisp, dc); return L"bFailed to create picture."; };
    SelectObject(picdc, picmap);
    ReleaseDC(wdisp, dc);
  };
  if(picpal==0) {
    picpal= (int*)malloc(256*sizeof(int));
    if(picpal==0) return L"mFailed to create picture palette.";
  };

  WCHAR picp[_MAX_PATH];
  if(config->pic[0]!='\\'&&config->file[0]=='\\') { //add path
    int a= wcsrchr(config->file, L'\\')-config->file+1;
    wcsncpy(picp, config->file, a);
    wcscpy(picp+a, config->pic);
  } else {
    wcscpy(picp, config->pic);
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
  palset(0x2e, config->gammacolor(0x000000));
  palset(0x2f, config->gammacolor(0xffffff));
  int pal[256];
  int i;
  picpaln= 0;
  for(i= 0; i<(int)bih.biClrUsed; i++) {
    DWORD col;
    ReadFile(pich, &col, sizeof(col), &dd, 0);
    if((col&0xffffff)==0) { //black
      pal[i]= 0x2e;
    } else if((col&0xffffff)==0xffffff) { //white
      pal[i]= 0x2f;
    } else if(picpaln<128) {
      picpal[picpaln]= col;
      palset(0x30+picpaln, config->gammacolor(col));
      pal[i]= 0x30+picpaln;
      picpaln++;
    } else {
      pal[i]= 0x1b;
    };
  };
  DEBUGMSG(1, (L"picpaln=%d\n", picpaln));
  for(; i<256; i++) pal[i]= 0x1b;

  int y;
  BYTE *p;
  if(bih.biWidth>=485) {
    picwidth= 485;
    p= (BYTE*)picdata-3;
  } else if(bih.biWidth>=481) {
    picwidth= 481;
    p= (BYTE*)picdata-1;
  } else {
    picwidth= 480;
    p= (BYTE*)picdata;
  };
  for(y= 0; y<240; y++) {
    int y0= y;
    if(bih.biHeight>0) y0= bih.biHeight-y-1;
    SetFilePointer(pich, bfh.bfOffBits+(bih.biWidth+3)/4*4*y0, 0, FILE_BEGIN);
    ReadFile(pich, p, picwidth, &dd, 0);
    for(i= 0; i<picwidth; i++) {
      *p= pal[*p];
      p++;
    };
    p+= BD_LINE-picwidth;
  };
  CloseHandle(pich);

  SetPaletteEntries(palette, 0, 256, &logpal->palPalEntry[0]);

  return L"";
};

void Pic8::
clear(int vsn, int sx, int sy)
{
  space(vsn, bitdata+sx*6, picdata+sx*6+(23-sy)*(10*BD_LINE));
};

//空白を書く。
void Pic8::
space(int vsn, char* bd, const char* pic)
{
  int bda, pica;
  bda= vsn*6; bd+= bda;
  if(vsn&1) {
    if((long)bd&2) { //aligned-misaligned
      if(vsn==1) { //1文字
        bda+= BD_LINE; pica= bda-10;
        long d;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d; bd+= bda; pic+= pica;
	d= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d;
      } else {
        pic+= bda; pic-= 6;
	bda+= BD_LINE; pica= bda+6;
        vsn= vsn>>1; int y= 10; do {
	  long d, d0, d1; 
	  d= *(long*)pic;pic+= 4; bd-=2;*(short*)bd= *(short*)pic; bd-= 4;*(long*)bd= d; pic-= 16;
          int x= vsn; do{ d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd=*(long*)pic; bd-=4;*(long*)bd=d1; bd-=4;*(long*)bd=d0; pic-= 20; }while(--x);
          bd+= bda; pic+= pica;
        } while(--y);
      };
    } else { //misaligned-aligned
      if(vsn==1) { //1文字
        bda+= BD_LINE; pica= bda-8;
        short d;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d;
      } else {
        pic+= bda; pic-= 12;
	bda+= BD_LINE; pica= bda-14;
        vsn= vsn>>1; int y= 10; do {
	  short d;
	  long d0, d1;
          int x= vsn; do{ d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd=*(long*)pic; bd-=4;*(long*)bd=d1; bd-=4;*(long*)bd=d0; pic-= 20; }while(--x);
	  pic+= 6; d= *(short*)pic;pic+=2; bd-=4;*(long*)bd= *(long*)pic; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        } while(--y);
      };
    };
  } else {
    if(!((long)bd&2)) { //aligned-aligned
      if(vsn==2) { //2文字
        bda+= BD_LINE; pica= bda-20;
        long d0, d1;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; bd+= bda; pic+= pica;
	d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd= *(long*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0;
      } else {
        pic+= bda; pic-= 12;
	bda+= BD_LINE;
        vsn= vsn>>1; int y= 10; do {
	  long d0, d1;
          int x= vsn; do{ d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd=*(long*)pic; bd-=4;*(long*)bd=d1; bd-=4;*(long*)bd=d0; pic-=20; }while(--x);
          bd+= bda; pic+= bda;
        } while(--y);
      };
    } else { //misaligned-misaligned
      if(vsn==2) { //2文字
        bda+= BD_LINE; pica= bda-22;
	short d;
	long d1, d2;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d; bd+= bda; pic+= pica;
        d= *(short*)pic;pic+=2; d1= *(long*)pic;pic+=4; d2= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4; *(long*)bd= d2; bd-=4;*(long*)bd= d1; bd-=2;*(short*)bd= d;
      } else {
        pic+= bda; pic-= 10;
	bda+= BD_LINE; pica= bda-10;
        vsn= (vsn>>1)-1; int y= 10; do {
	  long d0, d1;
	  d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=2;*(short*)bd= *(short*)pic; bd-=4;*(long*)bd= d1; bd-=4;*(long*)bd= d0; pic-= 20; //doに入れるとロードストア連続で遅い
          int x= vsn; do{ d0= *(long*)pic;pic+=4; d1= *(long*)pic;pic+=4; bd-=4;*(long*)bd=*(long*)pic; bd-=4;*(long*)bd=d1; bd-=4;*(long*)bd=d0; pic-= 20; }while(--x);
	  pic+= 10; bd-=2;*(short*)bd= *(short*)pic; bd+= bda; pic+= pica;
        } while(--y);
      };
    };
  };
};

void Pic8::
draw(const WORD* vs, int vsn, int sx, int sy)
{
  char* bd= bitdata+sx*6;
  const char* pic= picdata+sx*6+(23-sy)*(10*BD_LINE);
  WORD sc;
  while(vsn>0) {
    sc= *vs++; vsn--;
    if((sc&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {//space
      int n= vsn;
      while(vsn>0&&(*vs&~(SC_COLOR|SC_BLINK|SC_BOLD))<=0x0020) {
	vs++; vsn--;
      };
      n= n-vsn+1;
      space(n, bd, pic);
      bd+= n*6; pic+= n*6;
    } else if(!(sc&SC_KANJI)) { //anks
      BYTE const* mask;
      long c, c1;
      c1= colfore[sc>>9&0xf];
      mask= bitmask;
      if(sc&SC_REVERSE) mask= revmask;
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
        c= *(short*)(mask+(m0<<2&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0>>2&0x3c));  bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m0>>8&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0<<4&0x30|m0>>12&0xc));bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m0>>18&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0>>6&0x3c));  bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m0>>12&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0>>16&0x3c)); bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m0>>22&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0>>26&0x3c)); bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m1&0x30));    bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m0>>20&0x30|m1>>4&0xc));bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m1>>10&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m1<<2&0x3c));  bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m1>>4&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m1>>8&0x3c));  bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(short*)(mask+(m1>>14&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
        c= *(long*)(mask+(m1>>18&0x3c)); bd-=4;*(long*)bd=          (*(long*)pic^c1)&c^c1; bd+= BD_LINE+6; pic+= BD_LINE;
	if((sc&(SC_ULINE|SC_REVERSE))==SC_ULINE) {
	  int cu= coluline^c1;
          c= *(short*)(mask+(m1>>24&0x30));bd-=2;*(short*)bd= (short)(c&cu^c1);
          c= *(long*)(mask+(m1>>12&0x30|m1>>28&0xc));bd-=4;*(long*)bd= c&cu^c1;
	} else {
	  if(sc&SC_ULINE) c1= coluline;
          c= *(short*)(mask+(m1>>24&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+4)^c1)&c^c1);
          c= *(long*)(mask+(m1>>12&0x30|m1>>28&0xc));bd-=4;*(long*)bd=(*(long*)pic^c1)&c^c1;
	};
        bd-= BD_LINE*9-6; pic-= BD_LINE*9-6;
      } else {
        bd+= 6; pic-= 2;
        c= *(long*)(mask+(m0&0x3c));     bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0>>2&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m0>>10&0x3c)); bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0<<4&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m0>>4&0x30|m0>>20&0xc));bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0>>6&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m0>>14&0x3c)); bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0>>16&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m0>>24&0x3c)); bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0>>26&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m1>>2&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m0>>20&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m1<<4&0x30|m1>>12&0xc));bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m1<<2&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m1>>6&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m1>>8&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
        c= *(long*)(mask+(m1>>16&0x3c)); bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        c= *(short*)(mask+(m1>>18&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE+6; pic+= BD_LINE;
	if((sc&(SC_ULINE|SC_REVERSE))==SC_ULINE) {
	  int cu= coluline^c1;
          c= *(long*)(mask+(m1>>26&0x3c)); bd-=4;*(long*)bd= c&cu^c1;
          c= *(short*)(mask+(m1>>12&0x30));bd-=2;*(short*)bd= (short)(c&cu^c1);
	} else {
	  if(sc&SC_ULINE) c1= coluline;
          c= *(long*)(mask+(m1>>26&0x3c)); bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          c= *(short*)(mask+(m1>>12&0x30));bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1);
        };
        bd-= BD_LINE*9-6; pic-= BD_LINE*9-8;
      };
    } else if(!(sc&SC_KANJI2)&&vsn>0) { //漢字
      WORD sc2= *vs++&0x7f;
      vsn--;
      BYTE const* mask;
      long c, c1;
      c1= colfore[sc>>9&0xf];
      mask= bitmask;
      if(sc&SC_REVERSE) mask= revmask;
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
        bd+= BD_LINE+12; pic+= BD_LINE;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
            m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd-= BD_LINE-12; pic-= BD_LINE;
            m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE*3+12; pic+= BD_LINE*3;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
	} else {
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1; bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	};
	if((sc&(SC_ULINE|SC_REVERSE))==SC_ULINE) {
	  int cu= coluline^c1;
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= c&cu^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= c&cu^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= c&cu^c1;
	} else {
          int cu= c1; if(sc&SC_ULINE) cu= coluline;
          m<<= 2; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^cu)&c^cu;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^cu)&c^cu;
          m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^cu)&c^cu;
	};
	bd-= BD_LINE-12; pic-= BD_LINE;
        m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+8)^c1)&c^c1;
        m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)(pic+4)^c1)&c^c1;
        m>>= 4; c= *(long*)(mask+(m&0x3c)); bd-=4;*(long*)bd= (*(long*)pic^c1)&c^c1;
        bd-= BD_LINE*8-12; pic-= BD_LINE*8-12;
      } else {
        bd+= BD_LINE+12; pic+= BD_LINE-2;
        m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	if(sc&SC_BOLD) {
  	  m|= m>>1&~(m<<1)&0x7fe7fe;
	  int y= 4; do {
            m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
            m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
            m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd-= BD_LINE-12; pic-= BD_LINE;
            m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
            m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
            m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
            m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE*3+12; pic+= BD_LINE*3;
            m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
  	    m|= m>>1&~(m<<1)&0x7fe7fe;
	  } while(--y);
        } else {
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd-= BD_LINE-12; pic-= BD_LINE;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1); bd+= BD_LINE*3+12; pic+= BD_LINE*3;
          m= (long)*cd++; m= m<<8|(long)*cd++&255; m= m<<8|(long)*cd++&255;
	};
	if((sc&(SC_ULINE|SC_REVERSE))==SC_ULINE) {
	  int cu= coluline^c1;
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)(c&cu^c1);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          c&cu^c1;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          c&cu^c1;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)(c&cu^c1);
	} else {
          int cu= c1; if(sc&SC_ULINE) cu= coluline;
          m<<= 4; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^cu)&c^cu);
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^cu)&c^cu;
          m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^cu)&c^cu;
          m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^cu)&c^cu);
	};
	bd-= BD_LINE-12; pic-= BD_LINE;
        m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+12)^c1)&c^c1);
        m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+8)^c1)&c^c1;
        m>>= 4; c= *(long*)(mask+(m&0x3c));  bd-=4;*(long*)bd=          (*(long*)(pic+4)^c1)&c^c1;
        m>>= 2; c= *(short*)(mask+(m&0x30)); bd-=2;*(short*)bd= (short)((*(short*)(pic+2)^c1)&c^c1);
        bd-= BD_LINE*8-12; pic-= BD_LINE*8-14;
      };
    } else { //漢字2バイト目
      bd+= 6; pic+= 6;
    };
  };
};
