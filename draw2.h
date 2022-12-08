// draw2.h: Draw2 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAW2_H__FC0E4231_F4B4_4D65_83FC_717F6D098383__INCLUDED_)
#define AFX_DRAW2_H__FC0E4231_F4B4_4D65_83FC_717F6D098383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "draw.h"

struct Draw2
: Draw
{
  virtual PCWSTR init(const Config*, HWND);
  virtual void draw(const WORD* vs, int vsn, int sx, int sy);
  virtual void clear(int vsn, int sx, int sy);
  void space(int vsn, char*, int);

  //開放しない
  char colback;
  static char* patmask;
  static char* colpat[];
};

#endif // !defined(AFX_DRAW2_H__FC0E4231_F4B4_4D65_83FC_717F6D098383__INCLUDED_)
