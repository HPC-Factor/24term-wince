// config.h: Config クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__31EE453D_0FB3_46D8_A874_A1EA6B9307E3__INCLUDED_)
#define AFX_CONFIG_H__31EE453D_0FB3_46D8_A874_A1EA6B9307E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct Config
{
  WCHAR file[_MAX_PATH];
  WCHAR name[_MAX_PATH]; //タイトル名
  WCHAR host[_MAX_PATH];
  WCHAR pic[_MAX_PATH];
  int gammalow;
  int gammamid;
  int gammahigh;
  int black; //colback1の輝度
  int colback1;
  int colback2;
  int colback; //実際に使われる色
  int coluline;
  int color[16];
  int sjis;
  int yen;
  int nobold;
  int stay;
  int ymax;
  int enter; //0=CRLF/1=CR(Default)/2=LF
  int newline; //0=default/1=CR/2=LF
  int bsdel; //0=default/1=swap bs-del
  int echo; //1=localecho
  int certalloc; //cert allocated len
  int certlen; //長さ
  BYTE* cert;

  WCHAR title[_MAX_PATH]; //実タイトル
  int titletop;
  int titleh;

  void init(void);
  static int parsecolor(const char*);
  PCWSTR load(void);
  PCWSTR save(void);
  void setrandom(void);
  PCWSTR checkpic(void) const;
  int gammacolor(int) const;
  void writem(HANDLE, WCHAR const *);

  //ここから後ろはSetup関連
  HWND wconfig;
  HWND wcmenu;
  static LRESULT CALLBACK cmenuwrap(HWND, UINT, WPARAM, LPARAM);
  LRESULT cmenuproc(HWND, UINT, WPARAM, LPARAM);
  void cmenudrop();
  RECT drect;

  HWND whost;
  static void setcomboheight(HWND);
  static BOOL CALLBACK hostwrap(HWND, UINT, WPARAM, LPARAM);
  BOOL hostproc(HWND, UINT, WPARAM, LPARAM);
  void hostset(HWND);
  void hostget(HWND);

  HWND warrow;
  static LRESULT CALLBACK arrowwrap(HWND, UINT, WPARAM, LPARAM);
  LRESULT arrowproc(HWND, UINT, WPARAM, LPARAM);
  void arrowdraw(HDC, int);
  void arrowdrop(HWND);

  HWND wcolor;
  int curcolor;
  int currand;
  void colordraw(HDC);
  static BOOL CALLBACK colorwrap(HWND, UINT, WPARAM, LPARAM);
  BOOL colorproc(HWND, UINT, WPARAM, LPARAM);
  int colorcapt;
  void colorpalette();
  void colorset(HWND);
  void colorget(HWND);

  HWND wgamma;
  int gammacapt;
  int gammacaptx;
  double gammarate;
  static LRESULT CALLBACK gammawrap(HWND, UINT, WPARAM, LPARAM);
  LRESULT gammaproc(HWND, UINT, WPARAM, LPARAM);
  void gammadraw(HDC);
  void gammaset(HWND);
  void gammadisp();

  HWND wmisc;
  static BOOL CALLBACK miscwrap(HWND, UINT, WPARAM, LPARAM);
  BOOL miscproc(HWND, UINT, WPARAM, LPARAM);
  void miscgset(HWND);
  void miscset(HWND);
  void miscget(HWND);

  HWND wtab;
  static BOOL CALLBACK configwrap(HWND, UINT, WPARAM, LPARAM);
  BOOL configproc(HWND, UINT, WPARAM, LPARAM);
};

#endif // !defined(AFX_CONFIG_H__31EE453D_0FB3_46D8_A874_A1EA6B9307E3__INCLUDED_)
