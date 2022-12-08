// draw2.h: Draw2 �N���X�̃C���^�[�t�F�C�X
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

  //�J�����Ȃ�
  char colback;
  static char* patmask;
  static char* colpat[];
};

#endif // !defined(AFX_DRAW2_H__FC0E4231_F4B4_4D65_83FC_717F6D098383__INCLUDED_)
