// draw.h: Draw �N���X�̃C���^�[�t�F�C�X
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
  void update(Screen*, HDC, int); //�X�V�����̂� 0=�X�N���[���g��/1=�X�N���[���g���Ȃ�
  void redraw(Screen*, HDC); //�S��
  void cursor(Screen*, HDC, int);

  void updatewin(Screen*, HWND); //�X�V
  void redrawwin(Screen*, HWND); //�ĕ`��

  virtual PCWSTR init(const Config*, HWND);
  virtual void draw(const WORD* vs, int vsn, int sx, int sy)= 0;
  virtual void clear(int vsn, int sx, int sy)= 0;

  void drawframe(Screen*, HDC);

  int iy[24]; //redraw&refresh�ő���Acursor�Ŏg�p
  int useb[24];

  int picwidth; //Pic�N���X

  WORD curb, curb0, curb1; //�J�[�\���p�����ޔ�
  int curx, cury; //�J�[�\���ʒu
  int lastsel; //�Ōオselection�X�V�Ȃ�1

  //�m�ۂ�����j�����Ȃ�
  static HBRUSH basebrush;

  static HANDLE k12x10f;
  static HANDLE k12x10m;
  static const char *k12x10;
  static const BYTE boldanks[];

  static HBITMAP bitmap;
  static char* bitdata;
  static HDC bitdc;
  static HDC picdc;  //Pic�N���X

  //�قڒ萔
  static int winoffx;
  static int bitoffx;
  static int devcolor;
  static int devwidth;
  static int baseheight;
};

#endif // !defined(AFX_DRAW_H__956A2E61_2AB6_4B4B_9500_4514F35FFD9A__INCLUDED_)
