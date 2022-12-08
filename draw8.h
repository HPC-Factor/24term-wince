// draw8.h: Draw8 �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAW8_H__AC3BDD6E_0551_4FD8_8264_E739897A94B6__INCLUDED_)
#define AFX_DRAW8_H__AC3BDD6E_0551_4FD8_8264_E739897A94B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "draw.h"

struct Draw8
: Draw
{
  virtual PCWSTR init(const Config*, HWND);
  virtual void draw(const WORD* vs, int vsn, int sx, int sy);
  virtual void clear(int vsn, int sx, int sy);
  void space(int vsn, char*);

  void palset(int n, int col);

  //�m�ۂ�����j�����Ȃ�
  static HPALETTE palette;
  static LOGPALETTE *logpal;

  static char* patmask;
#ifdef ANKS1
  static char* hanfont;
#endif
  static long colfore[]; //const�͖��ȍœK���Œx���Ȃ�
  static long colback;
  static long coluline;
  static BYTE bitmask[];
  static BYTE revmask[];
};

#endif // !defined(AFX_DRAW8_H__AC3BDD6E_0551_4FD8_8264_E739897A94B6__INCLUDED_)
