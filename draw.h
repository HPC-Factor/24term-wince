// draw.h: Draw クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAW_H__956A2E61_2AB6_4B4B_9500_4514F35FFD9A__INCLUDED_)
#define AFX_DRAW_H__956A2E61_2AB6_4B4B_9500_4514F35FFD9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct Screen;
struct Config;

struct Draw
{
  void update(Screen*, HDC, int); //更新部分のみ 0=スクロール使う/1=スクロール使えない
  void redraw(Screen*, HDC); //全部
  void cursor(Screen*, HDC, int);

  void updatewin(Screen*, HWND); //更新
  void redrawwin(Screen*, HWND); //再描画

  virtual PCWSTR init(const Config*, HWND);
  virtual void draw(const WORD* vs, int vsn, int sx, int sy)= 0;
  virtual void clear(int vsn, int sx, int sy)= 0;

  void drawframe(Screen*, HDC);

  int iy[24]; //redraw&refreshで代入、cursorで使用
  int useb[24];

  int picwidth; //Picクラス

  WORD curb, curb0, curb1; //カーソル用文字退避
  int curx, cury; //カーソル位置
  int lastsel; //最後がselection更新なら1

  //確保したら破棄しない
  static HBRUSH basebrush;

  static HANDLE k12x10f;
  static HANDLE k12x10m;
  static const char *k12x10;
  static const BYTE boldanks[];

  static HBITMAP bitmap;
  static char* bitdata;
  static HDC bitdc;
  static HDC picdc;  //Picクラス

  //ほぼ定数
  static int winoffx;
  static int bitoffx;
  static int devcolor;
  static int devwidth;
  static int baseheight;
};

#endif // !defined(AFX_DRAW_H__956A2E61_2AB6_4B4B_9500_4514F35FFD9A__INCLUDED_)
