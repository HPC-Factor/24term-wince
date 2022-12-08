// pic8.h: Pic8 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIC8_H__02F22EB3_AC81_4E9F_BBC5_9AA9051FAAD6__INCLUDED_)
#define AFX_PIC8_H__02F22EB3_AC81_4E9F_BBC5_9AA9051FAAD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "draw8.h"

struct Pic8
: Draw8
{
  virtual PCWSTR init(const Config*, HWND);

  virtual void draw(const WORD* vs, int vsn, int sx, int sy);
  virtual void clear(int vsn, int sx, int sy);
  void space(int vsn, char*, const char*);

  //int picwidth; draw.hに移動
  int picpaln;
  //開放しない
  static HBITMAP picmap;
  static char* picdata;
  //static HDC picdc; draw.hに移動
  static int* picpal;
};

#endif // !defined(AFX_PIC8_H__02F22EB3_AC81_4E9F_BBC5_9AA9051FAAD6__INCLUDED_)
