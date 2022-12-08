// screen.h: Screen �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_)
#define AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "config.h"

#define SC_KANJI   0x0100
#define SC_KANJI2  0x0080
#define SC_COLOR   0x0e00
#define SC_BLINK   0x1000
#define SC_BOLD    0x2000
#define SC_ULINE   0x4000
#define SC_REVERSE 0x8000

struct Screen {
  HANDLE thread;
  CRITICAL_SECTION lock; //iy,ybase,selstart,selend,yscroll,copy�Ɋւ���lock
  int ybase;
  int selstart, selend;
  int ymax;
  int yscroll;
  int copy[24];     //y==copy[y]�Ȃ�ړ��Ȃ��A-1�Ȃ�R�s�[���Ȃ�
  int *iy;          //iy[(y-ybase)%ymax]���o�b�t�@���e y=0:�ŉ��s
  WORD (*text)[82];
  long *dirty;      //dirty[iy[y]]&0xff ... dirty[iy[y]]>>8 �����������͈�
                    //lock�s�v�ɂ��邽�߂�1�ϐ�
  int dirtyx;       //dirty�J�n�ʒu ... cx�܂ł�dirty

  enum { STbase, STkanji, STesc, STcsi, STiac, STchar
  } state;
  int curx, cury;
  WORD* textp;
  int kanji1;
  int rolltop, rolltail;
  int csimode;
  int csiargc;
  int csiargv[16];
  WORD attr; //�����̐F
  int iac, iacsn;
  BYTE iacs[32];
  int charset;

  WORD attrfg; //�O�i�F+�t���O
  WORD attrbg; //�w�i�F
  int sendset;	//0=ascii/1=jis-roman/2=���p����/3=����
  int recvset;

  void print(const char*, int);
  void printk(const char*, int);
  void rollup(int, int);
  void rolldown(int, int);
  void enddirty();
  void textpupdate();
  void clearbefore();
  void clearafter();
  void nextline();

  volatile DWORD sock;
  volatile int sockmode; //0=�ڑ��� 1=�ڑ� -2=�G���[ -1=�I��
  int socktype; //1=TCPIP/2=�r������socket(SSL)/3=IRDA/4=Serial
  int sockfreeze; //�G���[���ɂ�1�ő��M���~
  HANDLE socklock; //����Mlock�ASSL���̂ݎg�p
  int ssl; //1=ssl-telnet-connecting/2=ssl connected

  HANDLE printlock; //���[�J��echo���̂ݎg�p

  int rts, dtr;

  void init();
  PCWSTR allocate();
  PCWSTR open();
  static DWORD WINAPI screenwrap(void* parm);
  void main();
  void write(const char*p, int n);
  void writek(const char*p, int n); //�������ӎ����ď�������
  void close();
  void release();

  static int CALLBACK certwrap(DWORD, void*, DWORD, BLOB*, DWORD);
  int certhook(DWORD, DWORD, BLOB*, DWORD);
  int certreq;

  Config config;
  Config configback; //�I���W�i���̐F��ێ�
  WCHAR error[_MAX_PATH]; //error string
};

#endif // !defined(AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_)
