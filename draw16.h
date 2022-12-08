// draw16.h: Draw16 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAW16_H__4697D6D2_BDC3_4EC5_82D1_1152C2BD8B19__INCLUDED_)
#define AFX_DRAW16_H__4697D6D2_BDC3_4EC5_82D1_1152C2BD8B19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "draw.h"

struct Draw16
: Draw
{
  virtual PCWSTR init(const Config*, HWND);
  virtual void draw(const WORD* vs, int vsn, int sx, int sy);
  virtual void clear(int vsn, int sx, int sy);
  void space(int vsn, char*);

  //開放しない
  long colback;
  long coluline;
  static char* patmask;
  static long colfore[];
  static BYTE bitmask[];
  static BYTE revmask[];
};

#endif // !defined(AFX_DRAW16_H__4697D6D2_BDC3_4EC5_82D1_1152C2BD8B19__INCLUDED_)
