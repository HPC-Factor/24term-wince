// pic16.h: Pic16 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIC16_H__44DEC591_8E73_4332_864A_ED1A4188CB95__INCLUDED_)
#define AFX_PIC16_H__44DEC591_8E73_4332_864A_ED1A4188CB95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "draw16.h"

struct Pic16
: Draw16
{
  virtual PCWSTR init(const Config*, HWND);

  virtual void draw(const WORD* vs, int vsn, int sx, int sy);
  virtual void clear(int vsn, int sx, int sy);
  void space(int vsn, char*, const char*);

  //int picwidth; //Drawへ移動
  //開放しない
  static HBITMAP picmap;
  static char* picdata;
  //static HDC picdc; //Drawへ移動
};

#endif // !defined(AFX_PIC16_H__44DEC591_8E73_4332_864A_ED1A4188CB95__INCLUDED_)
