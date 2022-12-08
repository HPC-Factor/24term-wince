// main.cpp : アプリケーション用のエントリ ポイントの定義
//
//////////////////////////////////////////////////////////////////////
// 24term Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
// See source.txt for detail

#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <commdlg.h>
#include "common.h"
#include "resource.h"
#include "main.h"
#include "draw.h"
#include "draw2.h"
#include "draw8.h"
#include "pic8.h"
#include "draw16.h"
#include "pic16.h"
#include "screen.h"

Screen* screen[8];
int screenmax; //numbers
int screencur; //current

int notifyscreen;
int notifyipc;

Screen* theScreen;
Draw* theDraw;
HINSTANCE theInst;

HWND wbase;
HWND wdisp;
HWND wtitle;
HWND wscroll;
HWND wmenu;
HWND wcomm;
HWND wdebug;
HWND wmodal; //wdispを隠さないwindow
HFONT titlefont;
long alttick;
int altflag;
long yscrollpos; //for speedup
int scrollon; //480時にスクロールあり
int selpoint;
int breakcapt;

//#define SNAPSHOT

#ifdef SNAPSHOT
LOGPALETTE* myPalette;  ////todo
void snapshot()
{  
  BITMAPINFOHEADER *bi;
  bi= (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)+1024);//bmiColorsは領域のみ必要
  HDC dc= GetDC(0);

  bi->biSize= sizeof(BITMAPINFOHEADER);
  bi->biWidth= 640;
  bi->biHeight= -1;
  bi->biPlanes= 1;
  bi->biBitCount= 8;
  bi->biCompression= BI_RGB;
  bi->biSizeImage= 0;
  bi->biXPelsPerMeter= 0;
  bi->biYPelsPerMeter= 0;
  bi->biClrUsed= 0;
  bi->biClrImportant= 0;
  UCHAR *bd;
  HBITMAP bm= CreateDIBSection(dc, (BITMAPINFO*)bi, DIB_PAL_COLORS, (void**)&bd, 0, 0);
  HDC bc= CreateCompatibleDC(dc);
  HBITMAP ob= (HBITMAP)SelectObject(bc, bm);

  HANDLE fh= CreateFile(L"\\snap.bmp", GENERIC_WRITE
  , 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  BITMAPFILEHEADER bh;
  bh.bfType= 0x4d42;
  bh.bfReserved1= 0;
  bh.bfReserved2= 0;
  bh.bfOffBits= sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+1024;
  bh.bfSize= bh.bfOffBits+640*240;
  DWORD dd;
  WriteFile(fh, &bh, sizeof(bh), &dd, 0);
  bi->biSize= sizeof(BITMAPINFOHEADER);
  bi->biWidth= 640;
  bi->biHeight= 240;
  bi->biPlanes= 1;
  bi->biBitCount= 8;
  bi->biCompression= BI_RGB;
  bi->biSizeImage= 0;
  bi->biXPelsPerMeter= 0;
  bi->biYPelsPerMeter= 0;
  bi->biClrUsed= 0;
  bi->biClrImportant= 0;
  int i;
  for(i= 0; i<256; i++) {
    long k= *(long*)&(((Draw8*)theDraw)->logpal->palPalEntry[i]);
    *(long*)((char*)bi+sizeof(BITMAPINFOHEADER)+i*4)
    = k>>16&0xff|k&0xff00|k<<16&0xff0000;
  };
  WriteFile(fh, bi, sizeof(BITMAPINFOHEADER)+1024, &dd, 0);
  for(i= 0; i<240; i++) {
    BitBlt(bc, 0, 0, 640, 1, dc, 0, 239-i, SRCCOPY);
    WriteFile(fh, bd, 640, &dd, 0);
  };
  CloseHandle(fh);
  SelectObject(bc, ob);
  DeleteDC(bc);
  DeleteObject(bm);
  free(bi);
  ReleaseDC(0, dc);
};
#endif

void
modalenter()
{
  if(wmodal==0) {
    EnableWindow(wdisp, 0);
    EnableWindow(wcomm, 0);
    EnableWindow(wmenu, 0);
  };
};

void
modalexit()
{
  if(wmodal==0) {
    EnableWindow(wdisp, 1);
    EnableWindow(wcomm, 1);
    EnableWindow(wmenu, 1);
    SetWindowPos(wdisp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
  };
};

void
breakdraw(HDC dc, int mode)
{
  HBRUSH br;
  RECT r;
  int a= 7, b= 24, c= 53, d= 70;
  if(mode==0) {
    br= GetSysColorBrush(COLOR_3DSHADOW);
    r.left= a; r.top= c; r.right= b-1; r.bottom= c+1; FillRect(dc, &r, br);
    r.left= a; r.top= c+1; r.right= a+1; r.bottom= d-2; FillRect(dc, &r, br);
    r.left= b-2; r.top= c+1; r.right= b-1; r.bottom= d-2; FillRect(dc, &r, br);
    r.left= a; r.top= d-2; r.right= b-1; r.bottom= d-1; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= a; r.top= d-1; r.right= b-1; r.bottom= d; FillRect(dc, &r, br);
    r.left= b-1; r.top= c; r.right= b; r.bottom= d; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= a+1; r.top= c+1; r.right= b-2; r.bottom= c+2; FillRect(dc, &r, br);
    r.left= a+1; r.top= c+2; r.right= a+2; r.bottom= d-2; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= a+2; r.top= c+2; r.right= b-2; r.bottom= d-2; FillRect(dc, &r, br);
  } else {
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= a; r.top= c; r.right= b; r.bottom= c+2; FillRect(dc, &r, br);
    r.left= a; r.top= c+2; r.right= a+2; r.bottom= d; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= b-1; r.top= c+1; r.right= b; r.bottom= d-1; FillRect(dc, &r, br);
    r.left= a+1; r.top= d-1; r.right= b; r.bottom= d; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= a+2; r.top= c+2; r.right= b-1; r.bottom= d-1; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_BTNTEXT);
    r.left= a+5; r.top= c+5; r.right= b-4; r.bottom= d-4; FillRect(dc, &r, br);
  };
//  int om= SetBkMode(dc, TRANSPARENT);
//  ExtTextOut(dc, a+7+mode, c+1+mode, 0, 0, L"Break", 5, 0);
//  SetBkMode(dc, om);
};

BOOL CALLBACK
commproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    SetWindowLong(w, GWL_EXSTYLE, GetWindowLong(w, GWL_EXSTYLE)|WS_EX_NOACTIVATE);
    breakcapt= 0;
    return(TRUE);
  case WM_QUERYNEWPALETTE:
    return disppalette();
    break;
  case WM_COMMAND:
    if(wp==MAKELONG(IDC_RTS, BN_CLICKED)||wp==MAKELONG(IDC_RTS, BN_DBLCLK)) {
      if(theScreen&&theScreen->socktype==4&&theScreen->sockmode==1) {
        if(theScreen->rts==1) {
	  EscapeCommFunction((HANDLE)theScreen->sock, CLRRTS);
          SendDlgItemMessage(wcomm, IDC_RTS, BM_SETCHECK, BST_UNCHECKED, 0);
	  theScreen->rts= 0;
	} else if(theScreen->rts==0) {
	  EscapeCommFunction((HANDLE)theScreen->sock, SETRTS);
          SendDlgItemMessage(wcomm, IDC_RTS, BM_SETCHECK, BST_CHECKED, 0);
	  theScreen->rts= 1;
	};
      };
    } else if(wp==MAKELONG(IDC_DTR, BN_CLICKED)||wp==MAKELONG(IDC_DTR, BN_DBLCLK)) {
      if(theScreen&&theScreen->socktype==4&&theScreen->sockmode==1) {
        if(theScreen->dtr==1) {
	  EscapeCommFunction((HANDLE)theScreen->sock, CLRDTR);
          SendDlgItemMessage(wcomm, IDC_DTR, BM_SETCHECK, BST_UNCHECKED, 0);
	  theScreen->dtr= 0;
	} else if(theScreen->dtr==0) {
	  EscapeCommFunction((HANDLE)theScreen->sock, SETDTR);
          SendDlgItemMessage(wcomm, IDC_DTR, BM_SETCHECK, BST_CHECKED, 0);
	  theScreen->dtr= 1;
	};
      };
    };
    break;
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc= BeginPaint(w, &ps);
      breakdraw(dc, 0);
      EndPaint(w, &ps);      
    };
    break;
  case WM_LBUTTONDOWN:
    {
      int x= LOWORD(lp);
      int y= HIWORD(lp);
      if(x>=7&&x<=56&&y>=53&&y<=70) {
        if(theScreen&&theScreen->socktype==4&&theScreen->sockmode==1) {
          SetCapture(w);
	  breakcapt= 1;
          HDC dc= GetDC(w);
          breakdraw(dc, 1);
          ReleaseDC(w, dc);
	  SetCommBreak((HANDLE)theScreen->sock);
	};
      };
    };
    break;
  case WM_LBUTTONUP:
    {
      if(breakcapt) {
        ReleaseCapture();
	ClearCommBreak((HANDLE)theScreen->sock);
        HDC dc= GetDC(w);
        breakdraw(dc, 0);
        ReleaseDC(w, dc);
	breakcapt= 0;
      };
    };
    break;
  };
  return(FALSE);
};

void commmodem()
{
  DWORD d;
  if(theScreen&&theScreen->socktype==4&&GetCommModemStatus((HANDLE)theScreen->sock, &d)) {
    SendDlgItemMessage(wcomm, IDC_CTS, BM_SETCHECK, (d&MS_CTS_ON)?BST_CHECKED:BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_DSR, BM_SETCHECK, (d&MS_DSR_ON)?BST_CHECKED:BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_CD, BM_SETCHECK, (d&MS_RLSD_ON)?BST_CHECKED:BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_RING, BM_SETCHECK, (d&MS_RING_ON)?BST_CHECKED:BST_UNCHECKED, 0);
  } else {
    SendDlgItemMessage(wcomm, IDC_CTS, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_DSR, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_CD, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(wcomm, IDC_RING, BM_SETCHECK, BST_UNCHECKED, 0);
  };
};

void commredisp()
{
  if(theScreen==0||theScreen->socktype!=4) {
    ShowWindow(wcomm, SW_HIDE);
  } else {
    int a= BST_INDETERMINATE;
    if(theScreen->sockmode!=1) a= BST_UNCHECKED;
    else if(theScreen->rts==1) a= BST_CHECKED;
    else if(theScreen->rts==0) a= BST_UNCHECKED;
    SendDlgItemMessage(wcomm, IDC_RTS, BM_SETCHECK, a, 0);
    a= BST_INDETERMINATE;
    if(theScreen->sockmode!=1) a= BST_UNCHECKED;
    else if(theScreen->dtr==1) a= BST_CHECKED;
    else if(theScreen->dtr==0) a= BST_UNCHECKED;
    SendDlgItemMessage(wcomm, IDC_DTR, BM_SETCHECK, a, 0);
    commmodem();
    SetWindowPos(wcomm, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
  };
};


BOOL CALLBACK
messageproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_INITDIALOG:
    {
      wmodal= w;
      WCHAR const*p= (WCHAR const*)lp;
      if(p[0]!='q') ShowWindow(GetDlgItem(w, IDCANCEL), SW_HIDE);
      p++;
      WCHAR buf[_MAX_PATH];
      WCHAR const*p1= wcschr(p, '\n');
      wcsncpy(buf, p, p1-p); buf[p1-p]= 0;
      SetWindowText(w, buf);
      SetDlgItemText(w, IDC_MESSAGE, p1+1);
    };
    return(TRUE);
  case WM_COMMAND:
    switch(wp) {
    case IDOK:
      {
        EndDialog(w, TRUE);
      };
      return(TRUE);
    case IDCANCEL:
      {
        EndDialog(w, FALSE);
      };
      return(TRUE);
    };
    break;
  case WM_QUERYNEWPALETTE:
    return disppalette();
  case WM_ACTIVATE:
    return dispactivate(wp);
  case WM_WINDOWPOSCHANGED:
    if(((WINDOWPOS*)lp)->flags&SWP_SHOWWINDOW) disppalette();
    return(FALSE); 
    break;
  };
  return(FALSE);
};

void
scrollset()
{
  yscrollpos= theScreen->yscroll;
  scrollon= 0;
  if(Draw::devwidth<640) {
    if(yscrollpos==0) {
      ShowWindow(wscroll, SW_HIDE);
    } else {
      scrollon= 1;
      ShowWindow(wscroll, SW_SHOWNA);
    };
  };
  SCROLLINFO sif;
  sif.cbSize= sizeof(SCROLLINFO);
  sif.fMask= SIF_ALL; //|SIF_DISABLENOSCROLL;
  sif.nMin= 0;
  sif.nMax= theScreen->config.ymax;
  sif.nPage= 25;
  sif.nPos= sif.nMax-24-yscrollpos;
  SetScrollInfo(wscroll, SB_CTL, &sif, 1);
};

void
scrolldo()
{
  EnterCriticalSection(&theScreen->lock);
  int d= yscrollpos-theScreen->yscroll;
  if(d) {
    theScreen->yscroll= yscrollpos;
    if(d>0) {
      int y;
      for(y= 0; y<24-d; y++) theScreen->copy[y]= theScreen->copy[y+d];
      for(; y<24; y++) {
	theScreen->copy[y]= -10000;
	int w= y+theScreen->yscroll-theScreen->ybase; if(w<0) w+= theScreen->ymax;
	theScreen->dirty[theScreen->iy[w]]= 0|80<<8;
      };
    } else {
      int y;
      for(y= 23; y>=-d; y--) theScreen->copy[y]= theScreen->copy[y+d];
      for(; y>=0; y--) {
	theScreen->copy[y]= -10000;
	int w= y+theScreen->yscroll-theScreen->ybase; if(w<0) w+= theScreen->ymax;
	theScreen->dirty[theScreen->iy[w]]= 0|80<<8;
      };
    };
  };
  LeaveCriticalSection(&theScreen->lock);
  theDraw->updatewin(theScreen, wdisp);
  SCROLLINFO sif;
  sif.cbSize= sizeof(SCROLLINFO);
  sif.fMask= SIF_POS;
  sif.nPos= theScreen->ymax-24-yscrollpos;
  SetScrollInfo(wscroll, SB_CTL, &sif, 1);
};

LPCWSTR
drawnew()
{
  if(Draw::devcolor==2) {
    theDraw= new Draw2;
  } else if(Draw::devcolor==8) {
    if(theScreen->config.checkpic()[0]==0) {
      theDraw= new Pic8;
    } else {
      theDraw= new Draw8;
    };
  } else if(Draw::devcolor>=15) {
    if(theScreen->config.checkpic()[0]==0) {
      theDraw= new Pic16;
    } else {
      theDraw= new Draw16;
    };
  };
  return theDraw->init(&theScreen->config, wdisp);
};


void
menudraw(HDC dc, int mode)
{
  HBRUSH br;
  RECT r;
  if(mode==0) {
    br= GetSysColorBrush(COLOR_3DSHADOW);
    r.left= 0; r.top= 0; r.right= 21; r.bottom= 1; FillRect(dc, &r, br);
    r.left= 0; r.top= 1; r.right= 1; r.bottom= 20; FillRect(dc, &r, br);
    r.left= 20; r.top= 1; r.right= 21; r.bottom= 20; FillRect(dc, &r, br);
    r.left= 0; r.top= 20; r.right= 21; r.bottom= 21; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= 0; r.top= 21; r.right= 21; r.bottom= 22; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= 1; r.top= 1; r.right= 20; r.bottom= 2; FillRect(dc, &r, br);
    r.left= 1; r.top= 2; r.right= 2; r.bottom= 20; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= 2; r.top= 2; r.right= 20; r.bottom= 20; FillRect(dc, &r, br);
  } else {
    br= GetSysColorBrush(COLOR_3DDKSHADOW);
    r.left= 0; r.top= 0; r.right= 21; r.bottom= 2; FillRect(dc, &r, br);
    r.left= 0; r.top= 2; r.right= 2; r.bottom= 22; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DLIGHT);
    r.left= 20; r.top= 1; r.right= 21; r.bottom= 21; FillRect(dc, &r, br);
    r.left= 1; r.top= 21; r.right= 21; r.bottom= 22; FillRect(dc, &r, br);
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= 2; r.top= 2; r.right= 20; r.bottom= 21; FillRect(dc, &r, br);
  };
  HICON mi= (HICON)LoadImage(theInst, MAKEINTRESOURCE(IDI_MENU), IMAGE_ICON, 16, 16, 0);
  DrawIcon(dc, 6+mode, 5+mode, mi);
  DestroyIcon(mi);
};

void
menudrop()
{
  if(wmodal!=0) return;
  if(Draw::devwidth<640) {
    SetWindowPos(wmenu, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    ValidateRect(wmenu, 0);
  };
  HDC dc= GetDC(wmenu);
  menudraw(dc, 1);
  ReleaseDC(wmenu, dc);
  HMENU mm= LoadMenu(theInst, MAKEINTRESOURCE(IDR_MENU));
  HMENU md= GetSubMenu(mm, 0);
  HMENU mp= CreatePopupMenu();
  for(int ix= 0; ix<screenmax; ix++) {
    WCHAR buf[_MAX_PATH];
    wsprintf(buf, L"&%d %s", ix+1, screen[ix]->config.title);
    int a= screen[ix]->sockmode;
    if(a==0) wcscat(buf, L" (connecting)");
    else if(a<0) wcscat(buf, L" (closed)");
    AppendMenu(mp, MF_STRING|(ix==screencur?MF_CHECKED:0), ix+41000, buf);
  };
  InsertMenu(md, IDM_NEW, MF_STRING|MF_POPUP, (long)mp, L"&Windows");
  if(Draw::devwidth<640) {
    CheckMenuItem(md, IDM_SCROLLBAR, scrollon?MF_CHECKED:0);
  } else {
    DeleteMenu(md, IDM_SCROLLBAR, 0);
  };
  if(theScreen->selstart) {
    MENUITEMINFO mi;
    mi.cbSize= sizeof(mi); mi.fMask= MIIM_TYPE;
    mi.fType= MFT_STRING; mi.dwTypeData= L"Co&py";
    SetMenuItemInfo(md, IDM_COPYPASTE, 0, &mi);
  } else {
    OpenClipboard(wbase);
    EnableMenuItem(md, IDM_COPYPASTE, IsClipboardFormatAvailable(CF_UNICODETEXT)?0:MF_GRAYED);
    CloseClipboard();
  };
  long act= TrackPopupMenu(md, 0, 21, 0, 0, wbase, 0);
  DestroyMenu(mp);
  DestroyMenu(mm);
  dc= GetDC(wmenu);
  menudraw(dc, 0);
  ReleaseDC(wmenu, dc);
  MSG umsg;
  while(PeekMessage(&umsg, wmenu, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE)) ;
  if(IsWindowVisible(wdisp)) { //480の場合のメニューボタン隠しを兼ねる
    SetWindowPos(wdisp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
  };
};

LRESULT CALLBACK
menuproc(
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
      menudraw(dc, 0);
      EndPaint(w, &ps);      
    };
    return(0);
  case WM_LBUTTONDOWN:
    menudrop();
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

void
titledraw(HDC dc)
{
  int i;
  HFONT of= (HFONT)SelectObject(dc, titlefont);
  int om= SetBkMode(dc, TRANSPARENT);
  HBRUSH br;
  RECT r;
  if(screenmax>0) {
    br= GetSysColorBrush(COLOR_3DFACE);
    r.left= 0; r.right= 21;
    r.bottom= screen[0]->config.titletop;
    if(screen[0]==theScreen) r.bottom-= 2;
    r.top= 0; FillRect(dc, &r, br);
    r.top= screen[screenmax-1]->config.titletop+screen[screenmax-1]->config.titleh;
    if(screen[screenmax-1]==theScreen) r.top+= 2;
    r.bottom= 240; FillRect(dc, &r, br);
    for(i= 0; i<screenmax; i++) {
      const Config* config= &screen[i]->config;
      int e= 0; if(screen[i]==theScreen) e= 2;
      int ep= 0; if(i>0&&screen[i-1]==theScreen) ep= 2;
      int en= 0; if(i<screenmax-1&&screen[i+1]==theScreen) en= 2;
      br= GetSysColorBrush(COLOR_3DFACE);
      if(e==0) {
        r.top= config->titletop+ep;
	r.bottom= config->titletop+config->titleh-en;
	r.left= 0; r.right= 2; FillRect(dc, &r, br);
      };
      if(ep==0) {
        r.top= config->titletop-e; r.bottom= r.top+2;
	r.left= 2-e; r.right= r.left+2; FillRect(dc, &r, br);
      };
      if(en==0) {
        r.top= config->titletop+config->titleh-2+e; r.bottom= r.top+2;
	r.left= 2-e; r.right= r.left+2; FillRect(dc, &r, br);
      };
      br= GetSysColorBrush(COLOR_3DSHADOW);
      if(ep==0) {
        r.top= config->titletop-e; r.bottom= r.top+1; r.left= 4-e; r.right= 21; FillRect(dc, &r, br);
        r.top++; r.bottom= r.top+1; r.left--; r.right= r.left+1; FillRect(dc, &r, br);
      };
      r.top= config->titletop+2-e; r.bottom= config->titletop+config->titleh-2+e;
      r.left= 2-e; r.right= r.left+1; FillRect(dc, &r, br);
      if(en==0) {
        r.top= config->titletop+config->titleh-2+e; r.bottom= r.top+1;
	r.left= 4-e; r.right= 21; FillRect(dc, &r, br);
      };
      br= GetSysColorBrush(config->black?COLOR_3DDKSHADOW:COLOR_3DLIGHT);
      if(ep==0) {
        r.top= config->titletop+1-e; r.bottom= r.top+1; r.left= 4-e; r.right= 21; FillRect(dc, &r, br);
      };
      r.top= config->titletop+2-e; r.bottom= config->titletop+config->titleh-2+e;
      r.left= 3-e; r.right= r.left+1; FillRect(dc, &r, br);
      if(en==0) {
        br= GetSysColorBrush(config->black?COLOR_3DLIGHT:COLOR_3DDKSHADOW);
        r.top= config->titletop+config->titleh-2+e; r.bottom= r.top+1;
	r.left= 3-e; r.right= r.left+1; FillRect(dc, &r, br);
        r.top++; r.bottom= r.top+1; r.left++; r.right= 21; FillRect(dc, &r, br);
      };
      COLORREF oc;
      if(Draw::devcolor==8) { //palette mode
        br= CreateSolidBrush(0x01000000|i*2+0x1c);
        oc= SetTextColor(dc, 0x01000000|i*2+0x1d);
      } else if(Draw::devcolor==2) {
        int colb= (config->colback>>16&255)*30+(config->colback>>8&255)*59+(config->colback&255)*11+50;
	int d= colb/6400;
	if(config->color[0]!=config->colback) {
	  int col= (config->color[0]>>16&255)*30+(config->color[0]>>8&255)*59+(config->color[0]&255)*11+50;
	  if(col/6400!=d) d= col/6400;
	  else if(col>colb) d++; else d--;
	};
	if(d<0) d= 0; if(d>3) d= 3;
	colb/= 6400;
        br= CreateSolidBrush(0x01000000|colb);
        oc= SetTextColor(dc, 0x01000000|d);
      } else {
        br= CreateSolidBrush(RGB(config->colback>>16&255, config->colback>>8&255, config->colback&255));
        oc= SetTextColor(dc, RGB(config->color[0]>>16&255, config->color[0]>>8&255, config->color[0]&255));
      };
      r.top= config->titletop+2-e; r.bottom= config->titletop+config->titleh-2+e;
      r.left= 4-e; r.right= 21; FillRect(dc, &r, br);
      DeleteObject(br);
      ExtTextOut(dc, 4, config->titletop+config->titleh-3, 0, 0, config->title, wcslen(config->title), 0);
      SetTextColor(dc, oc);
    };
    SetBkMode(dc, om);
    SelectObject(dc, of);
  };
};

//すべてのタイトル、位置再計算、再表示
void
titlesetup()
{
  int top= 4;
  int i;
  HDC dc= GetDC(wtitle);
  HFONT of= (HFONT)SelectObject(dc, titlefont);
  SIZE sz;
  for(i= 0; i<screenmax; i++) {
    //タイトル名作成
    wcscpy(screen[i]->config.title, screen[i]->config.name);
    if(screen[i]->config.title[0]==0&&screen[i]->config.file[0]=='\\') {
      wcscpy(screen[i]->config.title, wcsrchr(screen[i]->config.file, L'\\')+1);
      WCHAR *p= wcschr(screen[i]->config.title, L'.');
      if(p&&p!=screen[i]->config.title) *p= 0;
    };
    if(screen[i]->config.title[0]==0&&screen[i]->config.host[0]) {
      wcscpy(screen[i]->config.title, screen[i]->config.host);
      WCHAR *p= screen[i]->config.title;
      while(*p>='0'&&*p<='9'||*p=='.') p++;
      if(*p!=0&&*p!=':') p= wcspbrk(screen[i]->config.title, L":.");
      if(p&&p!=screen[i]->config.title) *p= 0;
    };
    if(screen[i]->config.title[0]==0) {
      wcscpy(screen[i]->config.title, L"24term");
    };

    screen[i]->config.titletop= top;
    GetTextExtentPoint32(dc, screen[i]->config.title
    , wcslen(screen[i]->config.title), &sz);
    screen[i]->config.titleh= sz.cx+6;
    top+= sz.cx+6;
  };
  SelectObject(dc, of);
  if(Draw::devcolor==8) {
    for(i= 0; i<screenmax; i++) {
      ((Draw8*)theDraw)->palset(0x1c+i*2, screen[i]->config.colback);
      ((Draw8*)theDraw)->palset(0x1d+i*2, screen[i]->config.color[0]);
    };
    SetPaletteEntries(((Draw8*)theDraw)->palette, 0x1c, screenmax*2
    , &((Draw8*)theDraw)->logpal->palPalEntry[0x1c]);
    disppalette(); //realize palette
  };
  titledraw(dc);
  ReleaseDC(wtitle, dc);
  SetWindowText(wbase, theScreen->config.title);
};


LRESULT CALLBACK
titleproc(
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
      titledraw(dc);
      EndPaint(w, &ps);      
    };
    return(0);
  case WM_LBUTTONDOWN:
    {
      int y= HIWORD(lp);
      int i;
      for(i= 0; i<screenmax; i++) {
        if(screen[i]->config.titletop<=y
	&&y<screen[i]->config.titletop+screen[i]->config.titleh) break;
      };
      if(i<screenmax&&theScreen!=screen[i]) {
        screencur= i;
        theScreen= screen[screencur];
        titlesetup();
	commredisp();
        scrollset();
	delete theDraw;
	drawnew();
        disppalette();
        theDraw->redrawwin(theScreen, wdisp);
      };
    };
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

long dispresetpalette()
{
  if(Draw::devcolor==8&&theDraw&&((Draw8*)theDraw)->palette) {
    HDC dc= GetDC(0);
    GetSystemPaletteEntries(dc, 0, 256, &((Draw8*)theDraw)->logpal->palPalEntry[0]);
    ReleaseDC(0, dc);
    SetPaletteEntries(((Draw8*)theDraw)->palette, 0, 256, &((Draw8*)theDraw)->logpal->palPalEntry[0]);
    dc= GetDC(wdisp);
    HPALETTE opal= SelectPalette(dc, ((Draw8*)theDraw)->palette, FALSE);
    RealizePalette(dc);
    SelectPalette(dc, opal, FALSE);
    ReleaseDC(wdisp, dc);
    return(TRUE);
  };
  return(0);
};

long disppalette()
{
  if(Draw::devcolor==8&&theDraw&&((Draw8*)theDraw)->palette) {
    HDC dc= GetDC(wdisp);
    HPALETTE opal= SelectPalette(dc, ((Draw8*)theDraw)->palette, FALSE);
    RealizePalette(dc);
    SelectPalette(dc, opal, FALSE);
    ReleaseDC(wdisp, dc);
    return(TRUE);
  };
  return(0);
};

//上に乗るダイアログから呼ばれる
long dispactivate(WPARAM wp)
{
  if(LOWORD(wp)==WA_INACTIVE) {
    ShowWindow(wdisp, SW_HIDE);
    if(Draw::devwidth>=640) {
      ShowWindow(wtitle, SW_HIDE);
      ShowWindow(wscroll, SW_HIDE);
    };
  } else {
    DEBUGMSG(1,(L"dispactivate active\n"));
    SetWindowPos(wdisp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOOWNERZORDER);
    if(Draw::devwidth>=640) {
      ShowWindow(wtitle, SW_SHOWNA);
      ShowWindow(wscroll, SW_SHOWNA);
    };
  };
  return(0);
};

void deletescreen(int ix)
{
  if(screen[ix]->config.cert) free(screen[ix]->config.cert);
  screen[ix]->release();
  delete screen[ix];
  screenmax--;
  for(int i= ix; i<screenmax; i++) screen[i]= screen[i+1];
  if(ix<=screencur&&screencur>0) screencur--;
  theScreen= 0;
  if(screenmax>0) {
    theScreen= screen[screencur];
    titlesetup();
    commredisp();
    scrollset();
    delete theDraw;
    drawnew();
    disppalette();
    theDraw->redrawwin(theScreen, wdisp);
  };
};

LPCWSTR createscreen(const Config &newconfig)
{
  screencur= screenmax;
  screenmax++;
  screen[screencur]= new Screen;
  screen[screencur]->init();
  screen[screencur]->config.init();
  screen[screencur]->config= newconfig;
  theScreen= screen[screencur];

  if(theDraw) delete theDraw;
  LPCWSTR es= drawnew();
  if(es[0]) return es;
  disppalette();
  titlesetup();
  commredisp();
  theDraw->redrawwin(theScreen, wdisp);
  int rt= TRUE;
  if(theScreen->config.host[0]==0) { //ホスト名なし…ダイアログ表示
    scrollset();
    HWND wback= wmodal;
    modalenter();
    rt= DialogBox(theInst, (LPCTSTR)IDD_CONFIG, wbase, Config::configwrap);
    wmodal= wback;
    modalexit();
  };
  if(rt==TRUE&&theScreen->config.host[0]&&theScreen->config.host[0]!='-') {
    scrollset();
    theScreen->allocate();
    theScreen->thread= CreateThread(0, 0, Screen::screenwrap, (void*)theScreen, 0, 0);
    return(L"");
  } else {
    if(theScreen->config.host[0]=='-') {
      scrollset();
      MessageBeep(MB_ICONEXCLAMATION);
      HWND wback= wmodal;
      modalenter();
      DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wbase, messageproc, (long)L"m24term\nNo host name specified.");
      wmodal= wback;
      modalexit();
    };
    deletescreen(screencur);
    if(screenmax==0) PostQuitMessage(0);
    return(L"0No Creation");
  };
};

//screenを見て廻って必要に応じダイアログ表示、閉じる
int checkscreen()
{
  //if(wmodal!=0) return; //sentinel
  int ix= 0;
  for(ix= 0; ix<screenmax; ix++) {
    if(screen[ix]->sockmode==-2) {
      screen[ix]->sockmode= -1; //表示おわったぞ
      MessageBeep(MB_OK);
      WCHAR buf[_MAX_PATH];
      wsprintf(buf, L"m%s\n%s", screen[ix]->config.title, screen[ix]->error+1);
      HWND wback= wmodal;
      modalenter();
      DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wbase, messageproc, (long)buf);
      wmodal= wback;
      modalexit();
      return(1);
    };
    if(screen[ix]->sockmode<0&&screen[ix]->config.stay==0) { //削除
      deletescreen(ix);
      if(screenmax==0) PostQuitMessage(0); //DestroyWindow(wbase);
      return(1);
    };
  };
  return(0);
};

//キー出力
void writekey(const char*p, int n)
{
  if(theScreen->sock==INVALID_SOCKET) return;
  if(theScreen->sockmode<=0) return;
  if(theScreen->selstart) { //selectionあったら解除
    theScreen->selstart= 0; theScreen->selend= 0;
    theDraw->redrawwin(theScreen, wdisp);
  };
  if(theScreen->config.echo==1) {
    theScreen->printk(p, n);
    PostMessage(wdisp, WM_USER, 0, (long)theScreen);
  };
  theScreen->writek(p, n);
};

LPCWSTR savelog(WCHAR const*fname)
{
  HANDLE fd= CreateFile(fname, GENERIC_WRITE, 0, 0
  , CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if(fd==INVALID_HANDLE_VALUE) return L"cFailed to create file.";
  if(theScreen->iy) {
    //lockかけると受信損失する可能性大なのでしない。
    int ybase= theScreen->ybase;
    int flag= 0;
    for(int y= theScreen->ymax-1; y>=0; y--) {
      int w= y-ybase; if(w<0) w+= theScreen->ymax;
      WORD* textp= theScreen->text[theScreen->iy[w]];
      int m;
      for(m= 80; m>0; m--) if((textp[m-1]&0x1ff)>=0x20) break;
      if(m||flag) {
        flag= 1;
        char buf[100], *p;
	p= buf;
        for(int i= 0; i<m; i++) {
	  WORD c= (textp[i]&0x1ff);
	  if(c<0x100) {
	    if(c<0x20) c= 0x20;
	    if(c==0x7f) c= 0x5c;
	    *p++= (char)c;
	  } else if(c<0x180&&(textp[i+1]&0x180)==0x180) {
	    *p++= (c/2+0x21)^0x20;
	    WORD c1= textp[++i]&0x7f;
            if(c&1) {
	      *p++= (char)(c1+0x9f);
	    } else {
	      *p++= (char)(c1+0x40+(c1>=0x3f));
	    };
	  };
	};
	if((textp[80]&0x1ff)!=0x20) *p++= 0x0d, *p++= 0x0a;
	DWORD d;
	WriteFile(fd, buf, p-buf, &d, 0);
      };
    };
  };
  CloseHandle(fd);
  return L"";
};

void selcopy()
{
  if(theScreen->iy==0) return;
  int ybase= theScreen->ybase;
  int cur= theScreen->selstart;
  int end= theScreen->selend;
  int csize= 100;
  WCHAR* clip= (WCHAR*)LocalAlloc(0, csize*2);
  int cpos= 0;
  while(cur>end) {
    int y1= (cur+79)/80;
    int w= y1-1-ybase; if(w<0) w+= theScreen->ymax;
    WORD* textp= theScreen->text[theScreen->iy[w]];
    int m= y1*80-end;
    if(m>80) m= 80;
    int i= y1*80-cur;
    for(; m>i; m--) if((textp[m-1]&0x1ff)>=0x20) break; //最後の空白カット
    char buf[82], *p;
    p= buf;
    for(; i<m; i++) {
      WORD c= (textp[i]&0x1ff);
      if(c<0x100) {
        if(c<0x20) c= 0x20;
        if(c==0x7f) c= 0x5c;
        *p++= (char)c;
      } else if(c<0x180&&(textp[i+1]&0x180)==0x180) { //漢字2バイト目は範囲外でもコピー対象
        *p++= (c/2+0x21)^0x20;
        WORD c1= textp[++i]&0x7f;
        if(c&1) {
          *p++= (char)(c1+0x9f);
        } else {
          *p++= (char)(c1+0x40+(c1>=0x3f));
        };
      };
    };
    if(cpos+100>csize) {
      csize= (csize*5/4)+100;
      clip= (WCHAR*)LocalReAlloc(clip, csize*2, LMEM_MOVEABLE);
    };
    cpos+= MultiByteToWideChar(0, 0, buf, p-buf, clip+cpos, 98);
    cur= y1*80-80;
    if(cur>end&&(textp[80]&0x1ff)!=0x20) {
      clip[cpos++]= 0x0d; clip[cpos++]= 0x0a;
    };
  };
  clip[cpos++]= 0;
  LocalReAlloc(clip, cpos*2, LMEM_MOVEABLE);
  OpenClipboard(wbase);
  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, clip);
  CloseClipboard();
};

LRESULT CALLBACK
baseproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  int rt;
  switch(msg) {
#ifdef SNAPSHOT
  case WM_LBUTTONDOWN:
    if(GetKeyState(VK_CONTROL)&0x8000) {
      if(Draw::devcolor==8) snapshot();
      return(0);
    };
#endif
    break;
  case WM_CTLCOLORSTATIC:
    {
      SetTextColor((HDC)wp, GetSysColor(COLOR_STATICTEXT));
      SetBkMode((HDC)wp, TRANSPARENT);
      return (long)GetSysColorBrush(COLOR_BTNFACE);
    };
  case WM_QUERYNEWPALETTE:
    return disppalette();
  case WM_ACTIVATE:
    if(LOWORD(wp)==WA_INACTIVE) {
      if(wmodal&&(HWND)lp==wmodal) { //消さない配慮…なくてもdispactivateで表示されるけど
        SetWindowPos(wdisp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
      } else {
        ShowWindow(wdisp, SW_HIDE);
        if(Draw::devwidth>=640) {
          ShowWindow(wtitle, SW_HIDE);
	  ShowWindow(wscroll, SW_HIDE);
	};
      };
    } else {
      MSG umsg;
      while(PeekMessage(&umsg, w, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE)) ; //左クリック削除
      if(wmodal==0) {
        SetWindowPos(wdisp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
      } else {
        SetWindowPos(wdisp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOOWNERZORDER);
      };
      if(Draw::devwidth>=640) {
	ShowWindow(wtitle, SW_SHOWNA);
        ShowWindow(wscroll, SW_SHOWNA);
      };
    };
    return(0);
  case WM_VSCROLL:
    {
      if(theScreen->iy==0) return(0);
      SCROLLINFO sif;
      sif.cbSize= sizeof(SCROLLINFO);
      sif.fMask= SIF_ALL;
      GetScrollInfo(wscroll, SB_CTL, &sif);
      yscrollpos= theScreen->ymax-24-sif.nPos;
      switch(LOWORD(wp)) {
      case SB_LINEUP: yscrollpos++; break;
      case SB_LINEDOWN: yscrollpos--; break;
      case SB_PAGEUP: yscrollpos+=23; break;
      case SB_PAGEDOWN: yscrollpos-=23; break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK: yscrollpos= theScreen->ymax-24-sif.nTrackPos; break;
      };
      if(yscrollpos<0) yscrollpos= 0;
      if(yscrollpos>theScreen->ymax-24) yscrollpos= theScreen->ymax-24;
      if(yscrollpos!=theScreen->ymax-24-sif.nPos) {
        scrolldo();
      };
      return(0);
    };
  case WM_SIZE:
    {
      Draw::baseheight= HIWORD(lp); if(Draw::baseheight>240) Draw::baseheight= 240;
      if(Draw::devwidth>=640) MoveWindow(wscroll, 506, 0, 19, Draw::baseheight, 1);
      MoveWindow(wtitle, 0, 24, 21, Draw::baseheight-24, 1);
      return(0);
    };
  case WM_COMMAND:
    if(wp>=41000&&wp<41099) { //screen
      int i= wp-41000;
      if(i<screenmax&&theScreen!=screen[i]) {
        screencur= i;
        theScreen= screen[screencur];
        titlesetup();
	commredisp();
	scrollset();
	delete theDraw;
	drawnew();
        disppalette();
        theDraw->redrawwin(theScreen, wdisp);
      };
    } else switch(wp) {
    case IDM_SCROLLBAR:
      {
        if(scrollon) {
	  scrollon= 0;
	  ShowWindow(wscroll, SW_HIDE);
	  yscrollpos= 0;
	  scrolldo();
	} else {
	  scrollon= 1;
	  ShowWindow(wscroll, SW_SHOWNA);
	};
      };
      break;
    case IDM_COPYPASTE:
      {
        if(theScreen->selstart) { //copy
	  selcopy();
	} else { //paste
	  char* buf;
	  OpenClipboard(wbase);
	  WCHAR* data= (WCHAR*)GetClipboardData(CF_UNICODETEXT);
	  if(data) {
            int sz= WideCharToMultiByte(0, 0, data, -1, 0, 0, 0, 0);
	    buf= (char*)malloc(sz);
	    if(buf) WideCharToMultiByte(0, 0, data, -1, buf, sz, 0, 0);
	  };
	  CloseClipboard();
	  if(buf) {
	    char* p= buf;
            for(;;) {
	      char* q= p;
	      while(*p&&*p!='\r'&&*p!='\n') p++;
	      if(p!=q) writekey(q, p-q);
              if(*p==0) break;
	      //cr or lf
	      if(*p=='\r'&&p[1]=='\n') p++; //cr+lf
	      p++;
	      if(theScreen->config.enter==0) {
	        writekey("\r\n", 2);
	      } else if(theScreen->config.enter==2) {
	        writekey("\n", 1);
	      } else {
	        writekey("\r", 1);
	      };
	    };
	    free(buf);
	  };
	};
      };
      break;
    case IDM_LOG:
      {
        WCHAR fname[_MAX_PATH];
	fname[0]= 0;
        OPENFILENAME fns;
        memset((void*)&fns, 0, sizeof(fns));
        fns.lStructSize= sizeof(fns);
        fns.hwndOwner= w;
        fns.lpstrFilter= L"All Files (*.*)\0*.*\0";
        fns.nFilterIndex= 1;
        fns.lpstrFile= fname;
        fns.nMaxFile= numof(fname);
	fns.lpstrInitialDir= 0;
        fns.lpstrTitle= L"Save Buffer Log";
        fns.Flags= OFN_OVERWRITEPROMPT;
	HWND wback= wmodal;
	modalenter();
	wmodal= (HWND)1;
        int rt= GetSaveFileName(&fns);
	wmodal= wback;
	if(rt) {
	  PCWSTR es= savelog(fname);
	  if(es[0]) {
            MessageBeep(MB_ICONEXCLAMATION);
	    WCHAR buf[_MAX_PATH];
	    wsprintf(buf, L"mSave Error\n%s", es+1);
	    HWND wback= wmodal;
            DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, w, messageproc, (long)buf);
            wmodal= wback;
	  };
	};
	modalexit();
      };
      break;
    case IDM_RESET:
      {
        theScreen->state= Screen::STbase;
        theScreen->attrfg= 0;
	theScreen->attr= theScreen->attrfg; theScreen->attrbg= 0;
	theScreen->rolltop= 23; theScreen->rolltail= 0;
	theScreen->recvset= 0; theScreen->sendset= 0;
      };
      break;
    case IDM_NEW:
      {
        if(screenmax>=8) break;
	Config newconfig;
	newconfig.init();
	newconfig.load();
	createscreen(newconfig);
      };
      break;
    case IDM_CONFIG:
      {
        HWND wback= wmodal;
	modalenter();
        rt= DialogBox(theInst, (LPCTSTR)IDD_CONFIG, wbase, Config::configwrap);
        wmodal= wback;
	modalexit();
        //シリアルセット
	if(rt==TRUE&&wcscmp(theScreen->configback.host, theScreen->config.host)
	&&theScreen->socktype==4&&(HANDLE)theScreen->sock!=INVALID_HANDLE_VALUE
	&&theScreen->config.host[0]=='#'&&theScreen->config.host[1]!='#') {
	  WCHAR *p= theScreen->config.host;
	  p++;
          for(;*p&&*p!=':';p++) ; //ポート名スキップ
	  COMMPROP cp;
	  int rsize= 0;
	  if(GetCommProperties((HANDLE)theScreen->sock, &cp)) rsize= cp.dwCurrentRxQueue;
	  if(rsize==0) rsize= 256;
	  int irmode= 0;
	  DCB cd;
	  GetCommState((HANDLE)theScreen->sock, &cd);
	  cd.BaudRate= 9600; cd.ByteSize= 8;
	  cd.Parity= NOPARITY; cd.StopBits= ONESTOPBIT;
          cd.fOutxCtsFlow= FALSE; cd.fRtsControl= RTS_CONTROL_ENABLE;

	  cd.fOutX= FALSE; cd.fInX= FALSE; //no X_ON_X_OFF

	  cd.fNull= FALSE; cd.fTXContinueOnXoff= TRUE;
	  cd.fOutxDsrFlow= FALSE; cd.fDsrSensitivity= FALSE; //ignore DSR
	  if(theScreen->dtr==0) {
	    cd.fDtrControl= DTR_CONTROL_DISABLE;
	  } else if(theScreen->dtr==1) {
	    cd.fDtrControl= DTR_CONTROL_ENABLE;
	  };
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
	  if(cd.fRtsControl==RTS_CONTROL_HANDSHAKE) {
	    theScreen->rts= -1;
	  } else {
	    if(theScreen->rts==0) {
	      cd.fRtsControl= RTS_CONTROL_DISABLE;
	    } else {
	      theScreen->rts= 1;
	    };
          };
	  if(SetCommState((HANDLE)theScreen->sock, &cd)==0) {
	    MessageBeep(MB_OK);
	    WCHAR buf[_MAX_PATH];
	    wsprintf(buf, L"m%s\nFailed to set com state.", theScreen->config.title);
            HWND wback= wmodal;
	    modalenter();
            rt= DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wbase, messageproc, (long)buf);
  	    wmodal= wback;
	    modalexit();
	  };
	  if(irmode==1) {
	    EscapeCommFunction((HANDLE)theScreen->sock, SETIR);
	  } else {
	    EscapeCommFunction((HANDLE)theScreen->sock, CLRIR);
	  };
          commredisp();
	};
      };
      break;
    case IDM_CLOSE:
      {
        rt= IDOK;
        if(theScreen->sockmode>=0&&theScreen->config.stay!=0) { //閉じていいか？
	  MessageBeep(MB_ICONQUESTION);
	  WCHAR buf[_MAX_PATH];
	  wsprintf(buf, L"q%s\nClose?", theScreen->config.title);
          HWND wback= wmodal;
	  modalenter();
          rt= DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wbase, messageproc, (long)buf);
	  wmodal= wback;
	  modalexit();
	};
	if(rt==IDOK) {
          theScreen->close(); //この間走り回るけど、気にしない。
	  TerminateThread(theScreen->thread, 0); //maybe error when already closed
	  theScreen->sockmode= -1;
          deletescreen(screencur);
          if(screenmax==0) PostQuitMessage(0); //DestroyWindow(wbase);
	};
      };
      break;
    case IDM_QUIT:
      {
        DEBUGMSG(1, (L"*******************************************************\n"));
        MessageBeep(MB_ICONQUESTION);
        HWND wback= wmodal;
	modalenter();
        rt= DialogBoxParam(theInst, (LPCTSTR)IDD_MESSAGE, wbase, messageproc, (long)L"q24term\nQuit?");
	wmodal= wback;
	modalexit();
        if(rt==IDOK) {
	  PostQuitMessage(0);
	  //DestroyWindow(wbase);
	};
      };
      break;
    };
    return(0);
  case WM_DESTROY:
    PostQuitMessage(0);
    return(0);
  case WM_CHAR:
  case WM_SYSCHAR:
    {
      if(wp<128) {
        if(lp&0x20000000) { //ALT
          char c[2];
          c[0]= 27;
          c[1]= (char)wp;
          writekey(c, 2);
	} else {
          char c;
	  c= (char)wp;
          writekey(&c, 1);
	};
      } else {
        WCHAR wc;
        char c[4];
	wc= wp;
	int n= WideCharToMultiByte(0, 0, &wc, 1, c, 4, 0, 0);
	writekey(c, n);
      };
    };
    return(0);
  case WM_KEYDOWN:
    {
      altflag= 0;
      if(wp==VK_UP) {
        writekey("\33OA", 3);
      } else if(wp==VK_DOWN) {
        writekey("\33OB", 3);
      } else if(wp==VK_RIGHT) {
        writekey("\33OC", 3);
      } else if(wp==VK_LEFT) {
        writekey("\33OD", 3);
      } else break; //continue to defproc
    };
    return(0);
  case WM_KEYUP:
    altflag= 0;
    return(0);
  case WM_SYSKEYDOWN:
    {
      altflag= 0;
      if(wp==VK_MENU) {
        if(!(lp&0x40000000)) altflag= 1, alttick= GetTickCount();
      };
    };
    return(0);
  case WM_SYSKEYUP:
    if(altflag&&wp==VK_MENU) {
      DWORD dt= GetTickCount()-alttick;
      if(dt<=167) { //short alt click
        menudrop();
      };
    };
    altflag= 0;
    return(0);
  case WM_USER+2:
    notifyipc= 1;
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};


LRESULT CALLBACK
dispproc(
  HWND w,
  UINT msg,
  WPARAM wp,
  LPARAM lp
) {
  switch(msg) {
  case WM_HSCROLL:
    return SendMessage(wbase, WM_VSCROLL, wp, lp);
  case WM_QUERYNEWPALETTE:
    return TRUE; //guardian
  case WM_LBUTTONDOWN:
    if(wmodal==0) { //サブウィンドウにフォーカス取られた時用
      SetWindowPos(wdisp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
      SetFocus(wbase);
    };
    //continues
  case WM_MOUSEMOVE:
  case WM_LBUTTONUP:
    if(theScreen==0||theScreen->iy==0) return(0);
    {
      int x= (LOWORD(lp)-Draw::winoffx+3)/6;
      if(x<0) x= 0;
      if(x>80) x= 80;
      int y= 23-HIWORD(lp)/10;
      if(y<0) y= 0;
      if(y>23) y= 23;
      int z= y*80+80-x+theScreen->yscroll*80;
      if(msg==WM_LBUTTONDOWN) {
        if(wp&MK_SHIFT&&theScreen->selstart) {
	  if(z*2>theScreen->selstart+theScreen->selend) {
	    selpoint= theScreen->selend;
	  } else {
	    selpoint= theScreen->selstart;
	  };
	} else {
          selpoint= z;
	};
      } else if(selpoint<0) return(0);
      //selstart,selendのセット
      int ss= 0, se= 0;
      if(selpoint<z) {
        ss= z;
	se= selpoint;
      } else if(selpoint>z) {
        ss= selpoint;
	se= z;
      };
      if(msg==WM_LBUTTONUP) selpoint= -1;
      if(theScreen->selstart!=ss||theScreen->selend!=se) {
        DEBUGMSG(1,(L"%d-%d\n", theScreen->selstart, theScreen->selend));
        theScreen->selstart= ss;
	theScreen->selend= se;
        theDraw->redrawwin(theScreen, w);
      };
    };
    return(0);
  case WM_PAINT:
    {
      ValidateRect(w, 0);
      theDraw->redrawwin(theScreen, w);
    };
    return(0);
  case WM_USER:
    {
      MSG umsg;
      while(PeekMessage(&umsg, w, WM_USER, WM_USER, PM_REMOVE)) ;
      if(IsWindowVisible(w)&&theScreen) {
        theDraw->updatewin(theScreen, w);
	int a= theScreen->yscroll;
	if(yscrollpos!=a) {
	  yscrollpos= a;
	  SCROLLINFO sif;
	  sif.cbSize= sizeof(SCROLLINFO);
	  sif.fMask= SIF_POS;
	  sif.nPos= theScreen->ymax-24-a;
	  SetScrollInfo(wscroll, SB_CTL, &sif, 1);
	};
      };
    };
    return(0);
  case WM_USER+1: //change com state
    {
      MSG umsg;
      while(PeekMessage(&umsg, w, WM_USER+1, WM_USER+1, PM_REMOVE)) ;
      commmodem();
    };
    return(0);
  case WM_USER+2:
    notifyscreen= 1;
    return(0);
  };
  return DefWindowProc(w, msg, wp, lp);
};

void
initerror(const WCHAR* msg)
{
  MessageBox(wbase, msg+1
  , L"24term", MB_OK|MB_ICONERROR|MB_APPLMODAL);
  WSACleanup();
  ExitThread(0);
};

void
parsearg(Config *argconfig, WCHAR const* arg)
{ //コマンドライン解析
  int argm= 0; //switch mode
  for(;;) {
    WCHAR wc, argv[_MAX_PATH];
    while(*arg==L' ') arg++;
    if(*arg==0) break;
    int f= 0;
    int argl= 0;
    while(wc= *arg) {
      if(f==0&&wc==L' ') break;
      if(wc==L'"') {
	f= 1-f;
      } else {
	argv[argl++]= wc;
      };
      arg++;
    };
    if(f==1) break; //incomplete doublequotation
    argv[argl]= 0;
    if(argm==1) {
      //argconfig.pid= _wtol(argv);
      argm= 0;
    } else if(*argv==L'/'||*argv==L'-') { //switch
      if(_wcsicmp(argv+1, L"pid")==0) argm= 1;
    } else if(*argv==L'\\') { //config file
      wcscpy(argconfig->file, argv);
    } else { // host name
      wcscpy(argconfig->host, argv);
      //WideCharToMultiByte(0, 0, argv, -1, theScreen->config.host, _MAX_PATH, 0, 0);
    };
  };
};

int WINAPI
WinMain(
  HINSTANCE inst,
  HINSTANCE prev,
  LPTSTR arg,
  int show
) {
  theInst= inst;
  wbase= 0;
  DWORD rd;
  HANDLE ipcm= CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE
  , 0, _MAX_PATH*2+2, L"24mapping");
  if(ipcm==NULL) initerror(L"xFailed. (shm)");
  WCHAR* ipcp= (WCHAR*)MapViewOfFile(ipcm, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if(ipcp==0) initerror(L"xFailed. (map)");
  HANDLE ipcx= CreateMutex(0, 0, L"24mutex");
  if(ipcx==NULL) initerror(L"xFailed. (mutex)");
  //ウィンドウなしで刺さったままで前プロセスが残っていても無視して新規起動
  HWND ipcw= FindWindow(L"24term", 0);
  if(GetLastError()==ERROR_ALREADY_EXISTS&&ipcw!=0) {
    //後に作成
    rd= WaitForSingleObject(ipcx, 100);
    if(rd!=WAIT_OBJECT_0) initerror(L"xCan't lock mutex.");
    ipcp[0]= 1;
    wcscpy(ipcp+1, arg);
    ReleaseMutex(ipcx);
    SetForegroundWindow(ipcw);
    PostMessage(ipcw, WM_USER+2, 0, 0); //notify
    return(0);
  } else { //最初に作成
    rd= WaitForSingleObject(ipcx, 100);
    if(rd!=WAIT_OBJECT_0) initerror(L"xCan't lock mutex.");
    ipcp[0]= 0; //initialize
    ReleaseMutex(ipcx);
  };

  Config argconfig;
  argconfig.init();
  parsearg(&argconfig, arg);

  srand(GetTickCount());
  { //画面の色数とサイズ
    HDC dc= GetDC(0);
    int rt= GetDeviceCaps(dc, BITSPIXEL);
    if(rt==8) {
      if(!(GetDeviceCaps(dc, RASTERCAPS)&RC_PALETTE)) initerror(L"pNo palette found.");
      if(GetDeviceCaps(dc, SIZEPALETTE)!=256) initerror(L"pPalette size != 256.");
      Draw::devcolor= 8;
    } else if(rt==2) {
      Draw::devcolor= 2;
    } else if(rt>=15) {
      Draw::devcolor= 16;
    } else {
      initerror(L"pUnsupported colors.");
    };
    Draw::devwidth= GetDeviceCaps(dc, HORZRES);
    if(Draw::devwidth<480||GetDeviceCaps(dc, VERTRES)<240) initerror(L"pToo small screen.");
    ReleaseDC(0, dc);
  };
//  Draw::devwidth= 480;
//  Draw::devcolor= 2;
  { //winsock初期化(不要?)
     WSADATA wd;
     WSAStartup(MAKEWORD(1,1), &wd);
  };
  { //画面作成
    InitCommonControls();

    LOGFONT fnt;
    fnt.lfHeight= -12;
    fnt.lfWidth= 0;
    fnt.lfEscapement= 900;
    fnt.lfOrientation= 0;
    fnt.lfWeight= FW_NORMAL;
    fnt.lfItalic= 0;
    fnt.lfUnderline= 0;
    fnt.lfStrikeOut= 0;
    fnt.lfCharSet= DEFAULT_CHARSET; //SHIFTJIS_CHARSET;
    fnt.lfOutPrecision= OUT_DEFAULT_PRECIS;
    fnt.lfClipPrecision= CLIP_DEFAULT_PRECIS;
    fnt.lfQuality= DEFAULT_QUALITY;
    fnt.lfPitchAndFamily= DEFAULT_PITCH;
    wcscpy(fnt.lfFaceName, L"Tahoma");
    titlefont= CreateFontIndirect(&fnt);

    WNDCLASS wc;
    wc.style= 0;
    wc.lpfnWndProc= (WNDPROC)baseproc;
    wc.cbClsExtra= 0; wc.cbWndExtra= 0;
    wc.hInstance= theInst;
    wc.hIcon= LoadIcon(theInst, MAKEINTRESOURCE(IDI_MAIN)); wc.hCursor= 0;
    wc.hbrBackground= (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName= 0;
    wc.lpszClassName= L"24term";
    if(RegisterClass(&wc)==0) initerror(L"xFailed. (24term)");
    wc.style= 0;
    wc.lpfnWndProc= (WNDPROC)menuproc;
    wc.cbClsExtra= 0; wc.cbWndExtra= 0;
    wc.hInstance= theInst;
    wc.hIcon= 0; wc.hCursor= 0;
    wc.hbrBackground= (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName= 0;
    wc.lpszClassName= L"24menu";
    if(RegisterClass(&wc)==0) initerror(L"xFailed. (24menu)");
    wc.style= 0;
    wc.lpfnWndProc= (WNDPROC)titleproc;
    wc.cbClsExtra= 0; wc.cbWndExtra= 0;
    wc.hInstance= theInst;
    wc.hIcon= 0; wc.hCursor= 0;
    wc.hbrBackground= (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName= 0;
    wc.lpszClassName= L"24title";
    if(RegisterClass(&wc)==0) initerror(L"xFailed. (24title)");
    wc.style= 0;
    wc.lpfnWndProc= (WNDPROC)dispproc;
    wc.cbClsExtra= 0; wc.cbWndExtra= 0;
    wc.hInstance= theInst;
    wc.hIcon= 0; wc.hCursor= 0;
    wc.hbrBackground= (HBRUSH)(COLOR_WINDOW+1);// CreateSolidBrush(0x0100001b);
    wc.lpszMenuName= 0;
    wc.lpszClassName= L"24disp";
    if(RegisterClass(&wc)==0) initerror(L"xFailed. (24disp)");

    wmodal= 0;
    wbase= CreateWindowEx(0
    , L"24term", L"24term"
    , WS_VISIBLE|WS_CLIPCHILDREN //0 は OVERLAPPED と見なされる
    , 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, theInst, 0);
    if(wbase==0) initerror(L"xFailed. (wbase)");
    SendMessage(wbase, WM_SETICON, 0, (LPARAM)LoadImage(theInst, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0));
//    ShowWindow(wbase, SW_SHOW); UpdateWindow(wbase);
    RECT r; GetWindowRect(wbase, &r);
    Draw::baseheight= r.bottom-r.top;
    if(Draw::baseheight>240) Draw::baseheight= 240;
    selpoint= -1;
    wdisp= CreateWindowEx(WS_EX_TOPMOST|WS_EX_NOACTIVATE
    , L"24disp", L"disp"
    , WS_POPUP|WS_VISIBLE
    , Draw::devwidth<640?0:21, 0, Draw::devwidth<640?480:485, 240, wbase, 0, theInst, 0);
    if(wdisp==0) initerror(L"xFailed. (wdisp)");
    wmenu= CreateWindowEx(WS_EX_NOACTIVATE
    , L"24menu", L"menu"
    , WS_POPUP|WS_VISIBLE
    , 0, 0, 21, 22, wbase, 0, theInst, 0);
    if(wmenu==0) initerror(L"xFailed. (wmenu)");
    wtitle= CreateWindowEx(WS_EX_NOACTIVATE
    , L"24title", L"title"
    , WS_CHILD|(Draw::devwidth>=640?WS_VISIBLE:0)
    , 0, 24, 21, Draw::baseheight-24, wbase, 0, theInst, 0);
    if(wtitle==0) initerror(L"xFailed. (wtitle)");
    scrollon= 0;
    if(Draw::devwidth>=640) {
      wscroll= CreateWindowEx(0
      , L"SCROLLBAR", L""
      , WS_CHILD|WS_VISIBLE|SBS_VERT
      , 506, 0, 19, Draw::baseheight, wbase, 0, theInst, 0);
    } else {
      wscroll= CreateWindowEx(0
      , L"SCROLLBAR", L""
      , WS_CHILD|SBS_HORZ
      , 0, 220, 480, 20, wdisp, 0, theInst, 0);
    };
    if(wscroll==0) initerror(L"xFailed. (wscroll)");
    yscrollpos= 0;
    wcomm= CreateDialogIndirect(theInst
    , (DLGTEMPLATE*)LockResource(LoadResource(theInst
    , FindResource(theInst, MAKEINTRESOURCE(IDD_COMM), RT_DIALOG))), wbase, commproc);
    SetWindowLong(wcomm, GWL_EXSTYLE, GetWindowLong(wcomm, GWL_EXSTYLE)|WS_EX_NOACTIVATE);
    SetWindowPos(wcomm, 0, Draw::devwidth-115, 0, 115, 78, SWP_NOZORDER);
#if 0
    wdebug= CreateWindowEx(0
    , L"STATIC", L""
    , WS_CHILD|WS_VISIBLE
    , Draw::devwidth-100, 100, 100, 100, wbase, 0, theInst, 0);
#endif
    CreateWindowEx(0, L"STATIC", L"24term (2002-4-12)"
    , WS_CHILD|WS_VISIBLE|SS_NOPREFIX|SS_LEFTNOWORDWRAP, 28, 3, 150, 20, wbase, 0, theInst, 0);
    HWND wabout= CreateWindowEx(0, L"STATIC",
    L"80x24 vt100 terminal emulator for WindowsCE\n"
    L"Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan\n"
    L"Distributed under the BSD license.\n"
    L"http://www.softclub.jp/~zoro/ce/"
    , WS_CHILD|WS_VISIBLE|SS_NOPREFIX|SS_LEFTNOWORDWRAP, 28, 23, 270, 52, wbase, 0, theInst, 0);
    fnt.lfHeight= -11;
    fnt.lfEscapement= 0;
    SendMessage(wabout, WM_SETFONT, (WPARAM)CreateFontIndirect(&fnt), 1);
    SetWindowPos(wdisp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
  };

  argconfig.load();
  theDraw= 0;
  LPCWSTR es= createscreen(argconfig);
  if(es[0]!=0&&es[0]!='0') initerror(es);

  notifyscreen= 0;
  notifyipc= 0;
  for(;;) {
    int f= 0;
    for(;;) {
      MSG msg;
      if(f==0) {
        GetMessage(&msg, NULL, 0, 0);
	if(theScreen) theScreen->sockfreeze= 0;
      } else if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)==0) break;
      f= 1;
      if(msg.message==WM_QUIT) { //終了
        for(int i= 0; i<screenmax; i++) {
	  screen[i]->close();
	};
        dispresetpalette();
        WSACleanup();
        return(msg.wParam); //終了
      };
      if(msg.message==WM_KEYDOWN||msg.message==WM_SYSKEYDOWN) {
	if(msg.wParam==VK_SPACE&&(GetKeyState(VK_CONTROL)&0x8000)) { //C-SPC
	  PostMessage(msg.hwnd, WM_CHAR, 0x00, msg.lParam);
	} else if(msg.wParam==VK_BACKQUOTE&&(GetKeyState(VK_CONTROL)&0x8000)) { //C-@
	  PostMessage(msg.hwnd, WM_CHAR, 0x00, msg.lParam);
	} else if(msg.wParam==VK_APOSTROPHE&&(GetKeyState(VK_CONTROL)&0x8000)) { //C-^
	  PostMessage(msg.hwnd, WM_CHAR, 0x1e, msg.lParam);
	} else if(msg.wParam==VK_SLASH&&(GetKeyState(VK_CONTROL)&0x8000)) { //C-/
	  PostMessage(msg.hwnd, WM_CHAR, 0x1f, msg.lParam);
	} else if(msg.wParam==0xE2/*EXTEND_BSLASH*/&&(GetKeyState(VK_CONTROL)&0x8000)&&(GetKeyState(VK_SHIFT)&0x8000)) { //C-_
	  PostMessage(msg.hwnd, WM_CHAR, 0x1f, msg.lParam);
        } else if(msg.wParam==VK_RETURN&&theScreen->config.enter!=1) {
	  if(theScreen->config.enter==0) PostMessage(msg.hwnd, WM_CHAR, 0x0d, msg.lParam);
	  PostMessage(msg.hwnd, WM_CHAR, 0x0a, msg.lParam);
        } else if(msg.wParam==VK_BACK&&theScreen->config.bsdel) {
	  PostMessage(msg.hwnd, WM_CHAR, 0x7f, msg.lParam);
	} else if(msg.wParam==VK_DELETE) {
	  if(theScreen->config.bsdel) {
	    PostMessage(msg.hwnd, WM_CHAR, 0x8, msg.lParam);
	  } else {
	    PostMessage(msg.hwnd, WM_CHAR, 0x7f, msg.lParam);
	  };
	} else {
          TranslateMessage(&msg);
	};
      } else {
        TranslateMessage(&msg);
      };
      DispatchMessage(&msg);
    };
    //idle
    if(notifyipc) { //他から呼び出し
      DWORD rd= WaitForSingleObject(ipcx, 100);
      WCHAR ipcarg[_MAX_PATH];
      int ipckick= 0;
      if(rd==WAIT_OBJECT_0) {
        notifyipc= 0;
	if(ipcp[0]) { //arg exist
	  wcscpy(ipcarg, ipcp+1);
	  ipckick= 1;
	};
        ipcp[0]= 0; //initialize
        ReleaseMutex(ipcx);
      };
      if(ipckick&&screenmax<8) {
        Config newconfig;
	newconfig.init();
	parsearg(&newconfig, ipcarg);
	newconfig.load();
	createscreen(newconfig);
      };
    };
    if(notifyscreen) { //スクリーン停止処理
      commredisp();
      notifyscreen= 0;
      while(checkscreen()) ;
    };
  };
};
